/**
 *  @copyright Copyright 2020 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *  @file EventCategorizerTools.cpp
 */

#include "EventCategorizerTools.h"
#include "../LargeBarrelAnalysis/HitFinderTools.h"
#include <TMath.h>
#include <vector>

using namespace std;

bool Ellipse::isInside( const double& x, const double& y ) const
{
  double A = TMath::Power( ( ( x - fXCenter) * fCosTheta + ( y - fYCenter ) * fSinTheta ) / fRadiusX , 2 );
  double B = TMath::Power( ( ( x - fXCenter) * fSinTheta - ( y - fYCenter ) * fCosTheta ) / fRadiusY , 2 );

  return A + B < 1.0;
}

void Ellipse::setParamaters( double x0, double y0, double rx, double ry, double theta )
{
 fXCenter = x0;
 fYCenter = y0;
 fCosTheta = TMath::Cos( TMath::DegToRad() * theta );
 fSinTheta = TMath::Sin( TMath::DegToRad() * theta );
 fRadiusX = rx;
 fRadiusY = ry;
}

bool DVDFilter::IsInRight(const double& delta1i, const double& delta2i) const { return fRightEllipse.isInside(delta1i,delta2i); }

bool DVDFilter::IsInLeft(const double& delta1i, const double& delta2i) const { return fLeftEllipse.isInside(delta1i,delta2i);}

/**
* Method for determining type of event - back to back 2 gamma
*/
bool EventCategorizerTools::checkFor2Gamma(
  const JPetEvent& event, JPetStatistics& stats, bool saveHistos,
  double b2bSlotThetaDiff, double b2bTimeDiff
)
{
  if (event.getHits().size() < 2) {
    return false;
  }
  for (uint i = 0; i < event.getHits().size(); i++) {
    for (uint j = i + 1; j < event.getHits().size(); j++) {
      JPetHit firstHit, secondHit;
      if (event.getHits().at(i).getTime() < event.getHits().at(j).getTime()) {
        firstHit = event.getHits().at(i);
        secondHit = event.getHits().at(j);
      } else {
        firstHit = event.getHits().at(j);
        secondHit = event.getHits().at(i);
      }
      // Checking for back to back
      double timeDiff = fabs(firstHit.getTime() - secondHit.getTime());
      double deltaLor = (secondHit.getTime() - firstHit.getTime()) * kLightVelocity_cm_ps / 2.;
      double theta1 = min(firstHit.getBarrelSlot().getTheta(), secondHit.getBarrelSlot().getTheta());
      double theta2 = max(firstHit.getBarrelSlot().getTheta(), secondHit.getBarrelSlot().getTheta());
      double thetaDiff = min(theta2 - theta1, 360.0 - theta2 + theta1);
      if (saveHistos) {
        stats.fillHistogram("2Gamma_Zpos", firstHit.getPosZ());
        stats.fillHistogram("2Gamma_Zpos", secondHit.getPosZ());
        stats.fillHistogram("2Gamma_TimeDiff", timeDiff / 1000.0);
        stats.fillHistogram("2Gamma_DLOR", deltaLor);
        stats.fillHistogram("2Gamma_ThetaDiff", thetaDiff);
        stats.fillHistogram("2Gamma_Dist", calculateDistance(firstHit, secondHit));
      }
      if (fabs(thetaDiff - 180.0) < b2bSlotThetaDiff && timeDiff < b2bTimeDiff) {
        if (saveHistos) {
          TVector3 annhilationPoint = calculateAnnihilationPoint(firstHit, secondHit);
          stats.fillHistogram("Annih_TOF", calculateTOFByConvention(firstHit, secondHit));
          stats.fillHistogram("AnnihPoint_XY", annhilationPoint.X(), annhilationPoint.Y());
          stats.fillHistogram("AnnihPoint_ZX", annhilationPoint.Z(), annhilationPoint.X());
          stats.fillHistogram("AnnihPoint_ZY", annhilationPoint.Z(), annhilationPoint.Y());
          stats.fillHistogram("Annih_DLOR", deltaLor);
        }
        return true;
      }
    }
  }
  return false;
}

