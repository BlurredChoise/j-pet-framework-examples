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

#include <JPetMatrixSignal/JPetMatrixSignal.h>
#include "EventCategorizerTools.h"
#include <TMath.h>
#include <vector>

using namespace std;

/**
* Selecting pair of hits for calibrations based on TOT and Scin ID
*/
void EventCategorizerTools::selectForCalibration(
  const JPetEvent& event, JPetStatistics& stats, bool saveHistos, bool saveCalibHistos,
  double totCutAnniMin, double totCutAnniMax, double totCutDeexMin, double totCutDeexMax,
  boost::property_tree::ptree& calibTree
) {

  if (event.getHits().size() < 2) { return; }
  for (uint i = 0; i < event.getHits().size(); i++) {
    for (uint j = i + 1; j < event.getHits().size(); j++) {

      auto tot1 = event.getHits().at(i).getEnergy();
      auto tot2 = event.getHits().at(j).getEnergy();
      auto scin1ID = event.getHits().at(i).getScin().getID();
      auto scin2ID = event.getHits().at(j).getScin().getID();

      bool anih1 = false;
      bool anih2 = false;
      bool deex1 = false;
      bool deex2 = false;

      // Checking TOT of hits to classify them as annihilation or deexcitation
      if(tot1 > totCutAnniMin && tot1 < totCutAnniMax) { anih1 = true; }
      if(tot2 > totCutAnniMin && tot2 < totCutAnniMax) { anih2 = true; }
      if(tot1 > totCutDeexMin && tot1 < totCutDeexMax) { deex1 = true; }
      if(tot2 > totCutDeexMin && tot2 < totCutDeexMax) { deex2 = true; }

      // Time differences and strip ID to be assigned
      double aTime = 0.0, dTime = 0.0;
      int aScinID = -1, dScinID = -1;

      if(anih1 && deex2) {
        aScinID = event.getHits().at(i).getScin().getID();
        dScinID = event.getHits().at(j).getScin().getID();
        aTime = event.getHits().at(i).getTime();
        dTime = event.getHits().at(j).getTime();
      } else if(anih2 && deex1) {
        aScinID = event.getHits().at(j).getScin().getID();
        dScinID = event.getHits().at(i).getScin().getID();
        aTime = event.getHits().at(j).getTime();
        dTime = event.getHits().at(i).getTime();
      } else {
        continue;
      }

      double aTOFCorr = calibTree.get("scin."+to_string(aScinID)+".test_correction", 0.0);
      double dTOFCorr = calibTree.get("scin."+to_string(dScinID)+".test_correction", 0.0);

      double tDiff_A_D = aTime-dTime;
      double tDiff_A_D_corr = (aTime-aTOFCorr)-(dTime-dTOFCorr);

      // Filling histograms for specific scintillators
      if(saveHistos && saveCalibHistos && tDiff_A_D!=0.0 && aScinID!=-1 && dScinID!=-1) {
        stats.getHisto2D("tdiff_annih_scin")->Fill(aScinID, -tDiff_A_D);
        stats.getHisto2D("tdiff_deex_scin")->Fill(dScinID, tDiff_A_D);
        stats.getHisto2D("tdiff_annih_scin_corr")->Fill(aScinID, -tDiff_A_D_corr);
        stats.getHisto2D("tdiff_deex_scin_corr")->Fill(dScinID, tDiff_A_D_corr);
      }
    }
  }
}

