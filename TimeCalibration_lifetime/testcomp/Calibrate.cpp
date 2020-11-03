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
 *  @file Calibrate.cpp
 */

#include "CalibrationTools.h"

int main(int argc, char* argv[])
{
  std::string fileWithParameters = "";
  std::string calibrationOption = "";
  if (argc == 3) {
    fileWithParameters = argv[1];
    calibrationOption = argv[2];
  } else if (argc < 3) {
    // No file with parameters or calibration option
  } else {
    // other arguments are obsolete   
  }

  CalibrationTools calibTools(fileWithParameters, calibrationOption);
  calibTools.LoadCalibrationParameters();
  calibTools.Calibrate();
  
  return 0;
}
