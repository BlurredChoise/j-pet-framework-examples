#!/bin/bash
../../build/VelocityCalibration/VelocityCalibration.x -t hld -f ../dataForVelocity/dabc_16315170022.hld -p conf_trb3.xml -u userParams_root6.json -i 2 -c ../../CalibrationFiles/2_RUN/TOTConfigRun1_2.root -l ../../CalibrationFiles/2_RUN/detectorSetupRun2.json -o results_root_6&
../../build/VelocityCalibration/VelocityCalibration.x -t hld -f ../dataForVelocity/dabc_16315215024.hld -p conf_trb3.xml -u userParams_root6.json -i 2 -c ../../CalibrationFiles/2_RUN/TOTConfigRun1_2.root -l ../../CalibrationFiles/2_RUN/detectorSetupRun2.json -o results_root_6&
../../build/VelocityCalibration/VelocityCalibration.x -t hld -f ../dataForVelocity/dabc_16316203534.hld -p conf_trb3.xml -u userParams_root6.json -i 2 -c ../../CalibrationFiles/2_RUN/TOTConfigRun1_2.root -l ../../CalibrationFiles/2_RUN/detectorSetupRun2.json -o results_root_6&
../../build/VelocityCalibration/VelocityCalibration.x -t hld -f ../dataForVelocity/dabc_16317101413.hld -p conf_trb3.xml -u userParams_root6.json -i 2 -c ../../CalibrationFiles/2_RUN/TOTConfigRun1_2.root -l ../../CalibrationFiles/2_RUN/detectorSetupRun2.json -o results_root_6&
../../build/VelocityCalibration/VelocityCalibration.x -t hld -f ../dataForVelocity/dabc_16317230553.hld -p conf_trb3.xml -u userParams_root6.json -i 2 -c ../../CalibrationFiles/2_RUN/TOTConfigRun1_2.root -l ../../CalibrationFiles/2_RUN/detectorSetupRun2.json -o results_root_6&

wait
../../build/VelocityCalibration/estimateVelocity results_root6.txt calibrationRoot6
