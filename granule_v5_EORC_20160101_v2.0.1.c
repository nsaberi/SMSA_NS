/*  granule_v5_EORC_20160101_v2.0.1.c

    Developed by Richard Kelly, University of Waterloo (rejkelly@uwaterloo.ca)
    25 July 2016

    This program produces snow depth & SWE and quality flags from JAXA AMSR2 orbital data for one granule.
    IMPORTANT NOTES
	A. THE ALGORITHM REQUIRES THAT AMSR2 DATA ARE FED INTO THE ALGORITHM IN TIME ORDER THAT THEY WERE ACQUIRED. THEREFORE, PROCESSING MUST BE LINEAR.
     	   SEE THE INCLUDED SHELL SCRIPT FILE (granule_script_test_Oct12May13.sh) FOR AN EXAMPLE ON HOW TO RUN THE PROGRAM
	B. THE ALGORITHM IS CALIBRATED TO WORK WITH DESCENDING PASSES ONLY.IN PRINCIPLE, IT IS POSSIBE TO IMPLEMENT IT WITH ASCENDING PASSES TOO BUT THIS IS NOT YET TESTED AND NOT ADVISED.
	   SCIENTIFICALLY, USING NIGHT-TIME PASSES ONLY REDUCES THE IMPACT OF SNOWMELT MELT THAT MAKE IT PHYSICALLY IMPORSSIBLE TO RETRIEVE DEPTH/SWE
	C. THE CODE IS INSTALLED IN A BASH SHELL.
	D. EXECUTING THE CODE SHOULD BEGIN AROUND 1 OCTOBER EACH YEAR WHEN N. HEMISPHERE SNOW IS MINIMAL AND S. HEMISPHERE SNOW IS MINIMAL. THE CODE WILL PROCESS SINGLE DAYS OR MULTIPLE DAYS 
	   IF THE CODE STOPS, THERE IS NO NEED TO START FROM THE BEGINNING AS THE INTERNALDYNAMICPARMETER FILES KEEP TRACK OF TIMING. IT WILL ALSO DEAL WITH MISSING DATA - IF THE GAP IS SMALL, 
	   THE PERSISTENCE FLAGS WILL USE THE PREVIOUS DATA (UP TO 10 DAYS) OTHERWISE IT WILL RE-ESTIMATE A SIMPLIFIED V1 ALGORITHM.
	   

    Usage (general): ./SD_SWE_SMSA_v2.0.1 source_directory/InputGranuleFile.h5 destination_directory/OUtputGranuleFile.h5 1 2 3 4 5
    Usage (specific example): ./SD_SWE_SMSA_v2.0.1 /home/AMSR2/L1R/2013_01/GW1AM2_201301010021_185D_L1SGRTBR_0000000.h5 L2_test/GW1AM2_201301010021_185D_L1SGRTBR_0000000.SDv2.h5 1 2 3 4 5

    Code required (written in ANSI c):
	error_end.c  				:: error definition (from JAXA)
	set_dt_str_utc_.c			:: gets date/time (from JAXA)
	funct_jdate.c				:: obtain julian date)
	funct_lltoease2.c			:: calculate EASE2.0 coordinate from Lat/Lon coordinate)
	granule_v5_EORC_20160101_v2.0.1.c	:: main program that calls all other functions
							- gets ancillay data
							- calls L2A read/write routines
							- calculates presence/absence of snow and assigns flags
							- calls retrieval routine
	retrieve_swe_sd_v2.0.1.c		:: called from granule_v5_EORC_20160101_v2.0.1.c performs main snow depth and SWE retrieval.
	ReadDynamicInputs.c			:: read dyanmic data needed in flags and SWE/SD retrieval routine (before the retrieval)
	WriteDynamicOutputs.c			:: writes dynamic arrays to file (after the retrieval)
	UpdateStorageArrays.c			:: at the end of each day, update dynamic arrays from daily statistics files
	global.h				:: include definition file (same as first version of AMSR2)

    Libraries:
    To compile (see Makefile_dmrtml_LUT) include the standard AMSR2 libs from AMTK 1.14. Also ensure that the libraries are static and not dynamic (HDF5, szip)
    
    Compilation:
    Run the Makefile_dmrtml_LUT:  make -f Makefile_dmrtml_LUT. Executable filename is SD_SWE_SMSA_v2.0.1

    Code was developed for compilation under CentOS (CentOS Linux release 7.2.1511 (Core)) installed on a 64 Bit Windows 8.1 operating system with 32Gb RAM. The processors
    is Intel Core I7-4790 CPU @ 3.60 GHz. Execution of code to process one day's worth of data takes about 24 seconds.

    Ancillary Data Files (all '*.bin' files are 1440x2880 EASE Grid NH & SH Projection maps. Northern Hemisphere is (0:1439,0:1439) and Southern Hemisphere is (0:1439,1440:2879). 
    See NSIDC.org for details on EASE grid 2.0 projection (http://nsidc.org/data/ease/ease_grid2.html)
	"MOD12Q1IGBP_land_ice_SnowPossible_byte_majority_EASE2.bin" 	(byte: total=4,147,200 bytes)	[Land ice and snow climatology mask]
									Permanent ice: 64. Land snow possible: 8. Land snow impossible: 29. Oceans/Water: 0.
	"lol_v2_12_5km_ave_EASE2.bin"   				(byte: total=4,147,200 bytes)	[% Water Fraction ranges from 0-100]
	"TransmissivityCalculated_12_5km_EASE2.bin" 			(float: tota)=16,588,800 bytes) [% transmissivity ranges from 0-100]
	"elevation_12_5km_EASE2.bin"     				(2 byte integer: total=8,294,400 bytes) [height in metres above sea level]
	"SnowClass_Sturm_EASE2.bin"     				(byte: total=4,147,200 bytes)	[Sturm  et al. 1995 Seasonal Snow classes - used in snow density estimation]

	"DMRTML_LUT_d1836_v2-1-200cm.dat"				(2 byte integer: 18,000,000 bytes) [DMRT Look up table. Used in Retrieval.]
									dimensions: [25][20][45][200][2] of Tb difference 18V-36V ([*][*][*][*][0]) and 18H-36H ([*][*][*][*][1]
									[25][][][][] is Tphys from 225-273K in 2K increments
									[][20][][][] is density from 0.2-0.6 g/cm^3 in 0.02 g/cm^3 increments
									[][][45][][] is grain size from 0.1-0.98 mm in 0.02mm increments
									[][][][200][] is snowdepth from 0.01m-2.00m in 0.01m increments

    Internal Dynamic Parameter Files.
    22 Internal dynamic files are created (at first run) and re-used and updated continuously during the algorithm execution.
    **THESE FILES MUST NOT BE DELETED**
    Files are EASE grid V2.0 in size and contain between 1-11 days of historical data (for tracking purposes)
          InternalDynamicParameters/dysta_1836v.dat	tracks current day's Tb18V-Tb36V Tb data [1440,2880,2] from multiple granules for the day
          InternalDynamicParameters/dysta_1836h.dat	tracks current day's Tb18H-Tb36H Tb data [1440,2880,2] from multiple granules for the day
          InternalDynamicParameters/dysta_tphys.dat	tracks current day's Estimated Tphys [1440,2880,2] from multiple granules for the day
          InternalDynamicParameters/dysta_fet.dat	tracks current day's facet evolution time [1440,2880,2] from multiple granules for the day
          InternalDynamicParameters/dysta_gs.dat 	tracks current day's grain size estimates [1440,2880,2] from multiple granules for the day
          InternalDynamicParameters/dysta_den.dat	tracks current day's density estimates [1440,2880,2] 	 from multiple granules for the day
          InternalDynamicParameters/dysta_evol.dat	tracks current day's duration of snow presence [1440,2880,2] 	 from multiple granules for the day
          InternalDynamicParameters/dysta_reg.dat 	tracks current day's grain growth regime [1440,2880,2] from multiple granules for the day
          InternalDynamicParameters/dysta_swe.dat 	tracks current day's SWE [1440,2880,2]	 from multiple granules for the day
          InternalDynamicParameters/dysta_sd.dat 	tracks current day's snow depth [1440,2880,2]	 from multiple granules for the day
          InternalDynamicParameters/SCAhistory.dat	track 11 days of snow presence/absence (Julian date is used to track) [1440,2880,11]	
          InternalDynamicParameters/thistory.dat 	track 11 days of tphys estimates - was an AMSR2 obs present (Julian date is used to track) [1440,2880,11]	
          InternalDynamicParameters/SDhistoryInst.dat   track 10 days of snow depth estimation [1440][2880][10]	
          InternalDynamicParameters/T1836V-10days-GaussianSmoothed.dat 	track 10 days of gaussian smoothed Tb18V-Tb36V [1440][2880][10]
          InternalDynamicParameters/T1836H-10days-GaussianSmoothed.dat 	track 10 days of gaussian smoothed Tb18H-Tb36H [1440][2880][10]
          InternalDynamicParameters/T_est.dat		track 11 days of tphys estimates (Kelvin) [1440,2880,11]
          InternalDynamicParameters/regime.dat		track 1 day of grain size growth regime [1,2,3]) [1440,2880]
          InternalDynamicParameters/fet.dat		track 1 day of facet evolution growth duration (#days) [1440,2880]
          InternalDynamicParameters/evolution.dat	track 1 day of snow pack duration (#days) [1440,2880]	
          InternalDynamicParameters/gs.dat		track 1 day of grain size (m) [1440,2880]
          InternalDynamicParameters/density.dat 	track 1 day of density (g/cm^3) [1440,2880]
          InternalDynamicParameters/persistence.dat 	track 10 days of snow persistence (days) [1440,2880,10]

    Output:
	A. HDF5 granule files (as V1 algorithm but with SWE included). These are written to the L2_test directory. SD and SWE are written along with the flags.
	   Snow depth is stored in the granule HDF5 output as cmx10
	   SWE is stored stored in the granule HDF5 output as mm*10
	B. Daily SD, SWE, GS, Density data stored in flat binary files
	   The code will also output daily averaged SWE, SD, Grain size and Density if required. The variable 'outswitchtmp' should be set to 1 if you want to output these data.
	   SD Data (1440,2880) Filename is JulianData_sd.dat 2 byte integer. SD is stored as mm [cm *10]
	   SWE Data (1440,2880) Filename is JulianData_swe.dat 2 byte integer. SWE is stored as mm*10
	   Grain Size Data (1440,2880) Filename is JulianData_gs.dat Float. gs is stored as mm
	   Density Data (1440,2880) Filename is JulianData_den.dat Float. Density is stored as g/cm^3
    
    The flags array (bits 7,6,5,4,3,2,1,0) has the following meaning:
    .	Bit 7		Unused
    .	Bit 6		Sea ice
    .	Bit 5		Land surface: 0=smooth, 1=rough
    .	Bit 4		Land: 0=snow possible, 1=snow impossible
    .	Bit 3		0=water, 1=land
    .	Bit 2-0		Snow class			


*/

