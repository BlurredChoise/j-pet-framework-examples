/**
 *  @copyright Copyright 2022 The J-PET Framework Authors. All rights reserved.
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
 *  @file RandomEventFinderToolsTest.cpp
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE HitFinderToolsTestSuite

#include "../RandomEventFinder.h"
#include "../RandomEventFinderTools.h"
#include <Hits/JPetRecoHit/JPetRecoHit.h>
#include <boost/test/unit_test.hpp>

auto epsilon = 0.0001;
using namespace random_event_finder_tools;
using CoincidenceTimes = std::pair<double, double>;
using Coincidences = std::vector<CoincidenceTimes>;

Coincidences getTimesOfCoincidences(const std::vector<JPetEvent>& events)
{
  Coincidences times;
  for (auto & event: events) {
    assert(event.getHits().size() >=2);
    times.push_back({event.getHits()[0]->getTime(), event.getHits()[1]->getTime()});
  }
  return times;
}

JPetTimeWindow fillTimeWindow(const std::vector<double>& hitsTimes)
{
  JPetTimeWindow window("JPetRecoHit");
  for (auto time :hitsTimes) {
    JPetRecoHit hit;
    hit.setTime(time);
    window.add(hit);
  }
  return window;
}

BOOST_AUTO_TEST_SUITE(RandomEventFinderToolsTestSuite)

/// No coincidences found
BOOST_AUTO_TEST_CASE(getCoincidencesFromWindows_test_00)
{
  double tofWindow = 5;
  auto window1 = fillTimeWindow({1});
  auto window2 = fillTimeWindow({10});
  std::vector<int> usedHits ={0};
  auto results = getCoincidencesFromWindows(window1, window2, usedHits, tofWindow);
  auto used = results.first;
  auto events = results.second;
  BOOST_REQUIRE(events.empty());
}


/// One coincidence found
BOOST_AUTO_TEST_CASE(getCoincidencesFromWindows_test_01a)
{
  double tofWindow = 5;
  auto window1 = fillTimeWindow({1});
  auto window2 = fillTimeWindow({4});
  std::vector<int> usedHits ={0};
  auto results = getCoincidencesFromWindows(window1, window2, usedHits, tofWindow);
  auto used = results.first;
  auto events = results.second;
  BOOST_REQUIRE_EQUAL(events.size(), 1);
  auto times = getTimesOfCoincidences(events);
  BOOST_REQUIRE_CLOSE(times[0].first, 1, epsilon);
  BOOST_REQUIRE_CLOSE(times[0].second, 4, epsilon);
}

/// One coincidence found
BOOST_AUTO_TEST_CASE(getCoincidencesFromWindows_test_01b)
{
  double tofWindow = 5;
  auto window1 = fillTimeWindow({4});
  auto window2 = fillTimeWindow({1});
  std::vector<int> usedHits ={0};
  auto results = getCoincidencesFromWindows(window1, window2, usedHits, tofWindow);
  auto used = results.first;
  auto events = results.second;
  BOOST_REQUIRE_EQUAL(events.size(), 1);
  auto times = getTimesOfCoincidences(events);
  BOOST_REQUIRE_CLOSE(times[0].first, 4, epsilon);
  BOOST_REQUIRE_CLOSE(times[0].second, 1, epsilon);
}

BOOST_AUTO_TEST_CASE(getCoincidencesFromWindows_test_02a)
{
  double tofWindow = 5;
  auto window1 = fillTimeWindow({1,10});
  auto window2 = fillTimeWindow({9});
  std::vector<int> usedHits ={0};
  auto results = getCoincidencesFromWindows(window1, window2, usedHits, tofWindow);
  auto used = results.first;
  auto events = results.second;
  BOOST_REQUIRE_EQUAL(events.size(), 1);
  auto times = getTimesOfCoincidences(events);
  BOOST_REQUIRE_CLOSE(times[0].first, 10, epsilon);
  BOOST_REQUIRE_CLOSE(times[0].second, 9, epsilon);
}

BOOST_AUTO_TEST_CASE(getCoincidencesFromWindows_test_02b)
{
  double tofWindow = 5;
  auto window1 = fillTimeWindow({10});
  auto window2 = fillTimeWindow({1,9});
  std::vector<int> usedHits ={0};
  auto results = getCoincidencesFromWindows(window1, window2, usedHits, tofWindow);
  auto used = results.first;
  auto events = results.second;
  BOOST_REQUIRE_EQUAL(events.size(), 1);
  auto times = getTimesOfCoincidences(events);
  BOOST_REQUIRE_CLOSE(times[0].first, 10, epsilon);
  BOOST_REQUIRE_CLOSE(times[0].second, 9, epsilon);
}

BOOST_AUTO_TEST_CASE(getCoincidencesFromWindows_test)
{
  auto window1 = fillTimeWindow({1,10, 20, 33, 41});
  auto window2 = fillTimeWindow({4, 14, 26,34});
  /// from those sets with TOF size 5  we expect the following pairs
  /// {1,4} , {10,14}, {33,34}
  std::vector<int> usedHits ={0,0,0,0,0};
  auto results = getCoincidencesFromWindows(window1, window2, usedHits, 5);
  auto used = results.first;
  auto events = results.second;
  BOOST_REQUIRE_EQUAL(events.size(), 3);
  auto times = getTimesOfCoincidences(events);
  BOOST_REQUIRE_CLOSE(times[0].first, 1, epsilon);
  BOOST_REQUIRE_CLOSE(times[0].second, 4, epsilon);
  BOOST_REQUIRE_CLOSE(times[1].first, 10, epsilon);
  BOOST_REQUIRE_CLOSE(times[1].second, 14, epsilon);
  BOOST_REQUIRE_CLOSE(times[2].first, 33, epsilon);
  BOOST_REQUIRE_CLOSE(times[2].second, 34, epsilon);
}

BOOST_AUTO_TEST_CASE(getCoincidencesFromWindows_test2)
{
  auto window1 = fillTimeWindow({4,10, 20, 34, 41});
  auto window2 = fillTimeWindow({1, 14, 26,33});
  /// from those sets with TOF size 5  we expect the following pairs
  /// {1,4} , {10,14}, {33,34}
  std::vector<int> usedHits ={0,0,0,0,0};
  auto results = getCoincidencesFromWindows(window1, window2, usedHits, 5);
  auto used = results.first;
  auto events = results.second;
  BOOST_REQUIRE_EQUAL(events.size(), 3);
  auto times = getTimesOfCoincidences(events);
  BOOST_REQUIRE_CLOSE(times[0].first, 4, epsilon);
  BOOST_REQUIRE_CLOSE(times[0].second, 1, epsilon);
  BOOST_REQUIRE_CLOSE(times[1].first, 10, epsilon);
  BOOST_REQUIRE_CLOSE(times[1].second, 14, epsilon);
  BOOST_REQUIRE_CLOSE(times[2].first, 34, epsilon);
  BOOST_REQUIRE_CLOSE(times[2].second, 33, epsilon);
}

BOOST_AUTO_TEST_SUITE_END()
