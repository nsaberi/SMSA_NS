#ifdef __GLOBAL__

#define Extern

#else

#define Extern extern

#endif


Extern float L1rtb[14][8400][243];     /* Brightness temperature of L1R AMSR except 89GHz */
Extern float L1rtb89[4][8400][486];    /* Brightness temperature of L1R AMSR for 89GHz */
Extern float SurfATemp[8400][243];      /* Surface air temperature */
Extern float ProcDATA[3][8400][243];    /* Processed data */
Extern char  ProcDataQ[3][8400][243][8];/* Quality data of processed data */
Extern float SatAtt[8400][3];           /* Satellite Attitude Data */
Extern AM2_COMMON_LATLON LatLon[9400][243]; /* Observation point Lat./Long. of 89GHz-A(0,2,4...) */
Extern AM2_COMMON_SCANTIME ObsTime[8400];   /* Observation Time */

Extern int iobsline; /* observation line number */
Extern int iobscan;  /* observation scan number (243) */

Extern char L1R_f[512];
Extern char L2_f[512];
Extern unsigned char Ga_file[5][512];
Extern char Mgdsst[512]; 
Extern int ProcKind; /* 1:Standard 2:Nera Real Time */
Extern double PosOrbit[8400];
