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
std::pair<std::vector<int>, std::vector<JPetEvent>> getCoincidencesFromWindows(
    const JPetTimeWindow &window1, const JPetTimeWindow &window2,
    const std::vector<int> &inUsedHits, double kTOFWindow) {
  auto usedHits(inUsedHits);
  std::vector<JPetEvent> eventVec;

  const unsigned int nHits1 = window1.getNumberOfEvents();
  const unsigned int nHits2 = window2.getNumberOfEvents();
  // unsigned int count1 = 0;

  assert(usedHits.size() == nHits2);

  unsigned int count2 = 0;

  for (int i = 0; i < nHits1; i++) {
    auto hit1 = dynamic_cast<const JPetBaseHit *>(&window1.operator[](i));
    std::cout << "find hit from the first window with the time ="
              << hit1->getTime() << std::endl;
    for (int j = count2; j < nHits2; j++) {
      if (usedHits[j] == 1) {
        continue;
      }
      auto hit2 = dynamic_cast<const JPetBaseHit *>(&window2.operator[](j));
      std::cout << "find hit from the second window with the time ="
                << hit2->getTime() << std::endl;
      auto tDiff = hit2->getTime() - hit1->getTime();
      auto absTDiff = fabs(tDiff);

      if (absTDiff < kTOFWindow) {

        std::cout << "hits within the TOFtime window" << std::endl;
        std::cout << "hit from the first window with the time ="
                  << hit1->getTime() << std::endl;
        std::cout << "hit from the second window with the time ="
                  << hit2->getTime() << std::endl;
        JPetEvent event;
        event.setEventType(JPetEventType::kUnknown);
        event.addHit(hit1);
        event.addHit(hit2);
        usedHits[j] = 1;
        eventVec.push_back(event);
        count2 = j + 1;
        break;
      } else {
        /// If the second hit is later in time than first hit but not within the TOF window
        /// then we increment the first hit and keep the same second hit, to make the difference smaller
        if (tDiff > 0) {
          count2 = j;
          break;

        }
      }
    }
  }
    //while (count1 < nHits1 &&) {
      //auto hit = dynamic_cast<const JPetBaseHit *>(&window1.operator[](count1));
      //std::cout << "find hit from the first window with the time ="
                //<< hit->getTime() << std::endl;
      //JPetEvent event;
      //event.setEventType(JPetEventType::kUnknown);

      //while (count2 < nHits2) {
        //if (usedHits[count2] == 1) {
          //count2++;
          //continue;
        //}
        //auto nextHit =
            //dynamic_cast<const JPetBaseHit *>(&window2.operator[](count2));
        //std::cout << "find hit from the second window with the time ="
                  //<< nextHit->getTime() << std::endl;
        //auto tDiff = fabs(nextHit->getTime() - hit->getTime());
        //if (tDiff < kTOFWindow) {

          //std::cout << "hits within the TOFtime window" << std::endl;
          //std::cout << "hit from the first window with the time ="
                    //<< hit->getTime() << std::endl;
          //std::cout << "hit from the second window with the time ="
                    //<< nextHit->getTime() << std::endl;
          ////// Reco flag check
          //// if (dynamic_cast<const JPetRecoHit *>(nextHit)) {
          //// if (dynamic_cast<const JPetRecoHit *>(nextHit)->getRecoFlag() ==
          //// JPetRecoHit::Corrupted) {
          //// event.setRecoFlag(JPetEvent::Corrupted);
          ////}
          ////}
          //event.addHit(hit);
          //event.addHit(nextHit);
          //usedHits[count2] = 1;
          //count2++;
          //break;
        //}
        //if (tDiff >kTOFWindow) {
          //break;
        //}
          
      //}
      //count1++;

      //if (event.getHits().size() >= 2) {
        //eventVec.push_back(event);
      //}
    //}

    return {usedHits, eventVec};
  }

  std::vector<JPetEvent> buildRandomEvents(const std::vector<JPetTimeWindow>& timeWindows)
  {
    assert(timeWindows.size() >=3);
    std::vector<JPetEvent> eventVec;

    const int kTimeWindowShift = 2; // we compare the hits e.g. from n-th and n-th+2 time window,  
    for (auto & timeWindow :timeWindows)
    {
      const unsigned int nHits = timeWindow.getNumberOfEvents();
      unsigned int count = 0;

      while (count < nHits) {
        auto hit =
            dynamic_cast<const JPetBaseHit *>(&timeWindow.operator[](count));

         //If Event contains hits of reco class, then check corrupted data
         //filter
         //if (dynamic_cast<const JPetRecoHit*>(hit))
        //{
         //if (!fUseCorruptedHits && dynamic_cast<const
         //JPetRecoHit*>(hit)->getRecoFlag() == JPetRecoHit::Corrupted)
        //{
         //count++;
         //continue;
        //}
        //}

        // Creating new event with the first hit
        JPetEvent event;
        event.setEventType(JPetEventType::kUnknown);
        event.addHit(hit);

        // If hit is reco class, then check set corrupted data flag
        // approptiately
        if (dynamic_cast<const JPetRecoHit *>(hit)) {
          if (dynamic_cast<const JPetRecoHit *>(hit)->getRecoFlag() ==
              JPetRecoHit::Good) {
            event.setRecoFlag(JPetEvent::Good);
          } else if (dynamic_cast<const JPetRecoHit *>(hit)->getRecoFlag() ==
                     JPetRecoHit::Corrupted) {
            event.setRecoFlag(JPetEvent::Corrupted);
          }
        }

        // Checking, if following hits fulfill time window condition, then
        // moving the interator
        unsigned int nextCount = 1;
        while (count + nextCount < nHits) {
          auto nextHit = dynamic_cast<const JPetBaseHit *>(
              &timeWindow.operator[](count + nextCount));
          auto tDiff = fabs(nextHit->getTime() - hit->getTime());
          if (tDiff < 10) {
            // Reco flag check
            if (dynamic_cast<const JPetRecoHit *>(nextHit)) {
              if (dynamic_cast<const JPetRecoHit *>(nextHit)->getRecoFlag() ==
                  JPetRecoHit::Corrupted) {
                event.setRecoFlag(JPetEvent::Corrupted);
              }
            }
            event.addHit(nextHit);
            nextCount++;
          } else {
            break;
          }
        }
        count += nextCount;

        if (event.getHits().size() >= 3) {
          eventVec.push_back(event);
        }
      }
  }

  return eventVec;
  }
}