/**
* Method for determining type of event - back to back 2 gamma
*/
bool EventCategorizerTools::checkFor2Gamma(
  const JPetEvent& event, JPetStatistics& stats, bool saveHistos, bool saveCalibHistos,
  double b2bSlotThetaDiff, double b2bTimeDiff, double totCutAnniMin, double totCutAnniMax,
  boost::property_tree::ptree& calibTree
) {
  if (event.getHits().size() < 2) { return false; }

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

      int slot1ID = firstHit.getScin().getSlot().getID();
      int slot2ID = secondHit.getScin().getSlot().getID();
      int scin1ID = firstHit.getScin().getID();
      int scin2ID = secondHit.getScin().getID();

      double timeDiff = fabs(firstHit.getTime() - secondHit.getTime());
      double theta1 = min(firstHit.getScin().getSlot().getTheta(), secondHit.getScin().getSlot().getTheta());
      double theta2 = max(firstHit.getScin().getSlot().getTheta(), secondHit.getScin().getSlot().getTheta());
      double thetaDiff = min(theta2 - theta1, 360.0 - theta2 + theta1);
      // Average TOT is temporaily stored as hit energy
      auto tot1 = firstHit.getEnergy();
      auto tot2 = secondHit.getEnergy();
      // Calculating reversed TOT for time walk studies
      auto revTOT1 = calculateReveresedTOT(firstHit);
      auto revTOT2 = calculateReveresedTOT(secondHit);

      // TOF calculated in several ways
      double tof = calculateTOF(firstHit.getTime(), secondHit.getTime());
      double tofConv = calculateTOFByConvention(firstHit, secondHit);
      double firstHitTOFCorr = calibTree.get("scin."+to_string(scin1ID)+".tof_correction", 0.0);
      double secondHitTOFCorr = calibTree.get("scin."+to_string(scin2ID)+".tof_correction", 0.0);
      double tofCorr = tof+firstHitTOFCorr;
      double tofConvCorr = tofConv;
      if(scin1ID < scin2ID) tofConvCorr += firstHitTOFCorr;
      else tofConvCorr -= firstHitTOFCorr;

      // Pre-cuts histograms
      if(saveHistos){
        stats.getHisto1D("2g_tdiff")->Fill(timeDiff);
        stats.getHisto1D("2g_tot")->Fill(tot1);
        stats.getHisto1D("2g_tot")->Fill(tot2);
        stats.getHisto2D("2g_tot_scin")->Fill(scin1ID, tot1);
        stats.getHisto2D("2g_tot_scin")->Fill(scin2ID, tot2);
        stats.getHisto2D("2g_revtot_scin")->Fill(scin1ID, revTOT1);
        stats.getHisto2D("2g_revtot_scin")->Fill(scin2ID, revTOT2);
        stats.getHisto1D("2g_tof_conv")->Fill(tof);
        stats.getHisto1D("2g_tof_conv_corr")->Fill(tofConv);
        stats.getHisto1D("2g_hit_tdiff")->Fill(firstHit.getTimeDiff());
        stats.getHisto1D("2g_hit_tdiff")->Fill(secondHit.getTimeDiff());
        stats.getHisto1D("2g_hit_scin")->Fill(scin1ID);
        stats.getHisto1D("2g_hit_scin")->Fill(scin2ID);
      }

      // Checking selection conditions
      bool angleCut1 = false, angleCut2 = false, tDiffCut = false, totCut = false;
      // Angular cut is performed to select hits in opposite modules
      // Can be done in general way (modules in 180 degree angle) more precise
      // - exactly opposite scintillator, based on ID difference (equal to 156)
      if(fabs(thetaDiff - 180.0) < b2bSlotThetaDiff) {
        angleCut1 = true;
        if(saveHistos){
          stats.getHisto1D("cut_stats_a1")->Fill(scin1ID);
          stats.getHisto1D("cut_stats_a1")->Fill(scin2ID);
          stats.getHisto2D(Form("time_walk_scin_%d", scin1ID))->Fill(tofConvCorr, revTOT1);
          stats.getHisto2D(Form("time_walk_scin_%d", scin2ID))->Fill(tofConvCorr, revTOT2);
        }
      }
      int idDiff = max(scin1ID - scin2ID, scin2ID - scin1ID);
      if(idDiff > 154 && idDiff < 158) {
        angleCut2 = true;
        if(saveHistos){
          stats.getHisto1D("cut_stats_a2")->Fill(scin1ID);
          stats.getHisto1D("cut_stats_a2")->Fill(scin2ID);
        }
      }

      // Time difference cut
      if(timeDiff < b2bTimeDiff) {
        tDiffCut = true;
        if(saveHistos){
          stats.getHisto1D("cut_stats_tdiff")->Fill(scin1ID);
          stats.getHisto1D("cut_stats_tdiff")->Fill(scin2ID);
        }
      }

      // TOT cut
      if(tot1 > totCutAnniMin && tot1 < totCutAnniMax && tot2 > totCutAnniMin && tot2 < totCutAnniMax) {
        totCut = true;
        if(saveHistos){
          stats.getHisto1D("cut_stats_tot")->Fill(scin1ID);
          stats.getHisto1D("cut_stats_tot")->Fill(scin2ID);
        }
      }

      if(angleCut2 && tDiffCut && totCut){

        if(saveHistos && saveCalibHistos){
          stats.getHisto1D("ap_hit_tdiff")->Fill(firstHit.getTimeDiff());
          stats.getHisto1D("ap_hit_tdiff")->Fill(secondHit.getTimeDiff());
          stats.getHisto1D("ap_tof")->Fill(tof);
          stats.getHisto1D("ap_tof_conv")->Fill(tofConv);
          stats.getHisto1D("ap_tof_corr")->Fill(tofCorr);
          stats.getHisto1D("ap_tof_conv_corr")->Fill(tofConvCorr);

          stats.getHisto2D("ap_revtot_scin")->Fill(scin1ID, revTOT1);
          stats.getHisto2D("ap_revtot_scin")->Fill(scin2ID, revTOT2);

          stats.getHisto2D("ap_tof_scin")->Fill(scin1ID, tof);
          stats.getHisto2D("ap_tof_scin")->Fill(scin2ID, tof);
          stats.getHisto2D("ap_tof_conv_scin")->Fill(scin1ID, tofConv);
          stats.getHisto2D("ap_tof_conv_scin")->Fill(scin2ID, tofConv);
          stats.getHisto2D("ap_tof_corr_scin")->Fill(scin1ID, tofCorr);
          stats.getHisto2D("ap_tof_corr_scin")->Fill(scin2ID, tofCorr);
          stats.getHisto2D("ap_tof_conv_corr_scin")->Fill(scin1ID, tofConvCorr);
          stats.getHisto2D("ap_tof_conv_corr_scin")->Fill(scin2ID, tofConvCorr);

          stats.getHisto2D("ap_tof_conv_corr_z")->Fill(firstHit.getPosZ(), tofConvCorr);
          stats.getHisto2D("ap_tof_conv_corr_z")->Fill(secondHit.getPosZ(), tofConvCorr);
          TVector3 annhilationPoint = calculateAnnihilationPoint(firstHit, secondHit);
          stats.getHisto2D("ap_xy")->Fill(annhilationPoint.X(), annhilationPoint.Y());
          stats.getHisto2D("ap_xz")->Fill(annhilationPoint.X(), annhilationPoint.Z());
          stats.getHisto2D("ap_yz")->Fill(annhilationPoint.Y(), annhilationPoint.Z());
          stats.getHisto2D("ap_xy_zoom")->Fill(annhilationPoint.X(), annhilationPoint.Y());
          stats.getHisto2D("ap_xz_zoom")->Fill(annhilationPoint.X(), annhilationPoint.Z());
          stats.getHisto2D("ap_yz_zoom")->Fill(annhilationPoint.Y(), annhilationPoint.Z());

          for(auto& rawSig : firstHit.getSignalA().getRawSignals()){
            if(rawSig.second.getTimesVsThresholdNumber(JPetSigCh::Leading).size()==2) {
              int pmID = rawSig.second.getPM().getID();
              double thrTimeDiff = rawSig.second.getTimesVsThresholdNumber(JPetSigCh::Leading).at(2)-rawSig.second.getTimesVsThresholdNumber(JPetSigCh::Leading).at(1);
              stats.getHisto2D("ap_sig_thr_tdiff")->Fill(pmID, thrTimeDiff);
            }
          }
          for(auto& rawSig : firstHit.getSignalB().getRawSignals()){
            if(rawSig.second.getTimesVsThresholdNumber(JPetSigCh::Leading).size()==2) {
              int pmID = rawSig.second.getPM().getID();
              double thrTimeDiff = rawSig.second.getTimesVsThresholdNumber(JPetSigCh::Leading).at(2)-rawSig.second.getTimesVsThresholdNumber(JPetSigCh::Leading).at(1);
              stats.getHisto2D("ap_sig_thr_tdiff")->Fill(pmID, thrTimeDiff);
            }
          }
          for(auto& rawSig : secondHit.getSignalA().getRawSignals()){
            if(rawSig.second.getTimesVsThresholdNumber(JPetSigCh::Leading).size()==2) {
              int pmID = rawSig.second.getPM().getID();
              double thrTimeDiff = rawSig.second.getTimesVsThresholdNumber(JPetSigCh::Leading).at(2)-rawSig.second.getTimesVsThresholdNumber(JPetSigCh::Leading).at(1);
              stats.getHisto2D("ap_sig_thr_tdiff")->Fill(pmID, thrTimeDiff);
            }
          }
          for(auto& rawSig : secondHit.getSignalB().getRawSignals()){
            if(rawSig.second.getTimesVsThresholdNumber(JPetSigCh::Leading).size()==2) {
              int pmID = rawSig.second.getPM().getID();
              double thrTimeDiff = rawSig.second.getTimesVsThresholdNumber(JPetSigCh::Leading).at(2)-rawSig.second.getTimesVsThresholdNumber(JPetSigCh::Leading).at(1);
              stats.getHisto2D("ap_sig_thr_tdiff")->Fill(pmID, thrTimeDiff);
            }
          }
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
        thetaAngles.push_back(firstHit.getScin().getSlot().getTheta());
        thetaAngles.push_back(secondHit.getScin().getSlot().getTheta());
        thetaAngles.push_back(thirdHit.getScin().getSlot().getTheta());
        sort(thetaAngles.begin(), thetaAngles.end());

        vector<double> relativeAngles;
        relativeAngles.push_back(thetaAngles.at(1) - thetaAngles.at(0));
        relativeAngles.push_back(thetaAngles.at(2) - thetaAngles.at(1));
        relativeAngles.push_back(360.0 - thetaAngles.at(2) + thetaAngles.at(0));
        sort(relativeAngles.begin(), relativeAngles.end());
        double transformedX = relativeAngles.at(1) + relativeAngles.at(0);
        double transformedY = relativeAngles.at(1) - relativeAngles.at(0);

        if (saveHistos) {
          stats.getHisto2D("3Gamma_Angles")->Fill(transformedX, transformedY);
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
  double deexTOTCutMin, double deexTOTCutMax
) {
  for (unsigned i = 0; i < event.getHits().size(); i++) {
    double tot = calculateTOT(event.getHits().at(i));
    if (tot > deexTOTCutMin && tot < deexTOTCutMax) {
      if (saveHistos) {
        stats.getHisto1D("Deex_TOT_cut")->Fill(tot);
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
  const JPetEvent& event, JPetStatistics& stats, bool saveHistos, double scatterTOFTimeDiff
) {
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
        stats.getHisto1D("ScatterTOF_TimeDiff")->Fill(fabs(scattTOF - timeDiff));
      }

      if (fabs(scattTOF - timeDiff) < scatterTOFTimeDiff) {
        if (saveHistos) {
          stats.getHisto2D("ScatterAngle_PrimaryTOT")->Fill(scattAngle, calculateTOT(primaryHit));
          stats.getHisto2D("ScatterAngle_ScatterTOT")->Fill(scattAngle, calculateTOT(scatterHit));
        }
        return true;
      }
    }
  }
  return false;
}

/**
* Calculation of the total TOT of the hit - Time over Threshold:
* the sum of the TOTs on all of the thresholds and on the both sides (A,B)
*/
double EventCategorizerTools::calculateTOT(const JPetHit& hit)
{
  auto multi = hit.getSignalA().getRawSignals().size() + hit.getSignalB().getRawSignals().size();
  return hit.getEnergy()/((double) multi);
}

/**
* Calculation of the total reversed TOT of the hit. It is defined as:
* revTOT_hit = revTOT_sigB - revTOT_sigA. Reversed matrix signal TOT is defined as:
* revTOT_sig = 1/tot_simp1 + 1/tot_simp2 + 1/tot_simp3 + 1/tot_simp4;
*/
double EventCategorizerTools::calculateReveresedTOT(const JPetHit& hit)
{
  double revTOT_sigA = 0.0, revTOT_sigB = 0.0;
  for(auto& rawSig : hit.getSignalA().getRawSignals()){
    for(auto& thrTOT : rawSig.second.getTOTsVsThresholdNumber()) {
      if(thrTOT.second != 0.0) { revTOT_sigA += 1.0/thrTOT.second; }
    }
  }

  for(auto& rawSig : hit.getSignalB().getRawSignals()){
    for(auto& thrTOT : rawSig.second.getTOTsVsThresholdNumber()) {
      if(thrTOT.second != 0.0) { revTOT_sigB += 1.0/thrTOT.second; }
    }
  }

  return revTOT_sigB-revTOT_sigA;
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
  TVector3 versorOnLOR = (hitB - hitA).Unit();

  double shift = 0.5 * tof  * kLightVelocity_cm_ps;
  TVector3 annihilationPoint(
    middleOfLOR.X() + shift * versorOnLOR.X(),
    middleOfLOR.Y() + shift * versorOnLOR.Y(),
    middleOfLOR.Z() + shift * versorOnLOR.Z()
  );
  return annihilationPoint;
}

double EventCategorizerTools::calculateTOFByConvention(const JPetHit& hitA, const JPetHit& hitB)
{
  if (hitA.getScin().getSlot().getTheta() < hitB.getScin().getSlot().getTheta()) {
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

/**
* Method for determining type of event for streaming - 2 gamma
* @todo: the selection criteria b2b distance from center needs to be checked
* and implemented again
*/
bool EventCategorizerTools::stream2Gamma(
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
      double theta1 = min(firstHit.getScin().getSlot().getTheta(), secondHit.getScin().getSlot().getTheta());
      double theta2 = max(firstHit.getScin().getSlot().getTheta(), secondHit.getScin().getSlot().getTheta());
      double thetaDiff = min(theta2 - theta1, 360.0 - theta2 + theta1);
      if (saveHistos) {
        stats.getHisto1D("2Gamma_TimeDiff")->Fill(timeDiff / 1000.0);
        stats.getHisto1D("2Gamma_DLOR")->Fill(deltaLor);
        stats.getHisto1D("2Gamma_ThetaDiff")->Fill(thetaDiff);
      }
      if (fabs(thetaDiff - 180.0) < b2bSlotThetaDiff && timeDiff < b2bTimeDiff) {
        if (saveHistos) {
          TVector3 annhilationPoint = calculateAnnihilationPoint(firstHit, secondHit);
          stats.getHisto1D("2Annih_TimeDiff")->Fill(timeDiff / 1000.0);
          stats.getHisto1D("2Annih_DLOR")->Fill(deltaLor);
          stats.getHisto1D("2Annih_ThetaDiff")->Fill(thetaDiff);
          stats.getHisto2D("2Annih_XY")->Fill(annhilationPoint.X(), annhilationPoint.Y());
          stats.getHisto1D("2Annih_Z")->Fill(annhilationPoint.Z());
        }
        return true;
      }
    }
  }
  return false;
}

/**
* Method for determining type of event for streaming - 3 gamma annihilation
*/
bool EventCategorizerTools::stream3Gamma(
  const JPetEvent& event, JPetStatistics& stats, bool saveHistos,
  double d3SlotThetaMin, double d3TimeDiff, double d3PlaneCenterDist
)
{
  if (event.getHits().size() < 3) {
    return false;
  }
  for (uint i = 0; i < event.getHits().size(); i++) {
    for (uint j = i + 1; j < event.getHits().size(); j++) {
      for (uint k = j + 1; k < event.getHits().size(); k++) {
        JPetHit firstHit = event.getHits().at(i);
        JPetHit secondHit = event.getHits().at(j);
        JPetHit thirdHit = event.getHits().at(k);

        vector<double> thetaAngles;
        thetaAngles.push_back(firstHit.getScin().getSlot().getTheta());
        thetaAngles.push_back(secondHit.getScin().getSlot().getTheta());
        thetaAngles.push_back(thirdHit.getScin().getSlot().getTheta());
        sort(thetaAngles.begin(), thetaAngles.end());

        vector<double> relativeAngles;
        relativeAngles.push_back(thetaAngles.at(1) - thetaAngles.at(0));
        relativeAngles.push_back(thetaAngles.at(2) - thetaAngles.at(1));
        relativeAngles.push_back(360.0 - thetaAngles.at(2) + thetaAngles.at(0));
        sort(relativeAngles.begin(), relativeAngles.end());

        double transformedX = relativeAngles.at(1) + relativeAngles.at(0);
        double transformedY = relativeAngles.at(1) - relativeAngles.at(0);
        double timeDiff = fabs(thirdHit.getTime() - firstHit.getTime());
        double planeCenterDist = calculatePlaneCenterDistance(firstHit, secondHit, thirdHit);
        if (saveHistos) {
          stats.getHisto1D("3GammaTimeDiff")->Fill(timeDiff);
          stats.getHisto2D("3GammaThetas")->Fill(transformedX, transformedY);
          stats.getHisto1D("3GammaPlaneDist")->Fill(planeCenterDist);
        }
        if (transformedX > d3SlotThetaMin && timeDiff < d3TimeDiff && planeCenterDist < d3PlaneCenterDist) {
          if (saveHistos) {
            stats.getHisto1D("3AnnihPlaneDist")->Fill(planeCenterDist);
            stats.getHisto1D("3AnnihTimeDiff")->Fill(timeDiff);
          }
          return true;
        }
      }
    }
  }
  return false;
}