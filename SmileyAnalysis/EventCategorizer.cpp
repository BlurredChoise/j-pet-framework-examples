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
 *  @file EventCategorizer.cpp
 */

#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetWriter/JPetWriter.h>
#include "EventCategorizerTools.h"
#include "EventCategorizer.h"
#include <iostream>

using namespace jpet_options_tools;
using namespace std;
typedef EventCategorizerTools ECT;

EventCategorizer::EventCategorizer(const char* name): JPetUserTask(name) {}

EventCategorizer::~EventCategorizer() {}

bool EventCategorizer::init()
{
  INFO("Event categorization started.");
  // Parameter for back to back categorization
  if (isOptionSet(fParams.getOptions(), kBack2BackSlotThetaDiffParamKey)){
    fB2BSlotThetaDiff = getOptionAsFloat(fParams.getOptions(), kBack2BackSlotThetaDiffParamKey);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of %lf.",
      kBack2BackSlotThetaDiffParamKey.c_str(), fB2BSlotThetaDiff
    ));
  }
  // Parameter for scattering determination
  if (isOptionSet(fParams.getOptions(), kScatterTOFTimeDiffParamKey)) {
    fScatterTOFTimeDiff = getOptionAsFloat(fParams.getOptions(), kScatterTOFTimeDiffParamKey);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of %lf.",
      kScatterTOFTimeDiffParamKey.c_str(), fScatterTOFTimeDiff
    ));
  }
  // Parameters for deexcitation TOT cut
  if (isOptionSet(fParams.getOptions(), kDeexTOTCutMinParamKey)) {
    fDeexTOTCutMin = getOptionAsFloat(fParams.getOptions(), kDeexTOTCutMinParamKey);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of %lf.",
      kDeexTOTCutMinParamKey.c_str(), fDeexTOTCutMin
    ));
  }
  if (isOptionSet(fParams.getOptions(), kDeexTOTCutMaxParamKey)) {
    fDeexTOTCutMax = getOptionAsFloat(fParams.getOptions(), kDeexTOTCutMaxParamKey);
  } else {
    WARNING(Form(
      "No value of the %s parameter provided by the user. Using default value of %lf.",
      kDeexTOTCutMaxParamKey.c_str(), fDeexTOTCutMax
    ));
  }
  if (isOptionSet(fParams.getOptions(), kMaxTimeDiffParamKey)) {
    fMaxTimeDiff = getOptionAsFloat(fParams.getOptions(), kMaxTimeDiffParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kMaxTimeDiffParamKey.c_str(), fMaxTimeDiff));
  }
  // Getting bool for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey)) {
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }
  if (isOptionSet(fParams.getOptions(), kTOTCalculationType)) {
    fTOTCalculationType = getOptionAsString(fParams.getOptions(), kTOTCalculationType);
  } else {
    WARNING("No TOT calculation option given by the user. Using standard sum.");
  }
  
  //Parameters for DeltaVsDelta filter
  ////Left ellipse
  if (isOptionSet(fParams.getOptions(), kEllipseLeftX0)) {
    fEllipseLeftX0 = getOptionAsFloat(fParams.getOptions(), kEllipseLeftX0);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kEllipseLeftX0.c_str(), fEllipseLeftX0));
  }
  if (isOptionSet(fParams.getOptions(), kEllipseLeftY0)) {
    fEllipseLeftY0 = getOptionAsFloat(fParams.getOptions(), kEllipseLeftY0);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kEllipseLeftY0.c_str(), fEllipseLeftY0));
  }
  if (isOptionSet(fParams.getOptions(), kEllipseLeftRX)) {
    fEllipseLeftRX = getOptionAsFloat(fParams.getOptions(), kEllipseLeftRX);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kEllipseLeftRX.c_str(), fEllipseLeftRX));
  }
  if (isOptionSet(fParams.getOptions(), kEllipseLeftRY)) {
    fEllipseLeftRY = getOptionAsFloat(fParams.getOptions(), kEllipseLeftRY);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kEllipseLeftRY.c_str(), fEllipseLeftRY));
  }
  if (isOptionSet(fParams.getOptions(), kEllipseLeftRotAngle)) {
    fEllipseLeftRotAngle = getOptionAsFloat(fParams.getOptions(), kEllipseLeftRotAngle);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kEllipseLeftRotAngle.c_str(), fEllipseLeftRotAngle));
  }
  ////Right ellipse
  if (isOptionSet(fParams.getOptions(), kEllipseRightX0)) {
    fEllipseRightX0 = getOptionAsFloat(fParams.getOptions(), kEllipseRightX0);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kEllipseRightX0.c_str(), fEllipseRightX0));
  }
  if (isOptionSet(fParams.getOptions(), kEllipseRightY0)) {
    fEllipseRightY0 = getOptionAsFloat(fParams.getOptions(), kEllipseRightY0);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kEllipseRightY0.c_str(), fEllipseRightY0));
  }
  if (isOptionSet(fParams.getOptions(), kEllipseRightRX)) {
    fEllipseRightRX = getOptionAsFloat(fParams.getOptions(), kEllipseRightRX);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kEllipseRightRX.c_str(), fEllipseRightRX));
  }
  if (isOptionSet(fParams.getOptions(), kEllipseRightRY)) {
    fEllipseRightRY = getOptionAsFloat(fParams.getOptions(), kEllipseRightRY);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kEllipseRightRY.c_str(), fEllipseRightRY));
  }
  if (isOptionSet(fParams.getOptions(), kEllipseRightRotAngle)) {
    fEllipseRightRotAngle = getOptionAsFloat(fParams.getOptions(), kEllipseRightRotAngle);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kEllipseRightRotAngle.c_str(), fEllipseRightRotAngle));
  }
  
  if (isOptionSet(fParams.getOptions(), kAnnihTOTCutMin)) {
    fAnnihTOTCutMin = getOptionAsFloat(fParams.getOptions(), kAnnihTOTCutMin);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kAnnihTOTCutMin.c_str(), fAnnihTOTCutMin));
  }
  if (isOptionSet(fParams.getOptions(), kAnnihTOTCutMax)) {
    fAnnihTOTCutMax = getOptionAsFloat(fParams.getOptions(), kAnnihTOTCutMax);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kAnnihTOTCutMax.c_str(), fAnnihTOTCutMax));
  }
  if (isOptionSet(fParams.getOptions(), kAnnihRadiusXY)) {
    fAnnihRadiusXY = getOptionAsFloat(fParams.getOptions(), kAnnihRadiusXY);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kAnnihRadiusXY.c_str(), fAnnihRadiusXY));
  }
  if (isOptionSet(fParams.getOptions(), kAnnihZPosDelta)) {
    fAnnihZPosDelta = getOptionAsFloat(fParams.getOptions(), kAnnihZPosDelta);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kAnnihZPosDelta.c_str(), fAnnihZPosDelta));
  }
  if (isOptionSet(fParams.getOptions(), kNoiseTOTCut)) {
    fNoiseTOTCut = getOptionAsFloat(fParams.getOptions(), kNoiseTOTCut);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.", kNoiseTOTCut.c_str(), fNoiseTOTCut));
  }

  //Analysis params
  fAParams.fScatterTOFTimeDiff = fScatterTOFTimeDiff;
	fAParams.fB2BSlotThetaDiff = fB2BSlotThetaDiff;
	fAParams.fDeexTOTCutMin = fDeexTOTCutMin;//[ps]
	fAParams.fDeexTOTCutMax = fDeexTOTCutMax;//[ps]
	fAParams.fMaxTimeDiff = fMaxTimeDiff;//[ps]
  fAParams.fAnnihTOTCutMin = fAnnihTOTCutMin;//[ps]
  fAParams.fAnnihTOTCutMax = fAnnihTOTCutMax;//[ps]
  fAParams.fAnnihRadiusXY = fAnnihRadiusXY;//[cm]
  fAParams.fAnnihZPosDelta = fAnnihZPosDelta;//[cm]
  fAParams.fTOTCalculationType = fTOTCalculationType;
  fAParams.fNoiseTOTCut = fNoiseTOTCut;//[ps]

  // Input events type
  fOutputEvents = new JPetTimeWindow("JPetEvent");
  // Initialise hisotgrams
  if(fSaveControlHistos) initialiseHistograms();
  return true;
}

