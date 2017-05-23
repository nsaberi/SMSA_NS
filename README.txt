The Satellite-based Microwave Snow Mapping Algorithm (SMSA) for passive microwave

Author: Richard Kelly
Address: University of Waterloo, Canada
email: rejkelly@uwaterloo.ca

This program produces snow depth & SWE and quality flags from JAXA AMSR2 orbital data for one L1R granule.

IMPORTANT NOTES
A. THE ALGORITHM REQUIRES THAT AMSR2 DATA ARE FED INTO THE ALGORITHM IN TIME ORDER THAT THEY WERE ACQUIRED. THEREFORE, PROCESSING MUST BE LINEAR.
   SEE THE INCLUDED SHELL SCRIPT FILE (granule_script_test_Oct12May13.sh) FOR AN EXAMPLE ON HOW TO RUN THE PROGRAM
B. THE ALGORITHM IS CALIBRATED TO WORK WITH DESCENDING PASSES ONLY.IN PRINCIPLE, IT IS POSSIBE TO IMPLEMENT IT WITH ASCENDING PASSES TOO BUT THIS IS NOT YET TESTED AND NOT ADVISED.
   SCIENTIFICALLY, USING NIGHT-TIME PASSES ONLY REDUCES THE IMPACT OF SNOWMELT MELT THAT MAKE IT PHYSICALLY IMPORSSIBLE TO RETRIEVE DEPTH/SWE.
C. THE CODE IS INSTALLED IN A BASH SHELL.
D. EXECUTING THE CODE SHOULD BEGIN AROUND 1 OCTOBER EACH YEAR WHEN N. HEMISPHERE SNOW IS MINIMAL AND S. HEMISPHERE SNOW IS MINIMAL. THE CODE WILL PROCESS SINGLE DAYS OR MULTIPLE DAYS 
   IF THE CODE STOPS, THERE IS NO NEED TO START FROM THE BEGINNING AS THE INTERNALDYNAMICPARMETER FILES KEEP TRACK OF TIMING. IT WILL ALSO DEAL WITH MISSING DATA - IF THE GAP IS SMALL, 
   THE PERSISTENCE FLAGS WILL USE THE PREVIOUS DATA (UP TO 10 DAYS) OTHERWISE IT WILL RE-ESTIMATE A SIMPLIFIED V1 ALGORITHM.


Installation instructions of the SMSA code:

1) unzip the code package SMSA-v2.0.1.tar.gz at a convenient location:
	% gunzip SMSA-v2.0.1.tar.gz

2) untar the code package AMSA-v.2.0.1.tar:
	% tar -cf SMSA-v2.0.1.tar

3) you should now have the following directory structure:

	algo_v2.0.1_delivered/
	algo_v2.0.1_delivered/IternalDynamicParameters
	algo_v2.0.1_delivered/IternalDynamicParameters/density
	algo_v2.0.1_delivered/IternalDynamicParameters/gs
	algo_v2.0.1_delivered/IternalDynamicParameters/snow
	algo_v2.0.1_delivered/L2_snow
	L1R_test_input/

   feel free to change the name of the directory to algo_v2.0.1 if necessary.

4) the SMSA code resides in the main directory <algo_v2.0.1_delivered>. Change directory to here and ensure that you have the following libraries installed before you try and compile:
	hdf5
	szip
	AMTK1.14
   These should be available of AMTK is installed but please make sure that the libraries are 'static' and not 'dynamic'.

5) ensure that you have the stack size (ulimit -s) set to something like 65000. 
	% ulimit -s 65000

6) ensure that the Makefile <Makefile_dmrtml_LUT> has the correct paths in the first three lines:
	AMTK_DIR = /home/rejkelly/AMSR2/SND_AMSR2_HDF5/AMTK_AMSR2_1.14
	HDF5_DIR = /usr/local/lib64/hdf5-1.8.5
	SZIP_DIR = /usr/local/lib/szip-2.1