/**
* Method for determining type of event - 3Gamma
*/
bool EventCategorizerTools::checkFor3Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveHistos)
{
  if (event.getHits().size() < 3) return false;
  for (uint i = 0; i < event.getHits().size(); i++) {
    for (uint j = i + 1; j < event.getHits().size(); j++) {
      for (uint k = j + 1; k < event.getHits().size(); k++) {
        JPetHit firstHit = event.getHits().at(i);
        JPetHit secondHit = event.getHits().at(j);
        JPetHit thirdHit = event.getHits().at(k);

        vector<double> thetaAngles;
        thetaAngles.push_back(firstHit.getBarrelSlot().getTheta());
        thetaAngles.push_back(secondHit.getBarrelSlot().getTheta());
        thetaAngles.push_back(thirdHit.getBarrelSlot().getTheta());
        sort(thetaAngles.begin(), thetaAngles.end());

        vector<double> relativeAngles;
        relativeAngles.push_back(thetaAngles.at(1) - thetaAngles.at(0));
        relativeAngles.push_back(thetaAngles.at(2) - thetaAngles.at(1));
        relativeAngles.push_back(360.0 - thetaAngles.at(2) + thetaAngles.at(0));
        sort(relativeAngles.begin(), relativeAngles.end());
        double transformedX = relativeAngles.at(1) + relativeAngles.at(0);
        double transformedY = relativeAngles.at(1) - relativeAngles.at(0);

        if (saveHistos) {
          stats.fillHistogram("3Gamma_Angles", transformedX, transformedY);
        }
      }
    }
  }
  return true;
}

/**
* Method for determining type of event - prompt
*/
bool EventCategorizerTools::checkForPrompt(
  const JPetEvent& event, JPetStatistics& stats, bool saveHistos,
  double deexTOTCutMin, double deexTOTCutMax, std::string fTOTCalculationType)
{
  for (unsigned i = 0; i < event.getHits().size(); i++) {
    double tot = HitFinderTools::calculateTOT(event.getHits().at(i), 
                                              HitFinderTools::getTOTCalculationType(fTOTCalculationType));
    if (tot > deexTOTCutMin && tot < deexTOTCutMax) {
      if (saveHistos) {
        stats.fillHistogram("Deex_TOT_cut", tot);
      }
      return true;
    }
  }
  return false;
}

/**
* Method for determining type of event - scatter
*/
bool EventCategorizerTools::checkForScatter(
  const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double scatterTOFTimeDiff, 
  std::string fTOTCalculationType)
{
  if (event.getHits().size() < 2) {
    return false;
  }
  for (uint i = 0; i < event.getHits().size(); i++) {
    for (uint j = i + 1; j < event.getHits().size(); j++) {
      JPetHit primaryHit, scatterHit;
      if (event.getHits().at(i).getTime() < event.getHits().at(j).getTime()) {
        primaryHit = event.getHits().at(i);
        scatterHit = event.getHits().at(j);
      } else {
        primaryHit = event.getHits().at(j);
        scatterHit = event.getHits().at(i);
      }

      double scattAngle = calculateScatteringAngle(primaryHit, scatterHit);
      double scattTOF = calculateScatteringTime(primaryHit, scatterHit);
      double timeDiff = scatterHit.getTime() - primaryHit.getTime();

      if (saveHistos) {
        stats.fillHistogram("ScatterTOF_TimeDiff", fabs(scattTOF - timeDiff));
      }

      if (fabs(scattTOF - timeDiff) < scatterTOFTimeDiff) {
        if (saveHistos) {
          stats.fillHistogram("ScatterAngle_PrimaryTOT", scattAngle, HitFinderTools::calculateTOT(primaryHit, 
                                                        HitFinderTools::getTOTCalculationType(fTOTCalculationType)));
          stats.fillHistogram("ScatterAngle_ScatterTOT", scattAngle, HitFinderTools::calculateTOT(scatterHit, 
                                                        HitFinderTools::getTOTCalculationType(fTOTCalculationType)));
        }
        return true;
      }
    }
  }
  return false;
}

