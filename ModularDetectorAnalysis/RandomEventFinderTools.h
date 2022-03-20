/**
 *  @copyright Copyright 2021 The J-PET Framework Authors. All rights reserved.
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
 *  @file RandomEventFinderTools.h
 */

#ifndef RANDOMEVENTFINDERTOOLS_H
#define RANDOMEVENTFINDERTOOLS_H

//#include <Hits/JPetPhysRecoHit/JPetPhysRecoHit.h>
//#include <JPetEvent/JPetEvent.h>
//#include <JPetStatistics/JPetStatistics.h>
//#include <TVector3.h>
//#include <boost/property_tree/ptree.hpp>
#include <JPetTimeWindow/JPetTimeWindow.h>
#include <JPetEvent/JPetEvent.h>
#include <vector>


/**
 * @brief Tools for Random Event Finder
 *
 */
namespace random_event_finder_tools
{
    
  bool isCorrupted(const JPetBaseHit *hit);
  /// We assume that the hits  are ordered in time within given JPetTimeWindow
  /// We assume that window1 was earlier than window2
  /// inUsedHits is a vector corresponding to 0 or 1 flags which indicate if given 
  /// hit was previously used in window1, if it is set to 1, then it will be ignored
  /// during the processing
  //  @return a vector of used hits for window2 and a vector of created events as a pair 
  std::pair<std::vector<int>, std::vector<JPetEvent>> getCoincidencesFromWindows(const JPetTimeWindow& window1, const JPetTimeWindow& window2, const std::vector<int>& inUsedHits, double kTOFWindow =10);
};

#endif /* !RANDOMEVENTFINDERTOOLS_H */
