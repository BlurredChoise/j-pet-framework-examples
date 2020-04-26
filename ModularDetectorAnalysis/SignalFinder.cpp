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
 *  @file SignalFinder.cpp
 */

using namespace std;

#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetTimeWindow/JPetTimeWindow.h>
#include <JPetWriter/JPetWriter.h>
#include "SignalFinderTools.h"
#include "SignalFinder.h"
#include <TRandom.h>
#include <utility>
#include <string>
#include <vector>

using namespace jpet_options_tools;

SignalFinder::SignalFinder(const char* name): JPetUserTask(name) {}

SignalFinder::~SignalFinder() {}

bool SignalFinder::init()
{
  INFO("Signal finding started.");
  fOutputEvents = new JPetTimeWindow("JPetRawSignal");

  // Reading values from the user options if available
  // Time window parameter for leading edge
  if (isOptionSet(fParams.getOptions(), kEdgeMaxTimeParamKey)) {
    fSigChEdgeMaxTime = getOptionAsFloat(fParams.getOptions(), kEdgeMaxTimeParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.",
      kEdgeMaxTimeParamKey.c_str(), fSigChEdgeMaxTime)
    );
  }

  // Time window parameter for leading-trailing comparison
  if (isOptionSet(fParams.getOptions(), kLeadTrailMaxTimeParamKey)) {
    fSigChLeadTrailMaxTime = getOptionAsFloat(fParams.getOptions(), kLeadTrailMaxTimeParamKey);
  } else {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %lf.",
      kLeadTrailMaxTimeParamKey.c_str(), fSigChLeadTrailMaxTime));
  }

  // Getting bool for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey)) {
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }

  // Creating control histograms
  if(fSaveControlHistos) { initialiseHistograms(); }
  return true;
}

bool SignalFinder::exec()
{
  // Getting the data from event in an apropriate format
  if(auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent)) {
    // Distribute signal channels by PM IDs
    auto& sigChByPM = SignalFinderTools::getSigChByPM(timeWindow);
    // Building signals
    auto allSignals = SignalFinderTools::buildAllSignals(
      sigChByPM, fSigChEdgeMaxTime, fSigChLeadTrailMaxTime,
      kNumOfThresholds, getStatistics(), fSaveControlHistos
    );
    // Saving method invocation
    saveRawSignals(allSignals);
  } else { return false; }
  return true;
}

bool SignalFinder::terminate()
{
  INFO("Signal finding ended.");
  return true;
}

/**
 * Saving Raw Signals that have leading-trailing pairs,
 * otherwise filling histogram with incomplete signals
 */
void SignalFinder::saveRawSignals(const vector<JPetRawSignal>& rawSigVec)
{
  if(fSaveControlHistos){
    getStatistics().getHisto1D("rawsig_tslot")->Fill(rawSigVec.size());
  }

  for (auto & rawSig : rawSigVec) {
    auto leads = rawSig.getPoints(JPetSigCh::Leading, JPetRawSignal::ByThrValue);
    auto trails = rawSig.getPoints(JPetSigCh::Trailing, JPetRawSignal::ByThrValue);

    if(leads.size()==trails.size()){

      auto pmID = rawSig.getPM().getID();
      auto scinID = rawSig.getPM().getScin().getID();

      // Saving only signals with lead-trail pair on threshold
      fOutputEvents->add<JPetRawSignal>(rawSig);

      if(fSaveControlHistos){

        if(pmID > 400 && pmID < 505) {
          getStatistics().getHisto1D(Form("tot_thr1_sipm_%d", pmID))
          ->Fill(trails.at(0).getTime()-leads.at(0).getTime());

          if(leads.size() == 2){
            getStatistics().getHisto1D(Form("tot_thr2_sipm_%d", pmID))
            ->Fill(trails.at(1).getTime()-leads.at(1).getTime());

            getStatistics().getHisto1D(Form("lead_thr1_thr2_tdiff_sipm_%d", pmID))
            ->Fill(leads.at(1).getTime()-leads.at(0).getTime());

            getStatistics().getHisto1D(Form("trail_thr1_thr2_tdiff_sipm_%d", pmID))
            ->Fill(trails.at(1).getTime()-trails.at(0).getTime());
          }
        }

        if(gRandom->Uniform()<fScalingFactor) {
          getStatistics().getHisto1D("rawsig_per_pm")->Fill(pmID);
          getStatistics().getHisto1D("rawsig_per_scin")->Fill(scinID);
          getStatistics().getHisto1D("rawsig_multi")->Fill(leads.size()+trails.size());

          for(auto& sigCh : leads){
            getStatistics().getHisto1D("rawsig_mtx_occ")
            ->Fill(sigCh.getChannel().getPM().getMatrixPosition());
            getStatistics().getHisto1D("rawsig_thr_occ")
            ->Fill(sigCh.getChannel().getThresholdNumber());
          }

          for(auto& sigCh : trails){
            getStatistics().getHisto1D("rawsig_mtx_occ")
            ->Fill(sigCh.getChannel().getPM().getMatrixPosition());
            getStatistics().getHisto1D("rawsig_thr_occ")
            ->Fill(sigCh.getChannel().getThresholdNumber());
          }
        }
      }
    }
  }
}

