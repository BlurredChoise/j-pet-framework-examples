/**
 *  @copyright Copyright 2021 The J-PET Framework Authors. All rights reserved.
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
 *  @file RandomEventFinder.cpp
 */


#include "RandomEventFinder.h"
#include "RandomEventFinderTools.h"
#include "EventCategorizerTools.h"
#include <Hits/JPetMCRecoHit/JPetMCRecoHit.h>
#include <Hits/JPetRecoHit/JPetRecoHit.h>
#include <JPetOptionsTools/JPetOptionsTools.h>
#include <JPetRawMCHit/JPetRawMCHit.h>
#include <JPetWriter/JPetWriter.h>
#include <iostream>

using namespace std;
using namespace jpet_options_tools;
using namespace random_event_finder_tools;

RandomEventFinder::RandomEventFinder(const char* name) : JPetUserTask(name) {}

RandomEventFinder::~RandomEventFinder() {}

bool RandomEventFinder::init()
{
  INFO("Event finding started.");

  fOutputEvents = new JPetTimeWindow("JPetEvent");

  // Reading values from the user options if available
  // Getting bool for using corrupted hits
  if (isOptionSet(fParams.getOptions(), kUseCorruptedHitsParamKey))
  {
    fUseCorruptedHits = getOptionAsBool(fParams.getOptions(), kUseCorruptedHitsParamKey);
    if (fUseCorruptedHits)
    {
      WARNING("Event Finder is using Corrupted Hits, as set by the user");
    }
    else
    {
      WARNING("Event Finder is NOT using Corrupted Hits, as set by the user");
    }
  }
  else
  {
    WARNING("Event Finder is not using Corrupted Hits (default option)");
  }

  // Event time window
  if (isOptionSet(fParams.getOptions(), kEventTimeParamKey))
  {
    fEventTimeWindow = getOptionAsDouble(fParams.getOptions(), kEventTimeParamKey);
  }

  // Minimum number of hits in an event to save an event
  if (isOptionSet(fParams.getOptions(), kEventMinMultiplicity))
  {
    fMinMultiplicity = getOptionAsInt(fParams.getOptions(), kEventMinMultiplicity);
  }
  else
  {
    WARNING(Form("No value of the %s parameter provided by the user. Using default value of %d.", kEventMinMultiplicity.c_str(), fMinMultiplicity));
  }

  // Getting bool for saving histograms
  if (isOptionSet(fParams.getOptions(), kSaveControlHistosParamKey))
  {
    fSaveControlHistos = getOptionAsBool(fParams.getOptions(), kSaveControlHistosParamKey);
  }

  // Initialize histograms
  if (fSaveControlHistos)
  {
    initialiseHistograms();
  }
  return true;
}

void RandomEventFinder::findRandomsAndSaveEventsIfAnyFound()
{
  const int kMaxTimeWindowsStored = 3;
  if (fTimeWindowContainer.size() == kMaxTimeWindowsStored) {
     auto window1 = fTimeWindowContainer.front();
     auto window2 = fTimeWindowContainer.back();
     auto results = getCoincidencesFromWindows(window1, window2, {}, 2);
     saveEvents(results.second);
     fTimeWindowContainer.pop(); ///remove first element
  }
}

bool RandomEventFinder::exec()
{
  if (auto timeWindow = dynamic_cast<const JPetTimeWindow* const>(fEvent))
  {
    fTimeWindowContainer.push(*timeWindow);
  }
  else
  {
    return false;
  }
  findRandomsAndSaveEventsIfAnyFound();
  return true;
}

bool RandomEventFinder::terminate()
{
  INFO("Event fiding ended.");
  findRandomsAndSaveEventsIfAnyFound();
  return true;
}

void RandomEventFinder::saveEvents(const vector<JPetEvent>& events)
{
  for (const auto& event : events)
  {
    fOutputEvents->add<JPetEvent>(event);
  }
}



void RandomEventFinder::initialiseHistograms()
{
  getStatistics().createHistogramWithAxes(new TH1D("event_hits_tdiff_all", "Time difference of consecutive hits", 200, 0.0, 200000.0),
                                          "Time difference [ps]", "Number of Hit Pairs");

  getStatistics().createHistogramWithAxes(new TH1D("event_hits_tdiff_rejected", "Time difference of consecutive unmatched hits", 200, 0.0, 200000.0),
                                          "Time difference [ps]", "Number of Hit Pairs");

  getStatistics().createHistogramWithAxes(new TH1D("event_multi_all", "Number of Hits in all Events", 20, 0.5, 20.5), "Hits in Event",
                                          "Number of Hits");

  getStatistics().createHistogramWithAxes(new TH1D("event_multi_selected", "Number of Hits in selected Events", 20, 0.5, 20.5), "Hits in Event",
                                          "Number of Hits");

  getStatistics().createHistogramWithAxes(new TH1D("reco_flags_events", "Reconstruction flags of created events", 4, 0.5, 4.5), " ",
                                          "Number of Channel Signals");
  vector<pair<unsigned, string>> binLabels = {make_pair(1, "GOOD"), make_pair(2, "CORRUPTED"), make_pair(3, "UNKNOWN"), make_pair(4, "")};
  getStatistics().setHistogramBinLabel("reco_flags_events", getStatistics().AxisLabel::kXaxis, binLabels);
}