7) make sure that your paths (bash) in your .bashrc or .bash_profile are correct for the hdf5 and szip libraries. Also ensure that the AMTK paths are present in the startup scripts (.bashrc or .bash_profile) and that the leapsec and geophysical_file are referenced:
e	xport LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib64/hdf5-1.8.5/lib:/usr/local/lib/szip-2.1/lib:/usr/local/lib:/usr/local/lib64:/usr/lib64
	export AMSR2_LEAP_DATA=/home/rejkelly/AMSR2/SND_AMSR2_HDF5/AMTK_AMSR2_1.14/share/data/leapsec.dat
	export GEOPHYSICALFILE=/home/rejkelly/AMSR2/SND_AMSR2_HDF5/AMTK_AMSR2_1.14/share/data/geophysical_file

8) compile the code using the Makefile. Either:
	a) copy the Makefile_dmrtml_LUT to Makefile 
		% cp Makefile_dmrtml_LUT Makefile
		% make
	or
	b) make the file using:
		% make -f Makefile_dmrtml_LUT
  if this is successful you should find and executable file in the directory called SD_SWE_SMSA_v2.0.1.  Here is an example of the comman line out put form the make comand:

	% make -f Makefile_dmrtml_LUT
gcc -O3 -cpp -I/home/rejkelly/AMSR2/SND_AMSR2_HDF5/AMTK_AMSR2_1.14/include -I/home/rejkelly/AMSR2/SND_AMSR2_HDF5/AMTK_AMSR2_1.14/src -I/usr/local/lib64/hdf5-1.8.5/include -c granule_v5_EORC_20160101_v2.0.1.c
gcc -O3 -cpp -I/home/rejkelly/AMSR2/SND_AMSR2_HDF5/AMTK_AMSR2_1.14/include -I/home/rejkelly/AMSR2/SND_AMSR2_HDF5/AMTK_AMSR2_1.14/src -I/usr/local/lib64/hdf5-1.8.5/include -c set_dt_str_utc_.c
gcc -O3 -cpp -I/home/rejkelly/AMSR2/SND_AMSR2_HDF5/AMTK_AMSR2_1.14/include -I/home/rejkelly/AMSR2/SND_AMSR2_HDF5/AMTK_AMSR2_1.14/src -I/usr/local/lib64/hdf5-1.8.5/include -c error_end.c
gcc -O3 -cpp -I/home/rejkelly/AMSR2/SND_AMSR2_HDF5/AMTK_AMSR2_1.14/include -I/home/rejkelly/AMSR2/SND_AMSR2_HDF5/AMTK_AMSR2_1.14/src -I/usr/local/lib64/hdf5-1.8.5/include -c funct_jdate.c
gcc -O3 -cpp -I/home/rejkelly/AMSR2/SND_AMSR2_HDF5/AMTK_AMSR2_1.14(/include -I/home/rejkelly/AMSR2/SND_AMSR2_HDF5/AMTK_AMSR2_1.14/src -I/usr/local/lib64/hdf5-1.8.5/include -c funct_lltoease2.c
gcc -O3 -cpp -I/home/rejkelly/AMSR2/SND_AMSR2_HDF5/AMTK_AMSR2_1.14/include -I/home/rejkelly/AMSR2/SND_AMSR2_HDF5/AMTK_AMSR2_1.14/src -I/usr/local/lib64/hdf5-1.8.5/include -c ReadDynamicInputs.c
gcc -O3 -cpp -I/home/rejkelly/AMSR2/SND_AMSR2_HDF5/AMTK_AMSR2_1.14/include -I/home/rejkelly/AMSR2/SND_AMSR2_HDF5/AMTK_AMSR2_1.14/src -I/usr/local/lib64/hdf5-1.8.5/include -c WriteDynamicOutputs.c
gcc -O3 -cpp -I/home/rejkelly/AMSR2/SND_AMSR2_HDF5/AMTK_AMSR2_1.14/include -I/home/rejkelly/AMSR2/SND_AMSR2_HDF5/AMTK_AMSR2_1.14/src -I/usr/local/lib64/hdf5-1.8.5/include -c retrieve_swe_sd_v2.0.1.c
gcc -O3 -cpp -I/home/rejkelly/AMSR2/SND_AMSR2_HDF5/AMTK_AMSR2_1.14/include -I/home/rejkelly/AMSR2/SND_AMSR2_HDF5/AMTK_AMSR2_1.14/src -I/usr/local/lib64/hdf5-1.8.5/include -c UpdateStorageArrays.c
gcc -o ./SD_SWE_SMSA_v2.0.1 -static granule_v5_EORC_20160101_v2.0.1.o set_dt_str_utc_.o error_end.o funct_jdate.o funct_lltoease2.o ReadDynamicInputs.o WriteDynamicOutputs.o retrieve_swe_sd_v2.0.1.o UpdateStorageArrays.o  -L/home/rejkelly/AMSR2/SND_AMSR2_HDF5/AMTK_AMSR2_1.14/lib -L/usr/local/lib64/hdf5-1.8.5/lib -L/usr/local/lib/szip-2.1/lib -lAMSR2 -lhdf5 /usr/local/lib/szip-2.1/lib/libsz.a -lz -lm 
rm -f *.o

created SD_SWE_SMSA_v2.0.1

9) Executing the code: 
   A. COMMAND LINE
	Usage (general):% ./SD_SWE_SMSA_v2.0.1 source_directory/InputGranuleFile.h5 destination_directory/OUtputGranuleFile.h5 1 2 3 4 5
	Usage (specific example):% ./SD_SWE_SMSA_v2.0.1 /home/AMSR2/L1R/2013_01/GW1AM2_201301010021_185D_L1SGRTBR_0000000.h5 L2_test/GW1AM2_201301010021_185D_L1SGRTBR_0000000.SDv2.h5 1 2 3 4 5
   In general, the input L1R granule must be located in a separate directory. The output L2 HDF5 file is placed in the destination directory. In the specific example, the destination is the L2_test/ directory and the source L1R folder is /home/AMSR2/L1R/2013_01/. NB Do not place source files in eth code directory.

   B. SHELL SCRIPT
	Two bash script files are included. If the code is compiled (see 8 above) then by ensuring that the paths and the executable file names are correct the code can be run. 
	  (i) granule_script_test_v2.0.1.sh	(consecutive Descending passes for 1-3 January 2013)
	  (ii) granule_script_test_Oct12May13.sh  (consecutive descending passes from 1 Oct 2012 to 31 May 2013)
	To execute the scripts type the following at the prompt:
	  % sh granule_script_test_v2.0.1.sh

   NB It should be possible to use the same scripting as currently used for the V1 algorithm currently in production. However, PLEASE ENSURE THAT THE PROCESSING IS DONE IN A LINEAR FASHION (NOT PARALLEL) AND THAT THE L1R FILES ARE SEQUENTIAL IN ACQUISITION TIME. ALSO, IT IS ESSENTIAL THAT ONLY DESCENDING PASSES ARE USED.

10) A. Output L2 HDF5 files. The HDF contains
    i. the geophysical data layer that stores two variables
	- Snow Depth (0) scaled to cm * 10 (or mm)
	- SWE (1) scaled to mm*10
    ii. flags layer
    iii. Other data

    B. Daily SD, SWE, GS, Density data stored in flat binary files. The code will also output daily averaged SWE, SD, Grain size and Density if required. 
       The variable 'outswitchtmp' should be set to 1 if you want to output these data.
	- SD Data (1440,2880) Filename is JulianData_sd.dat 2 byte integer. SD is stored as mm [cm *10]
	- SWE Data (1440,2880) Filename is JulianData_swe.dat 2 byte integer. SWE is stored as mm*10
	- Grain Size Data (1440,2880) Filename is JulianData_gs.dat Float. gs is stored as mm
	- Density Data (1440,2880) Filename is JulianData_den.dat Float. Density is stored as g/cm^3



Date: 25 July, 2016