/**
* Calculation of distance between two hits
*/
double EventCategorizerTools::calculateDistance(const JPetHit& hit1, const JPetHit& hit2)
{
  return (hit1.getPos() - hit2.getPos()).Mag();
}

/**
* Calculation of time that light needs to travel the distance between primary gamma
* and scattered gamma. Return value in picoseconds.
*/
double EventCategorizerTools::calculateScatteringTime(const JPetHit& hit1, const JPetHit& hit2)
{
  return calculateDistance(hit1, hit2) / kLightVelocity_cm_ps;
}

/**
* Calculation of scatter angle between primary hit and scattered hit.
* This function assumes that source of first gamma was in (0,0,0).
* Angle is calculated from scalar product, return value in degrees.
*/
double EventCategorizerTools::calculateScatteringAngle(const JPetHit& hit1, const JPetHit& hit2)
{
  return TMath::RadToDeg() * hit1.getPos().Angle(hit2.getPos() - hit1.getPos());
}

/**
* Calculation point in 3D, where annihilation occured
*/
TVector3 EventCategorizerTools::calculateAnnihilationPoint(const JPetHit& hitA, const JPetHit& hitB)
{
  double tof = EventCategorizerTools::calculateTOF(hitA, hitB);
  return calculateAnnihilationPoint(hitA.getPos(), hitB.getPos(), tof);
}

TVector3 EventCategorizerTools::calculateAnnihilationPoint(const TVector3& hitA, const TVector3& hitB, double tof)
{
  TVector3 middleOfLOR = 0.5 * (hitA + hitB);
  TVector3 versorOnLOR = (hitB - hitA).Unit()  ;

  double shift = 0.5 * tof  * kLightVelocity_cm_ps;
  TVector3 annihilationPoint(middleOfLOR.X() + shift * versorOnLOR.X(),
                             middleOfLOR.Y() + shift * versorOnLOR.Y(),
                             middleOfLOR.Z() + shift * versorOnLOR.Z());
  return annihilationPoint;
}

double EventCategorizerTools::calculateTOFByConvention(const JPetHit& hitA, const JPetHit& hitB)
{
  if (hitA.getBarrelSlot().getTheta() < hitB.getBarrelSlot().getTheta()) {
    return calculateTOF(hitA, hitB);
  } else {
    return calculateTOF(hitB, hitA);
  }
}

double EventCategorizerTools::calculateTOF(const JPetHit& hitA, const JPetHit& hitB)
{
  return EventCategorizerTools::calculateTOF(hitA.getTime(), hitB.getTime());
}

double EventCategorizerTools::calculateTOF(double time1, double time2)
{
  return (time1 - time2);
}

/**
* Calculating distance from the center of the decay plane
*/
double EventCategorizerTools::calculatePlaneCenterDistance(
  const JPetHit& firstHit, const JPetHit& secondHit, const JPetHit& thirdHit)
{
  TVector3 crossProd = (secondHit.getPos() - firstHit.getPos()).Cross(thirdHit.getPos() - secondHit.getPos());
  double distCoef = -crossProd.X() * secondHit.getPosX() - crossProd.Y() * secondHit.getPosY() - crossProd.Z() * secondHit.getPosZ();
  if (crossProd.Mag() != 0) {
    return fabs(distCoef) / crossProd.Mag();
  } else {
    ERROR("One of the hit has zero position vector - unable to calculate distance from the center of the surface");
    return -1.;
  }
}

bool EventCategorizerTools::notPassed(JPetStatistics& stats,const std::string& eff_obj_name,const uint& selection_id,bool condition)
{
  stats.getEffiHisto(eff_obj_name.c_str())->Fill(condition,selection_id);
  return !condition;
}