#define __GLOBAL__ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <AMTK.h>
#include "global.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

#ifdef ADDITIONAL_SNOW_LOGIC

#include <AMTK_PLUS.h>
#include <GMTK.h>

#endif


/*****  #define ADDITIONAL_SNOW_LOGIC  *****/

#define SWE_TO_SNOW	1.0


#define ERROR	-1

/* 2005.09.22 RESTEC */
#define SND_FAIL -1

#define LAND_MASK_DSN  "MOD12Q1IGBP_land_ice_SnowPossible_byte_majority_EASE2.bin"	/*------  JANUARY 2016  ---------*/
#define WATER_FRACTION_DSN "lol_v2_12_5km_ave_EASE2.bin"    				/*------  JANUARY 2016  ---------*/
#define FOREST_TRANS_DSN "TransmissivityCalculated_12_5km_EASE2.bin"    		/*------  JANUARY 2016  ---------*/
#define ELEVATION_DSN "elevation_12_5km_EASE2.bin"     					/*------  JANUARY 2016  ---------*/
#define SNOWCLASS_DSN "SnowClass_Sturm_EASE2.bin"     					/*------  JANUARY 2016  ---------*/
#define DMRTLUT_DSN "DMRTML_LUT_d1836_v2-1-200cm.dat"     						/*------  JANUARY 2016  ---------*/

#define DYNAMIC_SCAhistory "InternalDynamicParameters/SCAhistory.dat"

#define ROWS	1800            /*2013 April UW MODIFY*/
#define COLS	3600            /*2013 April UW MODIFY*/
#define ROWS2	1440            /*------  JANUARY 2015  ---------*/
#define COLS2	2880            /*------  JANUARY 2015  ---------*/
#define ROWS_E2	2880            /*------  JANUARY 2016  ---------*/
#define COLS_E2	1440            /*------  JANUARY 2016  ---------*/
#define FORESTROWS	21600
#define FORESTCOLS	43200

#define V10	2
#define H10	3

#define V18	4
#define H18	5
#define V23	6
#define H23	7
#define V36	8
#define H36	9

#define V89     12
#define H89     13

#define V89A	0
#define V89B	1
#define H89A	2
#define H89B	3

#define LAT	0
#define LON	1

#define MAX_SNOW_CLASS	7
#define NO_SNOW		0

/* 2010/01 RESTEC */
/* #define UNKNOWN_SNOW	9999 */
#define UNKNOWN_SNOW 65535.0

/*** Bit flags for the flags array. (Bits 0,1,2 are for SnowClass) ***/
#define LAND			0x08
#define SNOW_IMPOSSIBLE		0x10
#define ROUGH			0x20
#define ICE			0x40

/*** Possible values for the SNOW output flags.  ***/
/* 2010/01 RESTEC modify */
#define FLAG_SNOW_POSSIBLE      0
#define FLAG_NORMAL_NOSNOW	1

/* Static landcover type or satellite quality flags [used] MINOR UPDATES*/
#define FLAG_OCEAN		16
#define FLAG_LAKE		8
#define FLAG_SNOW_IMPOSSIBLE	32
#define FLAG_PERMANENT_ICE	48	
#define FLAG_OUTOFRANGE		192
#define FLAG_SAT_ATT            208
#define FLAG_BAD_TB             224

/* Dynamic Geophysical Flags [used] NEW*/
#define FLAG_WET_SNOW		128	/*------ JANUARY 2015 ------*/
#define FLAG_DRY_SNOW		144	/*------ JANUARY 2015 ------*/
#define FLAG_DRY_SNOW_PERSIST	145	/*------ JANUARY 2015 ------*/
#define FLAG_LAKE_ICE		64	/*------ JANUARY 2015 ------*/
#define FLAG_SNOW_HIGHLAT	162	/*------ JANUARY 2015 ------*/
#define FLAG_TP_NOSNOW		168	/*------ JANUARY 2015 ------*/
#define FLAG_SHALLOW_SNOW	150	/*------ JANUARY 2015 ------*/

/* UNUSED FLAGS (UPDATED)*/
#define FLAG_SURFACE_TOO_WARM	255
#define FLAG_FOREST		255
#define FLAG_MOUNTAIN		255
#define FLAG_RAINING		255
#define FLAG_WET_SOIL		255
#define FLAG_DRY_SOIL		255
#define FLAG_NO_DATA_DENSITY    255
#define GA_MAX 3

#define NO_OBSERVATION 65535.0
#define ABNORMAL_TB    65534.0

#undef  M_PI
#define M_PI (3.14159265358979323846)

/* function declaration */
  int 	ReadDynamicInputs( void );
  int 	WriteDynamicOutputs( void );
  int 	UpdateStorageArrays( void );
  float retrieve_swe_sd( int *col, int *row, double *juliandate, int *retrievalflag, float *EstSD, float *EstSWE);
  int 	jdate(int num1, int num2, int num3);

  unsigned char	flags[ROWS_E2][COLS_E2];		/*----- JANUARY 2016  ---------*/
  unsigned char   WaterFraction[ROWS_E2][COLS_E2];	/*----- JANUARY 2016  ---------*/
  float		transmissivity[ROWS_E2][COLS_E2];	/*----- JANUARY 2016 --------- */
  short int 	elevation[ROWS_E2][COLS_E2];		/*----- JANUARY 2016 --------- */
  unsigned char	snowclass[ROWS_E2][COLS_E2];		/*----- JANUARY 2016  ---------*/

  void set_dt_str_utc(char *buf);
  void error_end(int, char*);
  void get_surftemp();
  void lltoease2(float *plat, float*plon);



  /* GLobal Dynamic File Declarations */
  short int 	sdhistory[ROWS_E2][COLS_E2][10],tb1836vf_smth[ROWS_E2][COLS_E2][10],tb1836hf_smth[ROWS_E2][COLS_E2][10],persistence[ROWS_E2][COLS_E2][2];
  short int	dayswe[ROWS_E2][COLS_E2],daysd[ROWS_E2][COLS_E2],daycoverage[ROWS_E2][COLS_E2];
  float		t_est[ROWS_E2][COLS_E2][11],gs[ROWS_E2][COLS_E2],initvol[ROWS_E2][COLS_E2],denest[ROWS_E2][COLS_E2];
  float		fet[ROWS_E2][COLS_E2];
  float		dysta_tphys[ROWS_E2][COLS_E2][2],dysta_fet[ROWS_E2][COLS_E2][2],dysta_gs[ROWS_E2][COLS_E2][2],dysta_den[ROWS_E2][COLS_E2][2];
  int		dysta_evol[ROWS_E2][COLS_E2];
  float		dysta_sd[ROWS_E2][COLS_E2][2],dysta_swe[ROWS_E2][COLS_E2][2];
  float		dysta_1836v[ROWS_E2][COLS_E2][2],dysta_1836h[ROWS_E2][COLS_E2][2];
  int		regime[ROWS_E2][COLS_E2],dysta_reg[ROWS_E2][COLS_E2][3],evolution[ROWS_E2][COLS_E2];
  double	thistory[ROWS_E2][COLS_E2][11],scahistory[ROWS_E2][COLS_E2][11];
  float		v10,h10,v18, h18,v23, h23, v36, h36, v89,h89;
  float		v10f,h10f,v18f, h18f,v23f, h23f, v36f, h36f, v89f,h89f; /*----- JANUARY 2015 --------- */
  float		klvn,EstSD,EstSWE;
  float		minsnowlimit=0.02;
  int		plimit=10;
  double	scat, sc1036v,sc1036h, sc1836v,sc1836h, sc89v,sc89h, sc36,sc89,scx;
  double	snow, albedo, coef, FloatSnow,pol36fact;
  double	pol18f,pol36f,pol89f,t1836vf,t1836hf,t2389hf,t89v36hf,t18h23vf,t3689vf ;  /*----- JANUARY 2015 --------- */
  short int	DMRT_LUT[25][20][45][200][2];
  int		outswitch,outswitchtmp;
  char		sJulian[8];
  char		debug_main=0,debug_retr=0, debug_read=0,debug_write=0,debug_upd=0;

