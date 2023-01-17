# Smiley analysis 

## Aim
This is an example of creating one's own analysis based on the `LargeBarrelAnalysis`. You can start your own analysis by copying the `NewAnalysisTemplate` directory.

## Starting you own Analysis
When starting you own analysis, do not edit any of the existing examples as this will lead to problems with Git-based version control. Ipstead, follow this procedure:

 1. Copy the `NewAnalysisTemplate` directory to a name you wish to give to you analysis. Let us use `MyAnalysis` as an example:  
    `cp -r NewAnalysisTemplate MyAnalysis`

 2. Add the following line at the end of the `j-pet-framework-examples/CMakeLists.txt`:  
    `add_subdirectory(MyAnalysis)`  
    (replace `MyAnalysis` with whichever name you gave to your directory)

 3. Edit the file `CMakeLists.txt` located in you new directory, e.g. `MyAnalysis/CMakeLists.txt` and and in the following line:  
    `set(projectName NewProjectName)`  
    replace `NewProjectName` with your project name, e.g. `MyAnalysis`

 4. Furter in the `CMakeLists.txt` file, you will see lines respopsible for using modules from the `LargeBarrelAnalysis` example. In case you want to customize any of the modules, copy this modules `.h` and `.cpp` files to your directory and comment out the lines concerning this module.

 5. Now when you follow the standard procedure to configure and build all of the J-PET framework examples, a directory with the name you chose will be created in your build directory, e.g. `build/MyAnalysis` and will contain an executable with the name you chose for the project, e.g. `MyAnalysis.x`.

## Additional Info
Note that the `TimeCalibration` and `VelocityCalibration` examples follow exactly the above scheme of being based on `LargeBarrelAnalysis` and its modules, so you can refer to these examples for advanced usage of the template described here.

## Author
[Aleksander Gajos](https://github.com/alekgajos)  
Please report any bugs and suggestiops of correctiops to: [aleksander.gajos@uj.edu.pl](aleksander.gajos@uj.edu.pl)

## Example run:

`/SmileyAnalysis.x -t root -f /home/krzemien/workdir/pet/framework/smiley_analysis/oldAnalysis/files/dabc_20184180113.unk.evt.root -p conf_trb3.xml  -o ./ -u userParams.json  -i 9 -l detectorSetupRun9.json -r 1 1000 ` 

## Flat tree structure

Tree name : **SAFlatTree**

| Branch       | Unit | Track | Hit |Description |
| ---          | ---  | ---   | --- | ---        |
| t1h1_x       | cm   | 1     | 1   | hit's x-position |
| t1h1_y       | cm   | 1     | 1   | hit's y-position |
| t1h1_z       | cm   | 1     | 1   | hit's z-position |
| t1h1_px      | 1    | 1     | 1   | photon momentum x-component |
| t1h1_py      | 1    | 1     | 1   | photon momentum y-component |
| t1h1_pz      | 1    | 1     | 1   | photon momentum z-component |
| t1h1_ed      | ps   | 1     | 1   | deposited energy during a hit (TOT) |
| t1h1_t       | ps   | 1     | 1   | hit window time |
| t1h1_lid     | 1    | 1     | 1   | layer ID where a hit occured |
| t2h1_x       | cm   | 2     | 1   | hit's x-position |
| t2h1_y       | cm   | 2     | 1   | hit's y-position |
| t2h1_z       | cm   | 2     | 1   | hit's z-position |
| t2h1_px      | 1    | 2     | 1   | photon momentum x-component |
| t2h1_pz      | 1    | 2     | 1   | photon momentum y-component |
| t2h1_py      | 1    | 2     | 1   | photon momentum z-component |
| t2h1_ed      | ps   | 2     | 1   | deposited energy during a hit (TOT) |
| t2h1_t       | ps   | 2     | 1   | hit window time |
| t2h1_lid     | 1    | 2     | 1   | layer ID where a hit occured |
| t1h2_x       | cm   | 1     | 2   | hit's x-position |
| t1h2_y       | cm   | 1     | 2   | hit's y-position |
| t1h2_z       | cm   | 1     | 2   | hit's z-position |
| t1h2_px      | 1    | 1     | 2   | photon momentum x-component |
| t1h2_py      | 1    | 1     | 2   | photon momentum y-component |
| t1h2_pz      | 1    | 1     | 2   | photon momentum z-component |
| t1h2_ed      | ps   | 1     | 2   | deposited energy during a hit (TOT) |
| t1h2_t       | ps   | 1     | 2   | hit window time |
| t1h2_lid     | 1    | 1     | 2   | layer ID where a hit occured |
| t2h2_x       | cm   | 2     | 2   | hit's x-position |
| t2h2_y       | cm   | 2     | 2   | hit's y-position |
| t2h2_z       | cm   | 2     | 2   | hit's z-position |
| t2h2_px      | 1    | 2     | 2   | photon momentum x-component |
| t2h2_py      | 1    | 2     | 2   | hoton momentum y-component |
| t2h2_pz      | 1    | 2     | 2   | photon momentum z-component |
| t2h2_ed      | ps   | 2     | 2   | deposited energy during a hit (TOT) |
| t2h2_t       | ps   | 2     | 2   | hit window time |
| t2h2_lid     | 1    | 2     | 2   | layer ID where a hit occured |
| theta_1      | deg  | 1     | 1   | scattering angle (Compton theta) |
| theta_2      | deg  | 2     | 1   | scattering angle (Compton theta) |
| delta_phi    | deg  | -     | -   | delta phi calculated with JPET method |