/**
** Extracts useful hits for 2+2 gits analysis
** Conditions:
** 1. Is not prompt gamma hit
** 2. Z position of hit is in range |z| < 23 cm
** 3. In not noise hit (i.e. very low value of TOT)
**/
std::vector<JPetHit> EventCategorizerTools::getHitsFor4HitsAnalysis(JPetStatistics& stats,const JPetEvent& event, const AnalysisParams& ap)
{
  std::vector<JPetHit> hits;
  for (uint i = 0; i < event.getHits().size(); i++)
  {
    JPetHit hit = event.getHits().at(i);
    double tot = HitFinderTools::calculateTOT(hit,HitFinderTools::getTOTCalculationType(ap.fTOTCalculationType));
    stats.fillHistogram("GHF4HA_All_TOT",ns(tot));
    if (notPassed(stats,"effHitsPreparation",0,tot < ap.fDeexTOTCutMin)) {continue;} //it is a prompt gamma's hit - skip this hit
    if (notPassed(stats,"effHitsPreparation",1,tot > ap.fNoiseTOTCut)) {continue;} //it is a noise's hit - skip this hit
    if (notPassed(stats,"effHitsPreparation",2,fabs(hit.getPosZ()) < 23.0)) {continue;} //it is a hit inside a part of scintillator covered by a detector
    stats.fillHistogram("GHF4HA_Passed_TOT",ns(tot));
    stats.fillHistogram("GHF4HA_Zpos",hit.getPosZ());
    hits.push_back(event.getHits().at(i));//save this hit - it can be the annihilator or scattered gamma hit
  }
  return hits;
}


