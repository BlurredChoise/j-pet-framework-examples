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
 *  @file RandomEventFinderTools.cpp
 */

#include "RandomEventFinderTools.h"
#include <Hits/JPetRecoHit/JPetRecoHit.h>
//#include <TMath.h>
//#include <TRandom.h>

namespace random_event_finder_tools
{

  std::vector<JPetEvent> buildRandomEvents(const std::vector<JPetTimeWindow>& timeWindows)
  {
  std::vector<JPetEvent> eventVec;
  if (timeWindows.size() >=3) {
    for (auto & timeWindow :timeWindows)
    {
      const unsigned int nHits = timeWindow.getNumberOfEvents();
      unsigned int count = 0;

  while (count < nHits)
  {
    auto hit = dynamic_cast<const JPetBaseHit*>(&timeWindow.operator[](count));

    // If Event contains hits of reco class, then check corrupted data filter
    //if (dynamic_cast<const JPetRecoHit*>(hit))
    //{
      //if (!fUseCorruptedHits && dynamic_cast<const JPetRecoHit*>(hit)->getRecoFlag() == JPetRecoHit::Corrupted)
      //{
        //count++;
        //continue;
      //}
    //}

    // Creating new event with the first hit
    JPetEvent event;
    event.setEventType(JPetEventType::kUnknown);
    event.addHit(hit);

    // If hit is reco class, then check set corrupted data flag approptiately
    if (dynamic_cast<const JPetRecoHit*>(hit))
    {
      if (dynamic_cast<const JPetRecoHit*>(hit)->getRecoFlag() == JPetRecoHit::Good)
      {
        event.setRecoFlag(JPetEvent::Good);
      }
      else if (dynamic_cast<const JPetRecoHit*>(hit)->getRecoFlag() == JPetRecoHit::Corrupted)
      {
        event.setRecoFlag(JPetEvent::Corrupted);
      }
    }

    // Checking, if following hits fulfill time window condition, then moving the interator
    unsigned int nextCount = 1;
    while (count + nextCount < nHits)
    {
      auto nextHit = dynamic_cast<const JPetBaseHit*>(&timeWindow.operator[](count + nextCount));
      auto tDiff = fabs(nextHit->getTime() - hit->getTime());
      if (tDiff < 10)
      {
        // Reco flag check
        if (dynamic_cast<const JPetRecoHit*>(nextHit))
        {
          if (dynamic_cast<const JPetRecoHit*>(nextHit)->getRecoFlag() == JPetRecoHit::Corrupted)
          {
            event.setRecoFlag(JPetEvent::Corrupted);
          }
        }
        event.addHit(nextHit);
        nextCount++;
      }
      else
      {
        break;
      }
    }
    count += nextCount;

    if (event.getHits().size() >= 3)
    {
      eventVec.push_back(event);
    }
    }
    }
  }

  return eventVec;
  }
}