/*==================================================================================*/
int main(int argc, char* argv[])
{
  float		longitude,latitude;
  int		FlagHandle, save_iobsline, save_iobscan, MsgSize;
  float		month, csec,cmin,chour,cday,cmonth,cyear,cjdate;
  double	retf,datediff;
  int		FlagHandleFT, FlagHandleE;  		/*----- JANUARY 2015 --------- */
  int           FlagHandleW;     			/*------  OCTOBER 2012  ---------*/
  int           FlagHandleSC,FlagHandleLUT;		/*------  OCTOBER 2012  ---------*/
  int		row, col, line, samp, SnowClass, n3;
  int		row2,col2,elev, tmpflag, retr_flag;  	    /*----- JANUARY 2015 --------- */
  int		easex,easey;  				    /*----- JANUARY 2016 --------- */
  float		zeasex,zeasey,retrSD,retrSWE;		    /*----- JANUARY 2016 --------- */
  float		ft,tp_ratio;  		    		    /*----- JANUARY 2015 --------- */
  char		message[512];
  float		wf;
  float		t1036v, t1036h, t1018h, t1018v, t1836v, t1836h, t2389v, t2389h;
  float		xlon,ylat ,llfcol,llfrow ;
  float 	snowct=0;

//  int 		a = 100;
//  int 		b = 200;


  /* 2003 Nov.10 RESTEC */
  float value_SatAtt;
  int jp;
  float limit_v = 0.01;

  /* 2005.09.22 RESTEC */
  long pntr;

  /* 2010/01 RESTEC */
  char cts[24];  /* current time str */
  char  buf[256]; /* message buffer */
  int  dim_size[3];
  int  err_code;
  int  file_id;
  void *ptr;
  int  scan_start, scan_end;
  int  i,j,k,kk;
  int  ret;
  char proc_kind[8]; 
  unsigned char  quality[16800][243];
  FILE *fp;
  float psb = 0.;   /* possible pixel*/
  float ipsb = 0.;  /* impossibule pixel*/
  float ocean = 0.; /* ocean pixel*/
  float lakew = 0.; /* lake water pixel*/
  float lakei = 0.; /* lake ice pixel*/
  float ice = 0.;   /* ice pixel*/
  int sqf;        /* science quality flag */

  // A+D offset  2015/02 RESTEC changed
  float slope_ad[] = {-0.01390, -0.00940, -0.01289, -0.00221, -0.04524,
                      -0.00858, -0.00957, -0.00947, -0.01019, -0.00561,
                      1.0, 1.0, -0.01403, -0.00980 };

  float offset_ad[] = {3.67421 , 3.03663 , 6.34775 , 3.79624 ,
                       12.57562 , 1.89574 , 4.40435 , 4.18710 ,
                     5.49799 , 4.19181 , 0.0, 0.0, 5.32379 , 3.75174 };

  float *pslope, *poffset;

  set_dt_str_utc(cts);
  if (debug_main ==1) fprintf(stderr, "%s %s\n",cts, "*** start SND algorithm program");

  /* input file set */
  if(argc != 8) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "Missing argument");
    err_code = 101;
    error_end(err_code, buf);
  }

  for(i=1; i<8; i++) {
  }

/* GET ARGUMENTS FROM COMMAND LINE */
  strcpy(L1R_f, argv[1]);
  strcpy(L2_f, argv[2]);
  strcpy(proc_kind, argv[3]);
  strcpy((char*)Ga_file[0], argv[4]);
  strcpy((char*)Ga_file[1], argv[5]);
  strcpy((char*)Ga_file[2], argv[6]);
  strcpy((char*)Ga_file[3], "9999");
  strcpy((char*)Ga_file[4], "9999");

  if(strcmp(proc_kind, "SG") == 0) {
/* Non JMA Processing*/
      ProcKind = 1;
  }
  else {
/* JMA Processing*/
    ProcKind = 2;
  }

  set_dt_str_utc(cts);

  /* L1R open */
  file_id = AMTK_openH5(L1R_f);
  if(file_id < 0){
    memset(buf, '\0', sizeof(buf)); 
    sprintf(buf, "%s  %s %d ",L1R_f," AMTK status : ", file_id);
    err_code = 53;
    error_end(err_code, buf);
  }

  /* get number of scans */
  ptr = buf;
  ret = AMTK_getMetaDataName(file_id, "NumberOfScans", (char**)&ptr);
  if(ret < 0){
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s  %s %d ","meta data(NumberOfScans) get error","AMTK status : ", ret);
    err_code = 53;
    error_end(err_code, buf);
  }

  iobsline = atoi(buf);
  iobscan = 243;


  /*** Get the month from the first line of the observation time data.  ***/
  scan_start = 1;
  scan_end   = iobsline;
  ptr = ObsTime;
  if((ret = AMTK_getScanTime(file_id, scan_start, scan_end, (AM2_COMMON_SCANTIME **)&ptr))< 0) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s  %s %d ","Scantime get error","AMTK status : ", ret);
    err_code = 53;
    error_end(err_code, buf);
  }

  month = ObsTime[0].month - 1;  
  cmonth = ObsTime[0].month ;
  cyear = ObsTime[0].year;
  cday = ObsTime[0].day;
  chour=ObsTime[0].hour;
  cmin=ObsTime[0].minute;
  csec=ObsTime[0].second;

     /* calling a function to get Julian Date (decimal) */
   retf = jdate(cyear,cmonth,cday)+(((cmin*60.)+(chour*3600.)+csec)/86400.);

  /*** Read flags array showing land/water/ice, level/mountain, snow possible/impossible,
       and snow class. ***/

  /*OPEN & Read Land Mask File (MOD12Q1IGBP_land_ice_SnowPossible_byte_majority_EASE2.bin) */
  if( (FlagHandle = open( LAND_MASK_DSN, O_RDONLY)) == -1 ) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s","Cannot open the land mask file.");
    err_code = 91;
    error_end(err_code, buf);
  }

  if( read( FlagHandle, (char *)flags[0], sizeof(flags) ) != sizeof( flags ) ) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s","Cannot read the land mask file.");
    err_code = 91;
    error_end(err_code, buf);
  }

  if( close( FlagHandle )) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s","Cannot close the land mask file.");
    err_code = 91;
    error_end(err_code, buf);
  }

  /*OPEN & Read Water Fraction Mask File (lol_v2_12_5km_ave_EASE2.bin) */
  if( (FlagHandleW = open( WATER_FRACTION_DSN, O_RDONLY)) == -1 ) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s","Cannot open the water fraction file.");
    err_code = 91;
    error_end(err_code, buf);
  }

  if( read( FlagHandleW, (char *)WaterFraction[0], sizeof(flags) ) != sizeof( flags ) ) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s","Cannot read the water fraction file.");
    err_code = 91;
    error_end(err_code, buf);
  }

  if( close( FlagHandleW )) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s","Cannot close the water fraction file.");
    err_code = 91;
    error_end(err_code, buf);
  }

  /*OPEN & Read Forest Transmissivity File (TransmissivityCalculated_12_5km_EASE2.bin) */
  if( (FlagHandleFT = open( FOREST_TRANS_DSN, O_RDONLY)) == -1 ) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s","Cannot open the forest transmissivity file.");
    err_code = 91;
    error_end(err_code, buf);
  }

  if( read( FlagHandleFT, (char *)transmissivity[0], sizeof(transmissivity) ) != sizeof( transmissivity ) ) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s","Cannot read the forest transmissivity file.");
    err_code = 91;
    error_end(err_code, buf);
  }

  if( close( FlagHandleFT )) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s","Cannot close the forest transmissivity file.");
    err_code = 91;
    error_end(err_code, buf);
  }

  /*OPEN & Read Elevation File (elevation_12_5km_EASE2.bin) */
  if( (FlagHandleE = open( ELEVATION_DSN, O_RDONLY)) == -1 ) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s","Cannot open the elevation file.");
    err_code = 91;
    error_end(err_code, buf);
  }

  if( read( FlagHandleE, (char *)elevation[0], sizeof(elevation) ) != sizeof( elevation ) ) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s","Cannot read the elevation file.");
    err_code = 91;
    error_end(err_code, buf);
  }

  if( close( FlagHandleE )) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s","Cannot close the elevation file.");
    err_code = 91;
    error_end(err_code, buf);
  }

  /*OPEN & Read Snow Climatology Class file (SnowClass_Sturm_EASE2.bin) 12.5 km EASE2 */
  if( (FlagHandleSC = open( SNOWCLASS_DSN, O_RDONLY)) == -1 ) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s","Cannot open the Sturm Snow Class file.");
    err_code = 91;
    error_end(err_code, buf);
  }

  if( read( FlagHandleSC, (char *)snowclass[0], sizeof(snowclass) ) != sizeof( snowclass ) ) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s","Cannot read the Snow Class file.");
    err_code = 91;
    error_end(err_code, buf);
  }

  if( close( FlagHandleSC )) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s","Cannot close the snow class file.");
    err_code = 91;
    error_end(err_code, buf);
  }

  /*OPEN & Read DMRT-ML Look-up Table (LUT) file (DMRTML_LUT_d1836.dat) */
  if( (FlagHandleLUT = open( DMRTLUT_DSN, O_RDONLY)) == -1 ) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s","Cannot open the DMRT-ML Look-ip Table file.");
    err_code = 91;
    error_end(err_code, buf);
  }

  if( read( FlagHandleLUT, (int *)DMRT_LUT[0], sizeof(DMRT_LUT) ) != sizeof( DMRT_LUT ) ) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s","Cannot read the DMRT-LUT file.");
    err_code = 91;
    error_end(err_code, buf);
  }

  if( close( FlagHandleLUT )) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s","Cannot close the DMRT-ML Look-up Table file.");
    err_code = 91;
    error_end(err_code, buf);
  }

