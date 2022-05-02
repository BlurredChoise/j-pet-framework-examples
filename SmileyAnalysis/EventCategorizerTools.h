/**
 *  @copyright Copyright 2020 The J-PET Framework Authors. All rights reserved.
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may find a copy of the License in the LICENCE file.
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  @file EventCategorizerTools.h
 */

#ifndef EVENTCATEGORIZERTOOLS_H
#define EVENTCATEGORIZERTOOLS_H

#include <JPetStatistics/JPetStatistics.h>
#include <JPetEvent/JPetEvent.h>
#include <JPetHit/JPetHit.h>
#include<vector>

static const double kLightVelocity_cm_ps = 0.0299792458;
static const double kUndefinedValue = 999.0;

class Ellipse
{
 public:
  bool isInside( const double& x, const double& y ) const;
  void setParamaters( double x0, double y0, double rx, double ry, double theta );

 private:
  double fXCenter = 0;
  double fYCenter = 0;
  double fCosTheta = 0;
  double fSinTheta = 0;
  double fRadiusX = 0;
  double fRadiusY = 0;
};

struct FourHitsEvent
{
  uint fAnniGamma1Index = 0;
  uint fAnniGamma2Index = 0;
  uint fScatGamma1Index = 0;
  uint fScatGamma2Index = 0;
  double fTheta1 = 0.0;
  double fTheta2 = 0.0;
  double fDeltaPhi = 0.0;
};

struct DVDFilter
{
  Ellipse fRightEllipse;
  Ellipse fLeftEllipse;
  bool IsInRight(const double& theta1i, const double& theta2i) const;
  bool IsInLeft(const double& theta1i, const double& theta2i) const;
};

struct AnalysisParams
{
  double fScatterTOFTimeDiff;//[ps]
	double fB2BSlotThetaDiff;//[deg]
	double fDeexTOTCutMin;//[ps]
	double fDeexTOTCutMax;//[ps]
	double fMaxTimeDiff;//[ps]
  double fAnnihTOTCutMin;//[ps]
  double fAnnihTOTCutMax;//[ps]
  double fAnnihRadiusXY;//[cm]
  double fAnnihZPosDelta;//[cm]
  std::string fTOTCalculationType;
};

/**
 * @brief Tools for Event Categorization
 *
 * Lots of tools in constatnt developement.
*/
class EventCategorizerTools
{
public:  
  static bool checkFor2Gamma(const JPetEvent& event, JPetStatistics& stats,
                           bool saveHistos, double b2bSlotThetaDiff, double b2bTimeDiff);
  static bool checkFor3Gamma(const JPetEvent& event, JPetStatistics& stats, bool saveHistos);
  static bool checkForPrompt(const JPetEvent& event, JPetStatistics& stats,
                             bool saveHistos, double deexTOTCutMin, double deexTOTCutMax, 
                             std::string fTOTCalculationType);
  static bool checkForScatter(const JPetEvent& event, JPetStatistics& stats,
                              bool saveHistos, double scatterTOFTimeDiff, 
                              std::string fTOTCalculationType);
  static double calculateDistance(const JPetHit& hit1, const JPetHit& hit2);
  static double calculateScatteringTime(const JPetHit& hit1, const JPetHit& hit2);
  static double calculateScatteringAngle(const JPetHit& hit1, const JPetHit& hit2);
  /// Tof is calculated as  time1 -time2.
  static double calculateTOF(const JPetHit& hitA, const JPetHit& hitB);
  static double calculateTOF(double time1, double time2);
  /// Tof calculated with the ordered hits with respect to scintillator number.
  /// The first one will be hit with smaller theta angle.
  /// See also: http://koza.if.uj.edu.pl/petwiki/index.php/Coordinate_system_in_Big_Barrel

  // cppcheck-suppress unusedFunction
  static double calculateTOFByConvention(const JPetHit& hitA, const JPetHit& hitB);
  static TVector3 calculateAnnihilationPoint(const JPetHit& hitA, const JPetHit& hitB);
  static TVector3 calculateAnnihilationPoint(const TVector3& hitA, const TVector3& hitB, double tof);
  static double calculatePlaneCenterDistance(const JPetHit& firstHit,
      const JPetHit& secondHit, const JPetHit& thirdHit);
  //4 hits methods
  static bool checkFor2Gamma4Hits(const std::vector<JPetHit>& hits, JPetStatistics& stats, FourHitsEvent& fhe, const AnalysisParams& ap);
  static bool checkFor2Gamma4Hits2ScatteringHits(const std::vector<JPetHit>& hits, JPetStatistics& stats, FourHitsEvent& fhe, const DVDFilter& dvd);
  static double calculateDeltaPhi(const JPetHit& ahit_1,const JPetHit& ahit_2, const JPetHit& shit_1, const JPetHit& shit_2);
  static double ns(double ps);
  static double caclulateDelta(const JPetHit& ahit,const JPetHit& hit);
  static bool IsInsideCircle(const double& theta1, const double& theta2, const double& theta_radius);
  static bool checkFor2Gamma4HitsCircleCut(JPetStatistics& stats, const FourHitsEvent& fhe, const double& theta_radius);
  
  static std::vector<JPetHit> getHitsFor4HitsAnalysis(JPetStatistics& stats,const JPetEvent& event, const AnalysisParams& ap);
  static bool isInTOTRange(const double& tot, const double& tot_cut_min, const double& tot_cut_max);
  static bool notPassed(JPetStatistics& stats,const std::string& eff_obj_name,const uint& selection_id,bool condition);

};

#endif /* !EVENTCATEGORIZERTOOLS_H */
