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
 *  @file RandomEventFinder.h
 */

#ifndef EVENTFINDER_H
#define EVENTFINDER_H

#include <Hits/JPetBaseHit/JPetBaseHit.h>
#include <JPetEvent/JPetEvent.h>
#include <JPetUserTask/JPetUserTask.h>
#include <map>
#include <vector>
#include <queue>

/**
 * @brief User Task creating JPetEvent from hits
 *
 * Simple task, that groups hits into uncategorized Events
 * with the use of some time window value. This value is given by
 * default, but it can be provided by the user in parameters file.
 * Also user can require to save only Events of minimum multiplicity
 * and if include Corrupted Hits in the created events.
 */
class RandomEventFinder : public JPetUserTask
{
public:
  RandomEventFinder(const char* name);
  virtual ~RandomEventFinder();
  virtual bool init() override;
  virtual bool exec() override;
  virtual bool terminate() override;

protected:
  void findRandomsAndSaveEventsIfAnyFound();
  void saveEvents(const std::vector<JPetEvent>& event);
  void initialiseHistograms();

  const std::string kUseCorruptedHitsParamKey = "EventFinder_UseCorruptedHits_bool";
  const std::string kEventMinMultiplicity = "EventFinder_MinEventMultiplicity_int";
  const std::string kSaveControlHistosParamKey = "Save_Control_Histograms_bool";
  const std::string kEventTimeParamKey = "EventFinder_EventTime_double";
  bool fUseCorruptedHits = false;
  bool fSaveControlHistos = true;
  double fEventTimeWindow = 5000.0;
  unsigned int fMinMultiplicity = 1;

  std::queue<std::vector<int>> fUsedHits;
  std::queue<JPetTimeWindow> fTimeWindowContainer;
};
#endif /* !EVENTFINDER_H */