bool EventCategorizer::exec()
{
  DVDFilter dvd;
  dvd.fRightEllipse.setParamaters(fEllipseRightX0,fEllipseRightY0,fEllipseRightRX,fEllipseRightRY,fEllipseRightRotAngle);
  dvd.fLeftEllipse.setParamaters(fEllipseLeftX0,fEllipseLeftY0,fEllipseLeftRX,fEllipseLeftRY,fEllipseLeftRotAngle);
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    vector<JPetEvent> events;
    for (uint i = 0; i < timeWindow->getNumberOfEvents(); i++) {
      const auto& event = dynamic_cast<const JPetEvent&>(timeWindow->operator[](i));
      //First extract only useful hits for 2+2 analysis (not prompt,not noise and in range |z|<23 cm)
      auto hits = ECT::getHitsFor4HitsAnalysis(getStatistics(),event,fAParams);
      FourHitsEvent fhe;
      //Condition 1: We have at least 4 hits
      if (ECT::notPassed(getStatistics(),"effGeneralSelection",0,ECT::checkHitsMinNumber(getStatistics(),hits, 4))) { continue;}
      //Condition 2: We found annihilation gammas
      if (ECT::notPassed(getStatistics(),"effGeneralSelection",1,ECT::checkFor2Gamma4Hits2AnnihilationHits(hits, getStatistics(), fhe,fAParams))) { continue;}
      //Condition 3: We have found scattering hits for each annihilation gamma
      if (ECT::notPassed(getStatistics(),"effGeneralSelection",2,ECT::checkFor2Gamma4Hits2ScatteringHits(hits, getStatistics(), fhe, dvd))) {continue;}
      //Condition 4: Scattering angles are in a circle with radiu 30 deg around (81.6,81.6) deg
      if (ECT::notPassed(getStatistics(),"effGeneralSelection",3,ECT::checkFor2Gamma4HitsCircleCut(getStatistics(), fhe, 30.0))) {continue;}
      //Condition 5: Scattering angles are in a circle with radiu 10 deg around (81.6,81.6) deg
      if (ECT::notPassed(getStatistics(),"effGeneralSelection",4,ECT::checkFor2Gamma4HitsCircleCut(getStatistics(), fhe, 10.0))) {continue;}
    }
  } else { return false; }
  return true;
}