/**
** Determines if given event is useful for 4 hist analysis (2+2).
** Conditions:
** 1. at least 4 hits
** 2. found annihilation gammas
** Conditions for annihilation gammas:
** 2.1 angle between angles is around 180 deg
** 2.2 time difference between 2 hits is lower then given by user.
**/
bool EventCategorizerTools::checkFor2Gamma4Hits(
  const std::vector<JPetHit>& hits, JPetStatistics& stats, FourHitsEvent& fhe, const AnalysisParams& ap)
{
  if (hits.size() < 4) {
    return false;
  }
  uint potential_ann_gamma_1_index = 0;//potential 1st annihilation gamma hit index in hits list
  uint potential_ann_gamma_2_index = 0;//potential 2nd annihilation gamma hit index in hits list
  bool passed;
  
  //compare all hits
  for (uint i = 0; i < hits.size(); i++) {
    for (uint j = i + 1; j < hits.size(); j++) {
      JPetHit firstHit, secondHit;
      //First hit is allways a hit which interacted earlien then other
      if (hits.at(i).getTime() < hits.at(j).getTime()) {
        firstHit = hits.at(i);
        secondHit = hits.at(j);
        potential_ann_gamma_1_index = i;
        potential_ann_gamma_2_index = j;
      } else {
        firstHit = hits.at(j);
        secondHit = hits.at(i);
        potential_ann_gamma_1_index = j;
        potential_ann_gamma_2_index = i; 
      }
      
      // Checking for back to back
      double timeDiff = fabs(firstHit.getTime() - secondHit.getTime());
      double deltaLor = (secondHit.getTime() - firstHit.getTime()) * kLightVelocity_cm_ps / 2.;
      double theta1 = min(firstHit.getBarrelSlot().getTheta(), secondHit.getBarrelSlot().getTheta());
      double theta2 = max(firstHit.getBarrelSlot().getTheta(), secondHit.getBarrelSlot().getTheta());
      double thetaDiff = min(theta2 - theta1, 360.0 - theta2 + theta1);
      double distance =  calculateDistance(firstHit, secondHit);
      double tot_1 = HitFinderTools::calculateTOT(firstHit,HitFinderTools::getTOTCalculationType(ap.fTOTCalculationType));
      double tot_2 = HitFinderTools::calculateTOT(secondHit,HitFinderTools::getTOTCalculationType(ap.fTOTCalculationType));
      //Fill histograms
      stats.fillHistogram("CF2G4H_AllHits_TimeDiff", timeDiff);
      stats.fillHistogram("CF2G4H_AllHits_ThetaDiff", thetaDiff);
      stats.fillHistogram("CF2G4H_AllHits_DeltaLOR",deltaLor);
      stats.fillHistogram("CF2G4H_AllHits_Zpos",firstHit.getPosZ());
      stats.fillHistogram("CF2G4H_AllHits_Zpos",secondHit.getPosZ());
      stats.fillHistogram("CF2G4H_AllHits_Dist",distance);
      stats.fillHistogram("CF2G4H_AllHits_XYpos",firstHit.getPosX(),firstHit.getPosY());
      stats.fillHistogram("CF2G4H_AllHits_XYpos",secondHit.getPosX(),secondHit.getPosY());
      stats.fillHistogram("CF2G4H_AllHits_TOT",ns(tot_1));
      stats.fillHistogram("CF2G4H_AllHits_TOT",ns(tot_2));
      
      if (notPassed(stats,"effAnnHitsFinding",0,fabs(thetaDiff - 180.0) < ap.fB2BSlotThetaDiff)) {continue;}
      if (notPassed(stats,"effAnnHitsFinding",1,timeDiff < ap.fMaxTimeDiff)) {continue;}
      if (notPassed(stats,"effAnnHitsFinding",2,isInTOTRange(tot_1,ap.fAnnihTOTCutMin,ap.fAnnihTOTCutMax) && isInTOTRange(tot_2,ap.fAnnihTOTCutMin,ap.fAnnihTOTCutMax))) {continue;}
      
      //Check annihilation point
      TVector3 apos = calculateAnnihilationPoint(firstHit,secondHit); //annihilation point
      double arxy = apos.x()*apos.x() + apos.y()*apos.y(); //annhilation point projection on xy plane
      //Fill histograms
      stats.fillHistogram("CF2G4H_PreAnnHits_TimeDiff", timeDiff);
      stats.fillHistogram("CF2G4H_PreAnnHits_ThetaDiff", thetaDiff);
      stats.fillHistogram("CF2G4H_PreAnnHits_DeltaLOR",deltaLor);
      stats.fillHistogram("CF2G4H_PreAnnHits_Zpos",firstHit.getPosZ());
      stats.fillHistogram("CF2G4H_PreAnnHits_Zpos",secondHit.getPosZ());
      stats.fillHistogram("CF2G4H_PreAnnHits_Dist",distance);
      stats.fillHistogram("CF2G4H_PreAnnHits_XYpos",firstHit.getPosX(),firstHit.getPosY());
      stats.fillHistogram("CF2G4H_PreAnnHits_XYpos",secondHit.getPosX(),secondHit.getPosY());
      stats.fillHistogram("CF2G4H_PreAnnHits_FoundIndexes",static_cast<double>(potential_ann_gamma_1_index),static_cast<double>(potential_ann_gamma_2_index));
      stats.fillHistogram("CF2G4H_PreAnnHits_AnnHitPosXY",apos.x(),apos.y());
      stats.fillHistogram("CF2G4H_PreAnnHits_AnnHitPosZ",apos.z());
      stats.fillHistogram("CF2G4H_PreAnnHits_TOT",ns(tot_1));
      stats.fillHistogram("CF2G4H_PreAnnHits_TOT",ns(tot_2));
      
      //if (notPassed(stats,"effAnnHitsFinding",0,)) {continue;}
      
      ////Annihilation point in a circle with radius R cm on a plane XY and its annhilation point z-position is |z|<4 cm 
      if (notPassed(stats,"effAnnHitsFinding",3,arxy < ap.fAnnihRadiusXY*ap.fAnnihRadiusXY)) {continue;}
      if (notPassed(stats,"effAnnHitsFinding",4,fabs(apos.z()) < ap.fAnnihZPosDelta)) {continue;}
      
      fhe.fAnniGamma1Index = potential_ann_gamma_1_index;
      fhe.fAnniGamma2Index = potential_ann_gamma_2_index;
      //Fill histograms
      stats.fillHistogram("CF2G4H_AnnHits_TimeDiff", timeDiff);
      stats.fillHistogram("CF2G4H_AnnHits_ThetaDiff", thetaDiff);
      stats.fillHistogram("CF2G4H_AnnHits_DeltaLOR",deltaLor);
      stats.fillHistogram("CF2G4H_AnnHits_Zpos",firstHit.getPosZ());
      stats.fillHistogram("CF2G4H_AnnHits_Zpos",secondHit.getPosZ());
      stats.fillHistogram("CF2G4H_AnnHits_Dist",distance);
      stats.fillHistogram("CF2G4H_AnnHits_XYpos",firstHit.getPosX(),firstHit.getPosY());
      stats.fillHistogram("CF2G4H_AnnHits_XYpos",secondHit.getPosX(),secondHit.getPosY());
      stats.fillHistogram("CF2G4H_AnnHits_FoundIndexes",static_cast<double>(potential_ann_gamma_1_index),static_cast<double>(potential_ann_gamma_2_index));
      stats.fillHistogram("CF2G4H_AnnHits_AnnHitPosXY",apos.x(),apos.y());
      stats.fillHistogram("CF2G4H_AnnHits_AnnHitPosZ",apos.z());
      stats.fillHistogram("CF2G4H_AnnHits_TOT",ns(tot_1));
      stats.fillHistogram("CF2G4H_AnnHits_TOT",ns(tot_2));
      return true;
    }
  }
  return false;
}