void SignalFinder::initialiseHistograms(){

  auto minPMID = getParamBank().getPMs().begin()->first;
  // auto maxPMID = getParamBank().getPMs().rbegin()->first;
  auto maxPMID = 505;
  auto minScinD = getParamBank().getScins().begin()->first;
  auto maxScinID = getParamBank().getScins().rbegin()->first;

  // Unused objects stats
  getStatistics().createHistogram(new TH1F(
    "unused_sigch_thr", "Unused Signal Channels per THR (downscaled)", 5, 0.5, 5.5
  ));
  getStatistics().getHisto1D("unused_sigch_thr")->GetXaxis()->SetBinLabel(1,"THR 1 Lead");
  getStatistics().getHisto1D("unused_sigch_thr")->GetXaxis()->SetBinLabel(2,"THR 1 Trail");
  getStatistics().getHisto1D("unused_sigch_thr")->GetXaxis()->SetBinLabel(3,"THR 2 Lead");
  getStatistics().getHisto1D("unused_sigch_thr")->GetXaxis()->SetBinLabel(4,"THR 2 Trail");
  getStatistics().getHisto1D("unused_sigch_thr")->GetXaxis()->SetBinLabel(5,"  ");
  getStatistics().getHisto1D("unused_sigch_thr")->GetYaxis()->SetTitle("Number of SigChs");

  getStatistics().createHistogram(new TH1F(
    "unused_sigch_pm", "Unused Signal Channels per SiPM",
    maxPMID-minPMID+1, minPMID-0.5, maxPMID+0.5
  ));
  getStatistics().getHisto1D("unused_sigch_pm")->GetXaxis()->SetTitle("SiPM ID");
  getStatistics().getHisto1D("unused_sigch_pm")->GetYaxis()->SetTitle("Number of Signal Channels");

  // Occupancies and multiplicities
  getStatistics().createHistogram(new TH1F(
    "rawsig_per_pm", "Raw Signals per SiPM",
    maxPMID-minPMID+1, minPMID-0.5, maxPMID+0.5
  ));
  getStatistics().getHisto1D("rawsig_per_pm")->GetXaxis()->SetTitle("SiPM ID");
  getStatistics().getHisto1D("rawsig_per_pm")->GetYaxis()->SetTitle("Number of Raw Signals");

  getStatistics().createHistogram(new TH1F(
    "rawsig_per_scin", "Raw Signals per scintillator",
    maxScinID-minScinD+1, minScinD-0.5, maxScinID+0.5
  ));
  getStatistics().getHisto1D("rawsig_per_scin")->GetXaxis()->SetTitle("Scin ID");
  getStatistics().getHisto1D("rawsig_per_scin")->GetYaxis()->SetTitle("Number of Raw Signals");

  getStatistics().createHistogram(new TH1F(
    "rawsig_thr_occ", "Thresholds occupation in createds Raw Signals",
    3, 0.5, 3.5
  ));
  getStatistics().getHisto1D("rawsig_thr_occ")->GetXaxis()->SetTitle("Threshold number");
  getStatistics().getHisto1D("rawsig_thr_occ")->GetYaxis()->SetTitle("Number of Signal Channels");

  getStatistics().createHistogram(new TH1F(
    "rawsig_mtx_occ", "Matrix SiPMs occupation in createds Raw Signals",
    6, -0.5, 5.5
  ));
  getStatistics().getHisto1D("rawsig_mtx_occ")->GetXaxis()->SetTitle("SiPM matrix position number");
  getStatistics().getHisto1D("rawsig_mtx_occ")->GetYaxis()->SetTitle("Number of Signal Channels");

  getStatistics().createHistogram(new TH1F(
    "rawsig_multi", "Raw Signal Multiplicity", 6, 0.5, 6.5
  ));
  getStatistics().getHisto1D("rawsig_multi")->GetXaxis()->SetTitle("Total number of SigChs in RawSig");
  getStatistics().getHisto1D("rawsig_multi")->GetYaxis()->SetTitle("Number of Signal Channels");

  getStatistics().createHistogram(new TH1F(
    "rawsig_tslot", "Number of Raw Signals in Time Window", 200, -0.5, 199.5
  ));
  getStatistics().getHisto1D("rawsig_tslot")->GetXaxis()->SetTitle("Number of Raw Signal in Time Window");
  getStatistics().getHisto1D("rawsig_tslot")->GetYaxis()->SetTitle("Number of Time Windows");

  // Time differences lead/trail/THR1/THR2 per SiPM
  for(int i=minPMID; i<=505; i++){
    getStatistics().createHistogram(new TH1F(
      Form("lead_thr1_thr2_tdiff_sipm_%d", i),
      Form("Time Difference between leading Signal Channels THR1 and THR2 in found signals on SiPM ID %d", i),
      200, -1.1*fSigChEdgeMaxTime, 1.1*fSigChEdgeMaxTime
    ));
    getStatistics().getHisto1D(Form("lead_thr1_thr2_tdiff_sipm_%d", i))
    ->GetXaxis()->SetTitle("time diff [ps]");
    getStatistics().getHisto1D(Form("lead_thr1_thr2_tdiff_sipm_%d", i))
    ->GetYaxis()->SetTitle("Number of Signal Channel Pairs");

    getStatistics().createHistogram(new TH1F(
      Form("trail_thr1_thr2_tdiff_sipm_%d", i),
      Form("Time Difference between trailing Signal Channels THR1 and THR2 in found signals on SiPM ID %d", i),
      200, -2.0*fSigChEdgeMaxTime, 2.0*fSigChEdgeMaxTime
    ));
    getStatistics().getHisto1D(Form("trail_thr1_thr2_tdiff_sipm_%d", i))
    ->GetXaxis()->SetTitle("time diff [ps]");
    getStatistics().getHisto1D(Form("trail_thr1_thr2_tdiff_sipm_%d", i))
    ->GetYaxis()->SetTitle("Number of Signal Channel Pairs");

    getStatistics().createHistogram(new TH1F(
      Form("tot_thr1_sipm_%d", i),
      Form("Signal Time over Threshold on THR1 on SiPM ID %d", i),
      200, 0.0, 1.1*fSigChLeadTrailMaxTime
    ));
    getStatistics().getHisto1D(Form("tot_thr1_sipm_%d", i))
    ->GetXaxis()->SetTitle("TOT [ps]");
    getStatistics().getHisto1D(Form("tot_thr1_sipm_%d", i))
    ->GetYaxis()->SetTitle("Number of Raw Signal");

    getStatistics().createHistogram(new TH1F(
      Form("tot_thr2_sipm_%d", i),
      Form("Signal Time over Threshold on THR1 on SiPM ID %d", i),
      200, 0.0, 1.1*fSigChLeadTrailMaxTime
    ));
    getStatistics().getHisto1D(Form("tot_thr2_sipm_%d", i))
    ->GetXaxis()->SetTitle("TOT [ps]");
    getStatistics().getHisto1D(Form("tot_thr2_sipm_%d", i))
    ->GetYaxis()->SetTitle("Number of Raw Signal");

  }
}