/*-----------------------------------------------------------*/
/*-----------------------------------------------------------*/
/*------  JANUARY 2016 OPEN AND READ DYNAMIC FILES  ---------*/

  if( ReadDynamicInputs() != 0 ) return(-1);
  if (debug_main ==1) fprintf(stderr, "%s %s\n",cts, "Finished Reading Dynamic history files");
  outswitch=0;		/* Flag to write history file out */
  outswitchtmp=1;	/* Flag to output daily SWE, SD, GS, DEN see WriteDynamicOUtputs.c. Set to 1 if you want outputs, 0 if not. */

/* CHECK TO SEE IF THIS IS A NEW DAY: IF THIS GRANULE IS, RESET/UPDATE STORAGE ARRAYS */
  if ((retf-dysta_tphys[0][0][0]) < 0 && dysta_tphys[0][0][0] > 0 ){
    datediff=(int)retf-(int)dysta_tphys[0][0][0];
    printf(" Cannot process granules before current day. ** Granule must be entered into algorithm in acquisition time order sequence ** \n");
    printf(" Trying to process julian day: %10.3f on processing sequence day: %10.3f %f \n",retf,dysta_tphys[0][0][0],datediff );
    printf(" Skipping this granule and moving on...\n");
    exit(0);
  }
/* CHECK TO SEE IF THIS IS A NEW DAY: IF THIS GRANULE IS, RESET/UPDATE STORAGE ARRAYS */
  if (((int)retf-(int)dysta_tphys[0][0][0]) != 0) {
    outswitch = 1;
    snprintf(sJulian, 8, "%f", retf-1);		/*Get Julian Date (not decimals) for output file */
    if (UpdateStorageArrays() != 0) return(-1);
    /*Zero out daily temporary arrays*/
    memset(dysta_tphys,0,sizeof(dysta_tphys));
    memset(dysta_gs,0,sizeof(dysta_gs));
    memset(dysta_den,0,sizeof(dysta_den));
    memset(dysta_fet,0,sizeof(dysta_fet));
    memset(dysta_reg,0,sizeof(dysta_reg));
    memset(dysta_evol,0,sizeof(dysta_evol));
    memset(dysta_sd,0,sizeof(dysta_sd));
    memset(dysta_swe,0,sizeof(dysta_swe));
    memset(dysta_1836v,0,sizeof(dysta_1836v));
    memset(dysta_1836h,0,sizeof(dysta_1836h));
    memset(daycoverage,0,sizeof(daycoverage));
    /*Place Julian Date in TLC of dysta_phys[0][0][0]*/
    dysta_tphys[0][0][0]=(int)(retf);  
    fprintf(stderr, "%s Processing Julian Day: %10.3f, Gregorian date (dd:mm:yyyy):  %2.0f : %2.0f : %4.0f  ***********\n",cts,retf,cday,cmonth,cyear);
    if (debug_main ==1)     fprintf(stderr, "%s %s %f\n",cts, "Reset Daily Statistics files (tphys, FET, GS, REG, EVOL",dysta_tphys[0][0][0]);
    if (debug_main ==1)     fprintf(stderr, "%s %s\n",cts, "Finished Updating Storage Arrays");
  }
/*-----------------------------------------------------------*/
/*-----------------------------------------------------------*/
    
/*------  OCTOBER 2012  ---------*/

  /* Brightness Temperature (10.7GHz, V) */
  ptr = L1rtb[2];
  if((ret = AMTK_get_SwathFloat(file_id, (float**)&ptr, scan_start, scan_end, AM2_RES10_TB10V)) < 0) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s  %s %d ","Cannot get Brightness Temperature(10.7G_V)","AMTK status : ", ret);
    err_code = 53;
    error_end(err_code, buf);
  }

  /* Brightness Temperature (10.7GHz, H) */
  ptr = L1rtb[3];
  if((ret = AMTK_get_SwathFloat(file_id, (float**)&ptr, scan_start, scan_end, AM2_RES10_TB10H)) < 0) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s  %s %d ","Cannot get Brightness Temperature(10.7G_H)","AMTK status : ", ret);
    err_code = 53;
    error_end(err_code, buf);
  }

  /* Brightness Temperature (18.7GHz, V) */
  ptr = L1rtb[4];
  if((ret = AMTK_get_SwathFloat(file_id, (float**)&ptr, scan_start, scan_end, AM2_RES23_TB18V)) < 0) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s  %s %d ","Cannot get Brightness Temperature(18.7G_V)","AMTK status : ", ret);
    err_code = 53;
    error_end(err_code, buf);
  }

  /* Brightness Temperature (18.7GHz, H) */
  ptr = L1rtb[5];
  if((ret = AMTK_get_SwathFloat(file_id, (float**)&ptr, scan_start, scan_end, AM2_RES23_TB18H)) < 0) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s  %s %d ","Cannot get Brightness Temperature(18.7G_H)","AMTK status : ", ret);
    err_code = 53;
    error_end(err_code, buf);
  }

  /* Brightness Temperature (23.8GHz, V) */
  ptr = L1rtb[6];
  if((ret = AMTK_get_SwathFloat(file_id, (float**)&ptr, scan_start, scan_end, AM2_RES23_TB23V)) < 0) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s  %s %d ","Cannot get Brightness Temperature(23.8G_V)","AMTK status : ", ret);
    err_code = 53;
    error_end(err_code, buf);
  }

  /* Brightness Temperature (23.8GHz, H) */
  ptr = L1rtb[7];
  if((ret = AMTK_get_SwathFloat(file_id, (float**)&ptr, scan_start, scan_end, AM2_RES23_TB23H)) < 0) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s  %s %d ","Cannot get Brightness Temperature(23.8G_H)","AMTK status : ", ret);
    err_code = 53;
    error_end(err_code, buf);
  }

  /* Brightness Temperature (36.5GHz, V) */
  ptr = L1rtb[8];
  if((ret = AMTK_get_SwathFloat(file_id, (float**)&ptr, scan_start, scan_end, AM2_RES36_TB36V)) < 0) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s  %s %d ","Cannot get Brightness Temperature(36.5G_V)","AMTK status : ", ret);
    err_code = 53;
    error_end(err_code, buf);
  }

  /* Brightness Temperature (36.5GHz, H) */
  ptr = L1rtb[9];
  if((ret = AMTK_get_SwathFloat(file_id, (float**)&ptr, scan_start, scan_end, AM2_RES36_TB36H)) < 0 ) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s  %s %d ","Cannot get Brightness Temperature(36.5G_H)","AMTK status : ", ret);
    err_code = 53;
    error_end(err_code, buf);
  }

  /* Brightness Temperature (89.0GHz-B, Res.36GV) */  //2015/02 RESTEC changed
  ptr = L1rtb[12];
  if((ret = AMTK_get_SwathFloat(file_id, (float**)&ptr, scan_start, scan_end, AM2_RES36_TB89V)) < 0) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s  %s %d ","Cannot get Brightness Temperature(89.0G_B_V)","AMTK status : ", ret);
    err_code = 53;
    error_end(err_code, buf);
  }


  /* Brightness Temperature (89.0GHz-B, H) */  //2015/02 RESTEC changed
  ptr = L1rtb[13];
  if((ret = AMTK_get_SwathFloat(file_id, (float**)&ptr, scan_start, scan_end, AM2_RES36_TB89H)) < 0) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s  %s %d ","Cannot get Brightness Temperature(89.0G_B_H)","AMTK status : ", ret);
    err_code = 53;
    error_end(err_code, buf);
  }

  save_iobsline = iobsline;
  save_iobscan = 243;


  ptr = buf;
  ret = AMTK_getMetaDataName(file_id, "OrbitDirection", (char**)&ptr);
  if(ret < 0){
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s  %s %d ","meta data(OrbitDirection) get error","AMTK status : ", ret);
    err_code = 53;
    error_end(err_code, buf);
  }

  /* TB offset 2013/03 RESTEC */
  pslope = (float*)slope_ad;
  poffset = (float*)offset_ad;


  for(i=0; i<iobsline; i++)
  {
    for(j=0; j<243; j++)
    {
      for(k=0; k<14; k++)
      {
          if(k==0 || k==1 || k==10 || k==11) continue;
          if(L1rtb[k][i][j] < 10.0 ) continue;
          L1rtb[k][i][j] = L1rtb[k][i][j]-(L1rtb[k][i][j]*pslope[k]+poffset[k]);
      }
    }
  
  }

  /*** Read AMSR latitude/longitude observation points 89GHz-A (0, 2, 4...).
       Latitude: +90.0 to -90.0    Longitude: -180.0 to +180.0	***/
   ptr = LatLon;
   if((ret = AMTK_getLatLon( file_id, (AM2_COMMON_LATLON **)&ptr, scan_start, scan_end, AM2_LATLON_RS_LO)) < 0)
   {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s  %s %d ","Cannot get latitude/longitude observation points","AMTK status : ", ret);
    err_code = 53;
    error_end(err_code, buf);
   }