/**
** Tries to find scattered hit for each annihilation gamma.
** If found it fill informations about scattered gammas indexes, scattering angles (theta) and delta phi.
** Conditions:
** 1. Found hit is not prompt gamma hit (not implemented yet)
** 2. Each scattering hit is inside different ellipse.
**/
bool EventCategorizerTools::checkFor2Gamma4Hits2ScatteringHits(
  const std::vector<JPetHit>& hits, JPetStatistics& stats, FourHitsEvent& fhe, const DVDFilter& dvd
)
{
  //ahit - annihilation gamma hit
  JPetHit ahit_1 = hits.at(fhe.fAnniGamma1Index);
  JPetHit ahit_2 = hits.at(fhe.fAnniGamma2Index);
  //Potential scattering hits data (shit - scattered gamma hit)
  uint shit_1_index = 0;
  uint shit_2_index = 0;
  
  
  for (uint i = 0; i < hits.size(); i++) {
    //Skip annihilation gammas indexes
    if ( i == fhe.fAnniGamma1Index || i == fhe.fAnniGamma2Index ) {continue;}
    //Calculate delta1i and delta2i
    JPetHit hit_i = hits.at(i);
    double delta1i = caclulateDelta(ahit_1,hit_i);//[ns]
    double delta2i = caclulateDelta(ahit_2,hit_i);//[ns]
    if (dvd.IsInRight(delta1i,delta2i)) {shit_1_index = i;}
    else if (dvd.IsInLeft(delta1i,delta2i)) {shit_2_index = i;}
    
    //Fill histograms
    stats.fillHistogram("CF2G4H2SH_AllHits_DVD",delta1i,delta2i);
    
    if (shit_1_index == 0 || shit_2_index == 0) {continue;}
    
    JPetHit shit_1 = hits.at(shit_1_index);
    JPetHit shit_2 = hits.at(shit_2_index);
    fhe.fScatGamma1Index = shit_1_index;
    fhe.fScatGamma2Index = shit_2_index;
    fhe.fTheta1 = calculateScatteringAngle(ahit_1, shit_1);
    fhe.fTheta2 = calculateScatteringAngle(ahit_2, shit_2);
    fhe.fDeltaPhi = calculateDeltaPhi(ahit_1,ahit_2,shit_1,shit_2);
    //Fill histograms
    stats.fillHistogram("CF2G4H2SH_ScaHits_FoundIndexes",static_cast<double>(fhe.fScatGamma1Index),static_cast<double>(fhe.fScatGamma2Index));
    stats.fillHistogram("CF2G4H2SH_Theta1_vs_Theta2",fhe.fTheta1,fhe.fTheta2);
    stats.fillHistogram("CF2G4H2SH_Theta1",fhe.fTheta1);
    stats.fillHistogram("CF2G4H2SH_Theta2",fhe.fTheta2);
    stats.fillHistogram("CF2G4H2SH_DeltaPhi",fhe.fDeltaPhi);
    double delta11 = caclulateDelta(ahit_1,shit_1);//[ns]
    double delta21 = caclulateDelta(ahit_2,shit_1);//[ns]
    double delta12 = caclulateDelta(ahit_1,shit_2);//[ns]
    double delta22 = caclulateDelta(ahit_2,shit_2);//[ns]
    stats.fillHistogram("CF2G4H2SH_ScaHits_DVD",delta11,delta21);
    stats.fillHistogram("CF2G4H2SH_ScaHits_DVD",delta12,delta22);
    return true;
  }
  return false;  
}