bool EventCategorizer::terminate()
{
  INFO("Event categorization completed.");
  return true;
}

void EventCategorizer::saveEvents(const vector<JPetEvent>& events)
{
  for (const auto& event : events) { fOutputEvents->add<JPetEvent>(event); }
}

void EventCategorizer::initialiseHistograms(){
  //Efficiecny objects
  //// preparing hits
  getStatistics().createObject(
    new TEfficiency("effHitsPreparation","Hits preparation;Selection;#epsilon",3,-0.5,2.5)
  );
  getStatistics().createObject(
    new TEfficiency("effGeneralSelection","General program selection workflow;Selection;#epsilon",5,-0.5,4.5)
  );
  getStatistics().createObject(
    new TEfficiency("effAnnHitsFinding","Finding annhilation hits;Selection;#epsilon",5,-0.5,4.5)
  );
  //CHMN_FoundHitsSize
  getStatistics().createHistogramWithAxes(
    new TH1D("CHMN_FoundHitsSize", "Hits number", 20, -0.5, 19.5),
    "Hits number", "Counts"
  );
  //GHF4HA - getHitsFor4HitsAnalysis
  getStatistics().createHistogramWithAxes(
    new TH1D("GHF4HA_All_TOT", "TOT", 600, 0.0, 60),
    "TOT [ps]", "Counts"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("GHF4HA_Passed_TOT", "TOT", 600, 0.0, 60),
    "TOT [ps]", "Counts"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("GHF4HA_Zpos", "Zpos", 500, -50.0, 50.0),
    "Z axis position [cm]", "Counts"
  );
  //CF2G4H - histograms filled in ECT::checkFor2Gamma4Hits
  //// All hits
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4H_AllHits_TimeDiff", "Time difference of 2 gamma hits", 200, 0.0, 99900.0),
    "Time Difference [ps]", "Number of Hit Pairs"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4H_AllHits_ThetaDiff", "Angle difference of 2 gamma hits ", 181, -0.5, 180.5),
    "Hits theta diff [deg]", "Counts"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4H_AllHits_DeltaLOR", "Delta LOR distance", 100, -0.25, 49.25),
    "Delta LOR [cm]", "Counts"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4H_AllHits_Zpos", "Z-axis position of 2 gamma hits", 201, -50.25, 50.25),
    "Z axis position [cm]", "Number of Hits"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4H_AllHits_Dist", "B2B hits distance", 150, -0.5, 149.5),
    "Distance [cm]", "Number of Hit Pairs"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("CF2G4H_AllHits_XYpos", "Hit position XY", 240, -60.25, 59.75, 240, -60.25, 59.75),
    "Hit X position [cm]", "Hit Y position [cm]"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4H_AllHits_TOT", "TOT", 600, 0.0, 60),
    "TOT [ns]", "Counts"
  );
  ////Preannihilation hits - good candidates but have to meet annhilation point cut conditions
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4H_PreAnnHits_TimeDiff", "Time difference of 2 gamma hits", 200, 0.0, fMaxTimeDiff),
    "Time Difference [ps]", "Number of Hit Pairs"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4H_PreAnnHits_ThetaDiff", "Angle difference of 2 gamma hits ", 181, -0.5, 180.5),
    "Hits theta diff [deg]", "Counts"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4H_PreAnnHits_DeltaLOR", "Delta LOR distance", 100, -0.25, 49.25),
    "Delta LOR [cm]", "Counts"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4H_PreAnnHits_Zpos", "Z-axis position of 2 gamma hits", 201, -50.25, 50.25),
    "Z axis position [cm]", "Number of Hits"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4H_PreAnnHits_Dist", "B2B hits distance", 150, -0.5, 149.5),
    "Distance [cm]", "Number of Hit Pairs"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("CF2G4H_PreAnnHits_XYpos", "Hit position XY", 240, -60.25, 59.75, 240, -60.25, 59.75),
    "Hit X position [cm]", "Hit Y position [cm]"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("CF2G4H_PreAnnHits_FoundIndexes", "Found indexes for annhilation gammas", 9,-0.5,9.5,9,-0.5,9.5),
    "First a-gamma index", "Second a-gamma index"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("CF2G4H_PreAnnHits_AnnHitPosXY", "Reconstructed XY position of annihilation point", 1200,-60.0,60.0,1200,-60.0,60.0),
    "Annhilation point X [cm]", "Annhilation point Y [cm]"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4H_PreAnnHits_AnnHitPosZ", "Annhilation point Z position", 200, -10.0, 10.0),
    "Annihilation point Z position [cm]", "Counts"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4H_PreAnnHits_TOT", "TOT", 600, 0.0, 60),
    "TOT [ns]", "Counts"
  );
  ////Annihilation gammas hits
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4H_AnnHits_TimeDiff", "Time difference of 2 gamma hits", 200, 0.0, fMaxTimeDiff),
    "Time Difference [ps]", "Number of Hit Pairs"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4H_AnnHits_ThetaDiff", "Angle difference of 2 gamma hits ", 181, -0.5, 180.5),
    "Hits theta diff [deg]", "Counts"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4H_AnnHits_DeltaLOR", "Delta LOR distance", 100, -0.25, 49.25),
    "Delta LOR [cm]", "Counts"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4H_AnnHits_Zpos", "Z-axis position of 2 gamma hits", 201, -50.25, 50.25),
    "Z axis position [cm]", "Number of Hits"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4H_AnnHits_Dist", "B2B hits distance", 150, -0.5, 149.5),
    "Distance [cm]", "Number of Hit Pairs"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("CF2G4H_AnnHits_XYpos", "Hit position XY", 240, -60.25, 59.75, 240, -60.25, 59.75),
    "Hit X position [cm]", "Hit Y position [cm]"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("CF2G4H_AnnHits_FoundIndexes", "Found indexes for annhilation gammas", 9,-0.5,9.5,9,-0.5,9.5),
    "First a-gamma index", "Second a-gamma index"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("CF2G4H_AnnHits_AnnHitPosXY", "Reconstructed XY position of annihilation point", 100,-5.0,5.0,100,-5.0,5.0),
    "Annhilation point X [cm]", "Annhilation point Y [cm]"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4H_AnnHits_AnnHitPosZ", "Annhilation point Z position", 200, -10.0, 10.0),
    "Annihilation point Z position [cm]", "Counts"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4H_AnnHits_TOT", "TOT", 600, 0.0, 60),
    "TOT [ns]", "Counts"
  );
  //CF2G4H2SH - histograms filled in ECT::checkFor2Gamma4Hits2ScatteringHits
  //// All hits
  getStatistics().createHistogramWithAxes(
    new TH2D("CF2G4H2SH_AllHits_DVD", "D1i vs D2i", 240, -6.0, 6.0, 240, -6.0, 6.0),
    "\\Delta_{1i} [ns]", "\\Delta_{2i} [ns]"
  );
  //// Selected hits - we found 2+2 hits
  getStatistics().createHistogramWithAxes(
    new TH2D("CF2G4H2SH_ScaHits_FoundIndexes", "Found indexes for scattered gammas", 9,-0.5,9.5,9,-0.5,9.5),
    "First s-gamma index", "Second s-gamma index"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("CF2G4H2SH_Theta1_vs_Theta2", "Scattering angles", 181, -0.5, 180.5, 181, -0.5, 180.5),
    "\\theta_{1} [deg]", "\\theta_{2} [deg]"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4H2SH_DeltaPhi", "Delta phi", 181, -0.5, 180.5),
    "\\Delta\\phi [deg]", "Counts"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4H2SH_DeltaPhi_GAEPR", "Delta phi", 181, -0.5, 180.5),
    "\\Delta\\phi [deg]", "Counts"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("CF2G4H2SH_ScaHits_DVD", "D1i vs D2i", 240, -6.0, 6.0, 240, -6.0, 6.0),
    "\\Delta_{1i} [ns]", "\\Delta_{2i} [ns]"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4H2SH_Theta1", "1st gamma scattering angle", 181, -0.5, 180.5),
    "\\Theta_{1} [deg]", "Counts"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4H2SH_Theta2", "2nd gamma scattering angle", 181, -0.5, 180.5),
    "\\Theta_{} [deg]", "Counts"
  );
  
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4H2SH_AnnHit_Zpos", "Z position of annihilation hit", 201, -50.25, 50.25),
    "Z axis position [cm]", "Counts"
  );
  
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4H2SH_ScatHit_Zpos", "Z position of scattered hit", 201, -50.25, 50.25),
    "Z axis position [cm]", "Counts"
  );

  //CF2G4HCC - checkFor2Gamma4HitsCircleCut
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4HCC_DeltaPhi_Radius_30", "Delta phi", 181, -0.5, 180.5),
    "\\Delta\\phi [deg]", "Counts"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4HCC_DeltaPhi_Radius_10", "Delta phi", 181, -0.5, 180.5),
    "\\Delta\\phi [deg]", "Counts"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4HCC_DeltaPhi_Radius_30_GAEPR", "Delta phi", 181, -0.5, 180.5),
    "\\Delta\\phi [deg]", "Counts"
  );
  getStatistics().createHistogramWithAxes(
    new TH1D("CF2G4HCC_DeltaPhi_Radius_10_GAEPR", "Delta phi", 181, -0.5, 180.5),
    "\\Delta\\phi [deg]", "Counts"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("CF2G4HCC_Theta1_vs_Theta2_Radius_30", "Scattering angles", 181, -0.5, 180.5, 181, -0.5, 180.5),
    "\\theta_{1} [deg]", "\\theta_{2} [deg]"
  );
  getStatistics().createHistogramWithAxes(
    new TH2D("CF2G4HCC_Theta1_vs_Theta2_Radius_10", "Scattering angles", 181, -0.5, 180.5, 181, -0.5, 180.5),
    "\\theta_{1} [deg]", "\\theta_{2} [deg]"
  );
}