/*****************************************************************************
2003 Nov.10 RESTEC :Using "SatAtt",if bad-value,skip calculation. 
*****************************************************************************/
  /*** Read Satellite Attitude Data ***/
  ptr = SatAtt;
  if((ret = AMTK_get_SwathFloat(file_id, (float**)&ptr, scan_start, scan_end, AM2_ATT)) < 0) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s  %s %d ","Cannot get Satellite Attitude Data","AMTK status : ", ret);
    err_code = 53;
    error_end(err_code, buf);
  }

  /* 2010/01 RESTEC */
  /*** Read Position in Orbit Data ***/
  ptr = PosOrbit;
  if((ret = AMTK_get_SwathDouble(file_id, (double**)&ptr, scan_start, scan_end, AM2_POS_ORBIT)) < 0) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s  %s %d ","Cannot get Position in Orbit Data","AMTK status : ", ret);
    err_code = 53;
    error_end(err_code, buf);
  }


/*||||||||||||||||||||||||||||||||||||||||||||||*/		/*DO NOT USE*/
/*vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
    /*** Read in the surface temperatures. ***/
#ifdef ADDITIONAL_SNOW_LOGIC
  get_surftemp();
# endif
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*||||||||||||||||||||||||||||||||||||||||||||||*/

  /*** Compute the SNOW for each pixel of the granule. ***/

  set_dt_str_utc(cts);
  if (debug_main ==1) fprintf(stderr, "%s %s\n",cts, "*** calculate product");


  for( line=0; line<iobsline; line++ ) {
    for( samp=0; samp<iobscan; samp++ ) {

    retr_flag=42;   /*Used to determine kind of retrieval: set default as 42 (if system screens rule out a retrieval) */

/*initialise ProcData and ProcDataQ with 0 */
      ProcDATA[0][line][samp] = 0;
      ProcDATA[1][line][samp] = 0;
      ProcDataQ[0][line][samp][0] = 0;


/* CHECK FOR DYNAMIC AMSR2 SYSTEM ERROR FLAGS (UPDATED JANUARY 2015) */

      /***  (FLAG_BAD_TB) Check sample for bad data. ***/
      if( (v36 = L1rtb[V36][line][samp]) >= ABNORMAL_TB  ||
	  (h36 = L1rtb[H36][line][samp]) >= ABNORMAL_TB  ||
	  (v18 = L1rtb[V18][line][samp]) >= ABNORMAL_TB  ||
	  (h18 = L1rtb[H18][line][samp]) >= ABNORMAL_TB  ||
	  (v10 = L1rtb[V10][line][samp]) >= ABNORMAL_TB  ||
	  (h10 = L1rtb[H10][line][samp]) >= ABNORMAL_TB  ||
          (v23 = L1rtb[V23][line][samp]) >= ABNORMAL_TB  ||
	  (h23 = L1rtb[H23][line][samp]) >= ABNORMAL_TB ) {
        ProcDATA[0][line][samp] = NO_OBSERVATION;
        ProcDATA[1][line][samp] = NO_OBSERVATION;
        ProcDataQ[0][line][samp][0] = FLAG_BAD_TB;
	/*continue;*/
        goto Loopout;
      }
     /*** 2005 Oct.12 RESTEC : bL1bbt89 -> bL1bbt[12] or bL1bbt[13] ***/
      if(L1rtb[V89][line][samp] >= ABNORMAL_TB ||
          L1rtb[H89][line][samp] >= ABNORMAL_TB ) {
        ProcDATA[0][line][samp] = NO_OBSERVATION;
        ProcDATA[1][line][samp] = NO_OBSERVATION;
        ProcDataQ[0][line][samp][0] = FLAG_BAD_TB;
        /*continue;*/
        goto Loopout;
      }
      v89 = L1rtb[V89][line][samp];   /*----- MOVED in JANUARY 2015 ----*/
      h89 = L1rtb[H89][line][samp];   /*----- MOVED in JANUARY 2015 ----*/

      snow = 0;

      /*** 2003 Nov.10 RESTEC : check SatAtt-value(roll) ***/
      value_SatAtt = 0.0;
      value_SatAtt = SatAtt[line][0];

      /*** |AMSR2 SatAtt| < 0.01 ***/
      if((value_SatAtt >= limit_v) || (value_SatAtt <= ((-1)*limit_v)))
      {
          ProcDATA[0][line][jp] = NO_OBSERVATION;
          ProcDATA[1][line][jp] = NO_OBSERVATION;
          ProcDataQ[0][line][jp][0] = FLAG_SAT_ATT;
          goto Loopout;
     }
      /*** 2003 Nov.10 RESTEC : check SatAtt-value :end ***/

      /***(FLAG_SAT_ATT) CHECK SATELLITE GEOLOCATION DATA*********/
      if((LatLon[line][samp].lon < -180.) || (LatLon[line][samp].lon > 180.)
         || (LatLon[line][samp].lat < -90.) || (LatLon[line][samp].lat > 90.))
      {
        ProcDataQ[0][line][samp][0] = FLAG_OUTOFRANGE;
        ProcDATA[0][line][samp] = NO_OBSERVATION;
        ProcDATA[1][line][samp] = NO_OBSERVATION;
        goto Loopout;
      }

      /*** (FLAG_OUTOFRANGE) Check Out of Range flag ***/
      if( v36<50 || v36>320 || h36<50 || h36>320 ||
	  v18<50 || v18>320 || v23<50 || v23>320 || 
	  v23<50 || v23>320 || h23<50 || h23>320 ||
	  v89<50 || v89>320 || v89<50 || v89>320 ) {	
	ProcDataQ[0][line][samp][0] = FLAG_OUTOFRANGE;
	ProcDATA[0][line][samp] = NO_OBSERVATION;
        ProcDATA[1][line][samp] = NO_OBSERVATION;
	goto Loopout;		/*** Go to bottom loop -- there is no snow. ***/
      }
          

/*** Get flags array indices from latitude/longitude of this sample.
	   Convert longitudes from -180_+180 to 0_360. ***/
      longitude = LatLon[line][samp].lon;
      latitude = LatLon[line][samp].lat;
      col = (longitude>=0.0) ? longitude*10 : 3600.0+longitude*10;              /*2013 April UW MODIFY*/
      row = ( 90.0 - LatLon[line][samp].lat ) * 10.;


/* Get EASE2.0 GRID COORDINATES EASEX,EASEY */	/*----  JANUARY 2016 ----*/
      zeasex=longitude;
      zeasey=latitude;
      lltoease2(&zeasey, &zeasex);
      easex=zeasex;
      easey=zeasey;
      if (latitude < 0){
        easey=zeasey+1440;
//        printf("Lat : Lon : easex : easey %f %f %f %f \n",latitude, longitude,zeasex,zeasey);
      }
/* (FLAG_PERMANENT_ICE) CHECK FOR STATIC GEOPHYSICAL FLAGS */
      /*** Check for permanent ice. */
      if( (flags[easey][easex] & ICE) ) {
	ProcDataQ[0][line][samp][0] = FLAG_PERMANENT_ICE;
	ProcDATA[0][line][samp] = NO_SNOW;
        ProcDATA[1][line][samp] = NO_SNOW;
	goto Loopout;		/*** Go to bottom loop -- there is no snow. ***/
      }


/*------ Set up detection variables ------*/

        ft=transmissivity[easey][easex]/100.;		/*----- JANUARY 2015 -----*/ 
        elev=elevation[easey][easex]; 		        /*----- JANUARY 2015 -----*/
        wf=(WaterFraction[easey][easex]); 		/*----- JANUARY 2016 -----*/

/* FOREST CORRECTION dTb >= 0 USE THE FULL RANGE OF 0<Trans<100% */
	v10f=(ft>0.0 && ft < 5.583/6.787)   ? v10-((-6.787*ft)+5.583) : v10;		 /*----- April 2016 -----*/
	h10f=(ft>0.0 && ft < 22.414/24.621) ? h10-((-24.621*ft)+22.414) : h10;		 /*----- April 2016 -----*/
	v18f=(ft>0.0 && ft < 7.425/8.761)   ? v18-((-8.761*ft)+7.425): v18;		 /*----- April 2016 -----*/
	h18f=(ft>0.0 && ft < 25.219/27.836) ? h18-((-27.836*ft)+25.219) : h18;		 /*----- April 2016 -----*/
	v23f=(ft>0.0 && ft < 8.467/9.849)   ? v23-((-9.849*ft)+8.467) : v23;		 /*----- April 2016 -----*/
	h23f=(ft>0.0 && ft < 25.404/27.988) ? h23-((-27.988*ft)+25.404) : h23;		 /*----- April 2016 -----*/
	v36f=(ft>0.0 && ft < 12.412/13.468) ? v36-((-13.468*ft)+12.412) : v36;		 /*----- April 2016 -----*/
	h36f=(ft>0.0 && ft < 28.79/30.848)  ? h36-((-30.848*ft)+28.79) : h36;		 /*----- April 2016 -----*/
	v89f=(ft>0.0 && ft < 32.577/33.051) ? v89-((-33.051*ft)+32.577) : v89;		 /*----- April 2016 -----*/
	h89f=(ft>0.0 && ft < 43.733/45.204) ? h89-((-45.204*ft)+43.733) : h89;		 /*----- April 2016 -----*/

/* Estimate surface temperature */
        klvn=62.4-0.05*v18+0.84*v23-0.40*h36+0.41*v89;	 /*----- JANUARY 2015 (updated)-----*/
/*--------------------------------*/
/*** ACCUMULATE TPHYS DATA****/
/*--------------------------------*/
        dysta_tphys[easey][easex][0]=dysta_tphys[easey][easex][0]+klvn;
        dysta_tphys[easey][easex][1]=dysta_tphys[easey][easex][1]+1;
	t_est[row][col][0]=dysta_tphys[row][col][0]/dysta_tphys[row][col][1];

/* Set up ratios/differences for detection/retrievals */
      t2389hf=h23f-h89f;
      t3689vf=v36f-v89f;
      t1836vf=v18f-v36f;
      t1836hf=h18f-h36f;
      t18h23vf=h18f-v23f;
      t89v36hf=v89f-h36f;

      pol18f=v18f-h18f;
      pol36f=v36f-h36f;
      pol89f=v89f-h89f;
      if (pol18f < 3.) pol18f=3.;
      if (pol36f < 3.) pol36f=3.;
/*----------------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------------*/
/* Get WATER FRACTION Data */
      xlon = LatLon[line][samp].lon;
      ylat = LatLon[line][samp].lat;

      xlon=(xlon+180.)*120.;
      ylat=(90.-ylat)*120.;
      llfcol=xlon;
      llfrow=ylat;

/*----------------------------------------------------------------------------------*/

      /*** (FLAG_OCEAN | FLAG_LAKE | FLAG_LAKE_ICE) Check for water. NEW / IMPROVED FLAGS  JANUARY 2015***/
      if( ! (flags[easey][easex] & LAND) ) {
	ProcDataQ[0][line][samp][0] = FLAG_OCEAN;
	ProcDATA[0][line][samp] = -32768.0;
        ProcDATA[1][line][samp] = -32768.0;
	if (wf < 101 && t3689vf > 5.0) {
	    ProcDataQ[0][line][samp][0] = FLAG_LAKE_ICE;
	    ProcDATA[0][line][samp] = -32768.0;
            ProcDATA[1][line][samp] = -32768.0;
 	}
	if (wf < 101 && t3689vf <= 5.0) {
	    ProcDataQ[0][line][samp][0] = FLAG_LAKE;
	    ProcDATA[0][line][samp] = -32768.0;
            ProcDATA[1][line][samp] = -32768.0;
	}
	goto Loopout;		/*** Go to bottom loop -- there is no snow. ***/
      }
      /*** Check Frozen Lake flag ***/

      /*** (FLAG_SNOW_IMPOSSIBLE) Check snow impossible flag ***/
      if( (flags[easey][easex] & SNOW_IMPOSSIBLE) ) {
	ProcDataQ[0][line][samp][0] = FLAG_SNOW_IMPOSSIBLE;
	ProcDATA[0][line][samp] = NO_SNOW;
        ProcDATA[1][line][samp] = NO_SNOW;
	goto Loopout;		/*** Go to bottom loop -- there is no snow. ***/
      }

/* IF THE CODE HAS MADE IT HERE THEN SNOW IS POSSIBLE*/
      ProcDataQ[0][line][samp][0] = FLAG_SNOW_POSSIBLE;
      tmpflag=0;
      retr_flag=41;   /*Used to determine kind of retrieval, if any: 7 if there is land but no snow*/

/*------ SET DYNAMIC GEOPHYSICAL FLAGS ------*/

/* (FLAG_SNOW_HIGHLAT) Cold Snow (mostly at high latitude) - all elevation NEW FLAG*/
      if (tmpflag == 0 && v18f-h18f > 20. && v23f-h23f > 15. && v36f-h36f > 15 && v89f-h89f > 15 && v18f-v36f > 9 && klvn <= 270.){
        ProcDataQ[0][line][samp][0] = FLAG_SNOW_HIGHLAT;
	tmpflag=FLAG_SNOW_HIGHLAT;
        retr_flag=3;
        goto Loopout;		/*** Go to bottom loop. ***/
       }
/* (FLAG_TP_NOSNOW)High elevation false snow (frozen ground) Tsutsui and Koike, 2012 NEW FLAG*/
      tp_ratio=(v36f/v89f)/(v18f/v36f);
      if (tmpflag == 0 && elev > 2500 && tp_ratio < 1.03 && 
	latitude > 27. && latitude < 40. && longitude > 70. && longitude < 105.){
	tmpflag=FLAG_TP_NOSNOW;
        retr_flag=43;
	ProcDataQ[0][line][samp][0] = FLAG_TP_NOSNOW;
	ProcDATA[0][line][samp] = NO_SNOW;
	ProcDATA[1][line][samp] = NO_SNOW;
	goto Loopout;		/*** Go to bottom loop -- there is no snow. ***/
      }


/*(FLAG_DRY_SNOW) Snow on land (low/no lake fraction ) NEW FLAG*/
/* I. MODERATE SNOW (V-POL):  v18-v36 > 0 & v36 < 257 & v18 < 260 */ 
     if (tmpflag == 0 && v18f <= 260.00 && v36f <=  257.00 && t1836vf > 0.) {
       if (pol89f > 20. || t1836hf < 2 || klvn > 273.15)  { 		/* changed kelvin from 270 to 273 and removed   "t89v36hf > 20. ||"  23 Jul 2016 */
         if (t1836hf <= 0 || pol36f >= pol18f || klvn > 270.) { 
          tmpflag=FLAG_NORMAL_NOSNOW;
          ProcDataQ[0][line][samp][0] = FLAG_NORMAL_NOSNOW;
	  ProcDATA[0][line][samp] = NO_SNOW;
	  ProcDATA[1][line][samp] = NO_SNOW;
	  goto Loopout;		/*** Go to bottom loop -- there is no snow. ***/
         }
         else 
         {
	  tmpflag=FLAG_DRY_SNOW;
          ProcDataQ[0][line][samp][0] = FLAG_DRY_SNOW;
	  retr_flag=1;
          goto Loopout;		/*** Go to bottom loop. ***/
         }
       }
       else {
        tmpflag=FLAG_DRY_SNOW;
        ProcDataQ[0][line][samp][0] = FLAG_DRY_SNOW;
        retr_flag=1;
        goto Loopout;		/*** Go to bottom loop. ***/
       }
     }
/*;(FLAG_DRY_SNOW) MODERATE SNOW (H-POL): IF h18-h36 > 0 & H36 < 248.4 & h18 < 251 NEW FLAG*/
     if (tmpflag == 0 && h18f <=  251.00 && h36f <= 248.4 && t1836hf > 0.) {
       if (pol89f > 20. || t18h23vf > 25. || klvn > 273.15) {		/* changed kelvin from 270 to 273 and removed   "t89v36hf > 20. ||"  23 Jul 2016 */
 	 tmpflag=FLAG_NORMAL_NOSNOW;
         ProcDataQ[0][line][samp][0] = FLAG_NORMAL_NOSNOW;
	 ProcDATA[0][line][samp] = NO_SNOW;
	 ProcDATA[1][line][samp] = NO_SNOW;
	 goto Loopout;		/*** Go to bottom loop -- there is no snow. ***/
       }
       else {
   	 tmpflag=FLAG_DRY_SNOW;
         ProcDataQ[0][line][samp][0] = FLAG_DRY_SNOW;
 	 retr_flag=2;
         goto Loopout;		/*** Go to bottom loop. ***/
       }
     }
/*(FLAG_SHALLOW_SNOW) SHALLOW SNOW (H) (146) IF h23-h89 > 0 and h89 <211 THEN shallow snow NEW FLAG*/
     if (tmpflag == 0 && t2389hf > 0. && pol89f < 5. && v89f < 188.0) {
       tmpflag=FLAG_SHALLOW_SNOW;
       ProcDataQ[0][line][samp][0] = FLAG_SHALLOW_SNOW;
       retr_flag=4;
       goto Loopout;		/*** Go to bottom loop. ***/
     }

/* (FLAG_WET_SNOW) Wet Snow on land: IF v18-h18 > 10 & v18-v36 >=0 [Walker and Goodison (1993) Ann. Glac. 17:307-311.]  UPDATED FLAG*/
     if (tmpflag == 0 &&  pol18f > 10. && t1836vf == 0) {
       tmpflag=FLAG_WET_SNOW;
       ProcDataQ[0][line][samp][0] = FLAG_WET_SNOW;
       ProcDATA[0][line][samp] = NO_SNOW;
       ProcDATA[1][line][samp] = NO_SNOW;
       retr_flag=5;
       goto Loopout;		/*** Go to bottom loop -- Wet snow. ***/
     }

/* (FLAG_NORMAL_NOSNOW) If no snow detected flag land no snow */
     if (tmpflag == 0 ) {
       ProcDataQ[0][line][samp][0] = FLAG_NORMAL_NOSNOW;
       ProcDATA[0][line][samp] = NO_SNOW;
       ProcDATA[1][line][samp] = NO_SNOW;
       goto Loopout;		/*** Go to bottom loop -- there is no snow. ***/
     } 

/* Set up variables for Retrieval */
     snow=0.0;

     t1036v=v10-v36;
     t1036h=h10-h36;
     t1018v=v10-v18;  
     t1018h=h10-h18;
     t1836v=v18-v36;
     t1836h=h18-h36;
     t2389v=v23-v89;
     t2389h=h23-h89;

/* THIS IS THE LANDING POINT FROM ALL SCREENS */
     Loopout:

/* FINAL PERSISTENCE CHECK ON NO-SNOW PIXELS */

/* (FLAG_DRY_SNOW - PERSISTENCE) If not dry snow test, and not wet snow on land: but yesterday there was snow, and either tb18-36 V or H > 0, use 10 day persistence argument UPDATED FLAG*/
/* NB sdhistory[easey][easex][0] is yesterday */
     if (tmpflag == 0 || tmpflag == FLAG_NORMAL_NOSNOW) {
       if (sdhistory[easey][easex][0] > 0 && persistence[easey][easex][1] <= plimit && persistence[easey][easex][1] > 0) {
         if (t1836vf >= 0.0 || t1836hf >= 0.0 ){
           tmpflag=FLAG_DRY_SNOW_PERSIST;
           ProcDataQ[0][line][samp][0] = FLAG_DRY_SNOW_PERSIST;
           retr_flag=6;
           goto Loopout;		/*** Go to bottom loop -- PERSISTENT SNOW. ***/
         }
       }
     }

/* The following are internal flags */
/* retr_flag 42: System and Geophysical Screens Rule out Retrieval */
/* retr_flag 41: Pass to Retrieval: land no snow */
/* retr_flag  1: Pass to Retrieval: snow (V-Pol) */
/* retr_flag  2: Pass to Retrieval: snow (H-Pol) */
/* retr_flag  3: Pass to Retrieval: snow high latitude*/
/* retr_flag  4: Pass to Retrieval: shallow snow */
/* retr_flag  5: Pass to Retrieval: Wet snow */
/* retr_flag  6: Pass to Retrieval: Persistent snow (no retrieval but use previous day)*/
/* retf is decimal julian day */

/* If snow is detected, increment persistence flag tracker */
     if (retr_flag > 0 && retr_flag < 6) persistence[easey][easex][0]=persistence[easey][easex][0]+1;

     if (debug_main ==1) printf("Persistence flag:    %2d %2d %2d %2d \n",persistence[easey][easex][0],persistence[easey][easex][1],retr_flag,tmpflag);
     if (debug_main ==1) printf("Granule Flag ...     %2d : %5.3f : %6d \n",retr_flag,klvn,sdhistory[easey][easex][0]);
     if (debug_main ==1) printf("Granule Flag Tbs...  %5.1f %5.1f %5.1f %5.1f %5.1f %5.1f %5.1f %5.1f %5.2f \n",v18f, h18f,v23f, h23f, v36f, h36f, v89f,h89f,t1836vf);

/*--------------------------------------------------*/
/*       DO THE SWE/SD RETRIEVAL                    */
     if (retr_flag < 42 ) retrieve_swe_sd( &easex, &easey, &retf, &retr_flag, &EstSD, &EstSWE);
/*--------------------------------------------------*/

/*  ASSIGN Retrieved SD to FloatSnow and Output ProcDATA/ProcDATAQ arrays */
     if (retr_flag >= 1 && retr_flag <= 6) snow = EstSD;
     if (snow > 0.) {
       snowct=snowct+1;
       ProcDATA[0][line][samp] = snow <0 ? NO_SNOW : snow;
       ProcDATA[1][line][samp] = EstSWE <0 ? NO_SNOW : EstSWE;
     }
     ProcDataQ[1][line][samp][0] = ProcDataQ[0][line][samp][0];

    }		/* End: for( sample...   */ 
  }		/* End: for( line...     */

  AMTK_closeH5(file_id);
  if (debug_main ==1)   fprintf(stderr, "%s %s %f\n",cts, "Total # Snow Pixels",snowct);