bool EventCategorizerTools::checkFor2Gamma4HitsCircleCut(
  JPetStatistics& stats, const FourHitsEvent& fhe, const double& theta_radius
)
{
  if (IsInsideCircle(fhe.fTheta1,fhe.fTheta2,theta_radius))
  {
    std::string str_radius = std::to_string(static_cast<int>(theta_radius));
    std::string hname = "CF2G4HCC_DeltaPhi_Radius_"+str_radius;
    stats.fillHistogram(hname.c_str(),fhe.fDeltaPhi);
    hname = "CF2G4HCC_Theta1_vs_Theta2_Radius_"+str_radius;
    stats.fillHistogram(hname.c_str(),fhe.fTheta1,fhe.fTheta2);
    return true;
  }
  return false;
}

double EventCategorizerTools::calculateDeltaPhi(const JPetHit& ahit_1,const JPetHit& ahit_2, const JPetHit& shit_1, const JPetHit& shit_2)
{
  TVector3 apos = calculateAnnihilationPoint(ahit_1,ahit_2); //annihilation point
  TVector3 amom_1 = (ahit_1.getPos()-apos).Unit(); //annihilation gamma momentum direction
  TVector3 amom_2 = (ahit_2.getPos()-apos).Unit(); //annihilation gamma momentum direction
  TVector3 smom_1 = (shit_1.getPos()-ahit_1.getPos()).Unit(); //scattered gamma momentum direction
  TVector3 smom_2 = (shit_2.getPos()-ahit_2.getPos()).Unit(); //scattered gamma momentum direction
  TVector3 plane_nv_1 = amom_1.Cross(smom_1).Unit(); //scattering plane normal vector
  TVector3 plane_nv_2 = amom_2.Cross(smom_2).Unit(); //scattering plane normal vector
  return plane_nv_1.Angle(plane_nv_2) * TMath::RadToDeg();
}

double EventCategorizerTools::caclulateDelta(const JPetHit& ahit,const JPetHit& hit)
{
  return ns(fabs(calculateTOF(ahit,hit)) - calculateScatteringTime(ahit,hit));
}

double EventCategorizerTools::ns(double ps) { return ps/1000.0;}

bool EventCategorizerTools::IsInsideCircle(const double& theta1, const double& theta2, const double& theta_radius)
{
  double delta1 = theta1-81.6;
  double delta2 = theta2-81.6;
  return delta1*delta1 + delta2*delta2 <= theta_radius*theta_radius;
}

bool EventCategorizerTools::isInTOTRange(const double& tot, const double& tot_cut_min, const double& tot_cut_max)
{
  return tot_cut_min < tot && tot < tot_cut_max;
}


