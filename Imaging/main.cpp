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
 *  @file main.cpp
 */

// #include "../ModularDetectorAnalysis/EventFinder.h"
// #include "../ModularDetectorAnalysis/HitFinder.h"
// #include "../ModularDetectorAnalysis/SignalFinder.h"
// #include "../ModularDetectorAnalysis/SignalTransformer.h"
// #include "../ModularDetectorAnalysis/TimeWindowCreator.h"

#include "EventCategorizerImaging.h"
#include <JPetManager/JPetManager.h>

using namespace std;

int main(int argc, const char* argv[])
{
  try
  {
    JPetManager& manager = JPetManager::getManager();

    // TODO make LOR finder task
    manager.registerTask<LORFinder>("LORFinder");
    manager.useTask("LORFinder", "cat.evt", "lor");

    // manager.registerTask<TimeWindowCreator>("TimeWindowCreator");
    // manager.registerTask<SignalFinder>("SignalFinder");
    // manager.registerTask<SignalTransformer>("SignalTransformer");
    // manager.registerTask<HitFinder>("HitFinder");
    // manager.registerTask<EventFinder>("EventFinder");
    // manager.registerTask<EventCategorizerImaging>("EventCategorizerImaging");

    // manager.useTask("TimeWindowCreator", "hld", "tslot");
    // manager.useTask("SignalFinder", "tslot", "sipm.sig");
    // manager.useTask("SignalTransformer", "sipm.sig", "mtx.sig");
    // manager.useTask("HitFinder", "mtx.sig", "hits");
    // manager.useTask("EventFinder", "hits", "unk.evt");
    // manager.useTask("EventCategorizerImaging", "unk.evt", "cat.evt");

    manager.run(argc, argv);
  }
  catch (const std::exception& except)
  {
    std::cerr << "Unrecoverable error occured:" << except.what() << "Exiting the program!" << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