/*---------------------------------------------------------------*/
/*---------------------------------------------------------------*/
/*----------JANUARY 2016 WRITE AND UPDATE DYNAMIC FILES----------*/

  if( WriteDynamicOutputs() != 0 )
    return(-1);
  if (debug_main ==1)   fprintf(stderr, "%s %s\n\n",cts, "Finished updating Dynamic history files");

/*---------------------------------------------------------------*/
/*---------------------------------------------------------------*/

  /* calculate science quality flag */
  for(i=0; i<iobsline; i++) {
    for(j=0; j<iobscan; j++) {
      if(ProcDataQ[0][i][j][0]== FLAG_SNOW_POSSIBLE) psb++;
      if(ProcDataQ[0][i][j][0]== FLAG_SNOW_IMPOSSIBLE) ipsb++;
      if(ProcDataQ[0][i][j][0]== FLAG_OCEAN) ocean++;		
      if(ProcDataQ[0][i][j][0]== FLAG_LAKE) lakew++;		/*ADJUSTED JANUARY 2015 */
      if(ProcDataQ[0][i][j][0]== FLAG_LAKE_ICE) lakei++;		/*ADJUSTED JANUARY 2015 */
      if(ProcDataQ[0][i][j][0]== FLAG_PERMANENT_ICE) ice++;
    }
  }
  sqf = psb/(iobsline*243.-ocean-ice-ipsb-lakew-lakei)*100+0.5;

  /*** L2 temporary output ***/
  set_dt_str_utc(cts);
