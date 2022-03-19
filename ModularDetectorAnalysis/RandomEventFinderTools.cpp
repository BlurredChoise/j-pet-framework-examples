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

bool isCorrupted(const JPetBaseHit *hit) {
  if (dynamic_cast<const JPetRecoHit *>(hit)) {
    if (dynamic_cast<const JPetRecoHit *>(hit)->getRecoFlag() ==
        JPetRecoHit::Corrupted) {
      return true;
    }
  }
  return false;
}

std::pair<std::vector<int>, std::vector<JPetEvent>> getCoincidencesFromWindows(
    const JPetTimeWindow &window1, const JPetTimeWindow &window2,
    const std::vector<int> &inUsedHits, double kTOFWindow) {
  auto usedHits(inUsedHits);
  std::vector<JPetEvent> eventVec;

  const unsigned int nHits1 = window1.getNumberOfEvents();
  const unsigned int nHits2 = window2.getNumberOfEvents();
  // unsigned int count1 = 0;

  assert(usedHits.size() == nHits2);

  /// The current number of hits processed in the second time window
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
        if (isCorrupted(hit1) || isCorrupted(hit2)) {
          event.setRecoFlag(JPetEvent::Corrupted);
        }
        event.addHit(hit1);
        event.addHit(hit2);
        usedHits[j] = 1;
        eventVec.push_back(event);
        count2 = j + 1;
        break;
      } else {
        /// If the second hit is later in time than first hit but not within the
        /// TOF window then we take the next hit from the first container and
        /// keep the same second hit, to make the difference smaller
        if (tDiff > 0) {
          count2 = j;
          break;
        }
      }
    }
  }
  return {usedHits, eventVec};
}

std::vector<JPetEvent>
buildRandomEvents(const std::vector<JPetTimeWindow> &timeWindows) {
  assert(timeWindows.size() >= 3);
  //std::vector<JPetEvent> eventVec;

  const double kTOFWindow = 5;
  /// we compare the hits e.g. from n-th and n-th+2 time window,
  const int kTimeWindowShift = 2;
  //for (auto &timeWindow : timeWindows) {
  auto window1 = timeWindows[0];
  auto window2 = timeWindows[1];
    auto results = getCoincidencesFromWindows(
    window1, window2,
    {}, kTOFWindow);
    //JPetEvent event;
    //if (event.getHits().size() >= 3) {
      //eventVec.push_back(event);
    //}
  //}

  return results.second;
}
}