//fprintf(stderr, "%s %s\n",cts, "*** L2 Temporary output");

  /* create L2 temporary file */
  file_id = AMTK_openH5_Write(L2_f, AM2_CREATE_MODE);
  if(file_id < 0){
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s %s  %s %d ", "Cannot open",L2_f,"AMTK status : ", file_id);
    err_code = 81;
    error_end(err_code, buf);
  }

  /* output meta data (InputFileName)*/
  ptr = strrchr(L1R_f, '/')+1;
  strcpy(buf, ptr);
  ret = AMTK_setMetaDataName(file_id, "InputFileName", buf);
  if(ret < 0){
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s %d ","Cannot write the MetaData(InputFileName)  AMTK status : ", ret);
    err_code = 81;
    error_end(err_code, buf);
  }

  /* output meta data (Science Quality Flag)*/
  sprintf(buf, "%d[%]",sqf);
  ret = AMTK_setMetaDataName(file_id, "ScienceQualityFlag", buf);
  if(ret < 0){
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s %d ","Cannot write the MetaData(ScienceQualityFlag)  AMTK status : ", ret);
    err_code = 81;
    error_end(err_code, buf);
  }

  /* output meta data (Science Quality Flag Explanation)*/
  sprintf(buf, "Science Quality Flag [%] = (number of successfully retrieved pixels) / (number of targeted pixels) * 100.0");
  ret = AMTK_setMetaDataName(file_id, "ScienceQualityFlagExplanation", buf);
  if(ret < 0){
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s %d ","Cannot write the MetaData(ScienceQualityFlag)  AMTK status : ", ret);
    err_code = 81;
    error_end(err_code, buf);
  }

  /* output meta data (Overlap Scans) */
  ret = AMTK_setMetaDataName(file_id, "OverlapScans", "0");
  if(ret < 0){
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s %d ","Cannot write the MetaData(OverlapScans)  AMTK status : ", ret);
    err_code = 81;
    error_end(err_code, buf);
  }

  /* output meta data (Geophysical Name)*/
  ret = AMTK_setMetaDataName(file_id, "GeophysicalName", "Snow Depth");
  if(ret < 0){
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s %d ","Cannot write the MetaData(GeophysicalName)  AMTK status : ", ret);
    err_code = 81;
    error_end(err_code, buf);
  }

#ifdef ADDITIONAL_SNOW_LOGIC

#else
  /* output meta data (Global Meterological Data Type)*/
  ret = AMTK_setMetaDataName(file_id, "GlobalMeteorologicalDataType", "None");
  if(ret < 0){
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s %d ","Cannot write the MetaData(GlobalMeteorologicalDataType)  AMTK status : ", ret);
    err_code = 81;
    error_end(err_code, buf);
  }

  /* output meta data (Ancillary Data Information) */
  ret = AMTK_setMetaDataName(file_id, "AncillaryDataInformation", "");
  if(ret < 0){
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s %d ","Cannot write the MetaData(AncillaryDataInformation)  AMTK status : ", ret);
    err_code = 81;
    error_end(err_code, buf);
  }
#endif

  /* output dataset (Scan Time)*/
  dim_size[0] = iobsline;
  dim_size[1] = 0;
  dim_size[2] = 0;
  ret = AMTK_setDimSize(file_id, AM2_SCAN_TIME_DEF, dim_size);
  if(ret < 0){
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s %d ","Cannot create the dataset(Scan Time)  AMTK status : ", ret);
    err_code = 81;
    error_end(err_code, buf);
  }

  ret = AMTK_setScanTime(file_id, ObsTime, 1, iobsline);
  if(ret < 0){
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s %d ","Cannot write the Scan Time data  AMTK status : ", ret);
    err_code = 81;
    error_end(err_code, buf);
  }

  /* output dataset (Position in Orbit) */
  dim_size[0] = iobsline;
  dim_size[1] = 0;
  dim_size[2] = 0;
  ret = AMTK_setDimSize(file_id, AM2_POS_ORBIT, dim_size);
  if(ret < 0){
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s %d ","Cannot create the dataset(Posion in Orbit)  AMTK status : ", ret);
    err_code = 81;
    error_end(err_code, buf);
  }

  ret = AMTK_set_SwathDouble(file_id, PosOrbit, 1, iobsline, AM2_POS_ORBIT);
  if(ret < 0){
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s %d ","Cannot write the Posion in Orbit  AMTK status : ", ret);
    err_code = 81;
    error_end(err_code, buf);
  }

  /* output dataset (Geophysical Data) */
  dim_size[0] = iobsline;
  dim_size[1] = 243;
  dim_size[2] = 3;
  ret = AMTK_setDimSize(file_id, AM2_SWATH_GEO1, dim_size);
  if(ret < 0){
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s %d ","Cannot create the dataset (Geophysical Data)  AMTK status : ", ret);
    err_code = 81;
    error_end(err_code, buf);
  }
  /* write SND */
  ret = AMTK_set_SwathFloat(file_id, &ProcDATA[0][0][0], 1, iobsline, AM2_SWATH_GEO1);
  if(ret < 0){
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s %d ","Cannot write the SND data  AMTK status : ", ret);
    err_code = 81;
    error_end(err_code, buf);
  }

  /* write SWE */
  ret = AMTK_set_SwathFloat(file_id, &ProcDATA[1][0][0], 1, iobsline, AM2_SWATH_GEO2);
  if(ret < 0){
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s %d ","Cannot write the SWE data  AMTK status : ", ret);
    err_code = 81;
    error_end(err_code, buf);
  }

  /* output dataset (Lat/Lon) */
  dim_size[0] = iobsline;
  dim_size[1] = 243;
  dim_size[2] = 0;
  ret = AMTK_setDimSize(file_id, AM2_LATLON_L2_LO, dim_size);
  if(ret < 0){
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s %d ","Cannot create the dataset(Lat/Lon Data)  AMTK status : ", ret);
    err_code = 81;
    error_end(err_code, buf);
  }

  ret = AMTK_setLatLon(file_id, (AM2_COMMON_LATLON *)LatLon, 1, iobsline, AM2_LATLON_L2_LO);
  if(ret < 0){
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s %d ","Cannot write the Lat/Lon data  AMTK status : ", ret);
    err_code = 81;
    error_end(err_code, buf);
  }

  /* output dataset (Pixel Data Quality) */
  for(i=0; i<iobsline; i++) {
    for(j=0; j<iobscan; j++) {
      quality[i][j] = ProcDataQ[0][i][j][0];
      quality[i+iobsline][j] = ProcDataQ[1][i][j][0];
    }
  }

  dim_size[0] = 2;
  dim_size[1] = iobsline;
  dim_size[2] = 243;
  ret = AMTK_setDimSize(file_id, AM2_PIX_QUAL, dim_size);
  if(ret < 0){
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s %d ","Cannot create the dataset(Pixel Data Quality)  AMTK status : ", ret);
    err_code = 81;
    error_end(err_code, buf);
  }

  ret = AMTK_set_SwathUChar(file_id, (unsigned char*)quality, 1, iobsline, AM2_PIX_QUAL);
  if(ret < 0) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s %d ","Cannot write the Pixel Data Quality  AMTK status : ", ret);
    err_code = 81;
    error_end(err_code, buf);
  }

  /* close L2 temporary file */
  ret = AMTK_closeH5_Write(file_id);
  if(ret < 0) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s %d ","Cannot close output file  AMTK status : ", ret);
    err_code = 81;
    error_end(err_code, buf);
  }

  set_dt_str_utc(cts);
//////  fprintf(stderr, "%s %s\n",cts, "*** end SND algorithm program");
  return(0);
}

#ifdef ADDITIONAL_SNOW_LOGIC
void get_surftemp()
{
  int i;
  void *ptr;
  float ga_stemp[3][361][720];
  int dim_size[3];
  float resolution;
  float temp_data[3*243*2100];
  int ret;
  char buf[256], cts[24];;
  int err_code;
  AM2_COMMON_SCANTIME get_time;

  set_dt_str_utc(cts);
  fprintf(stderr, "%s %s\n",cts, "*** read GANAL");


  if((ret = GAopen((unsigned char*)Ga_file)) < 0) {
    memset(buf, '\0', sizeof(buf));
    sprintf(buf, "%s %d ","AMTK_GA status : ", ret);
    err_code = 56;
    error_end(err_code, buf);
  }

  for(i=0; i<GA_MAX; i++) {
    if(strcmp((char*)&Ga_file[i], "9999") == 0) {
      continue;
    }
    ptr = strrchr((const char*)Ga_file[i], '/');
    sscanf((char*)ptr+1,"GANAL_%04d%02d%02d%02d.grb2",&get_time.year,
                                                              &get_time.month,
                                                              &get_time.day,
                                                              &get_time.hour);
    get_time.minute = 0;
    get_time.second = 0;
    get_time.ms = 0;
    ptr = &ga_stemp[i][0][0];
    GAgetSurf2(&get_time,GA_NEAR_TIME,GA_S_TMP,(float**)&ptr) < 0;
    if(ret < 0) {
      memset(buf, '\0', sizeof(buf));
      sprintf(buf, "%s %d ","AMTK_GA status : ", ret);
      err_code = 56;
      error_end(err_code, buf);
    }
  }

  dim_size[0] = iobsline;
  dim_size[1] = iobscan;
  dim_size[2] = 0;
  resolution = 0.5;
  ret = gm_ga_resampling_((float*)ga_stemp, (char(*)[])Ga_file, dim_size, ObsTime,
        (AM2_COMMON_LATLON *)LatLon, &resolution, temp_data, (float*)SurfATemp);
  if(ret < 0) {
      memset(buf, '\0', sizeof(buf));
      sprintf(buf, "%s %d ","AMTK_GA status : ", ret);
      err_code = 56;
      error_end(err_code, buf);
  }

}
#endif
