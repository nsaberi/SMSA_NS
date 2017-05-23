/*   This routine writes out the  dynamic arrays at the end of the program.   */
/* CREATED JANUARY 2016 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define ROWS_E2	2880 
#define COLS_E2	1440
#define FAIL (-1)
#define SUCCESS	0

#define DYNAMIC_t1836v_smth "InternalDynamicParameters/T1836V_gaussian6.dat"
#define DYNAMIC_t1836h_smth "InternalDynamicParameters/T1836H_gaussian6.dat"
#define DYNAMIC_SCAhistory "InternalDynamicParameters/SCAhistory.dat"
#define DYNAMIC_SDhistoryInst "InternalDynamicParameters/SDhistoryInst.dat"
#define DYNAMIC_T_est "InternalDynamicParameters/T_est.dat"
#define DYNAMIC_regime "InternalDynamicParameters/regime.dat"
#define DYNAMIC_fet "InternalDynamicParameters/fet.dat"
#define DYNAMIC_evolution "InternalDynamicParameters/evolution.dat"
#define DYNAMIC_gs "InternalDynamicParameters/gs.dat"
#define DYNAMIC_den "InternalDynamicParameters/density.dat"
#define DYNAMIC_thistory "InternalDynamicParameters/thistory.dat"
#define DYNAMIC_dysta_tphys "InternalDynamicParameters/dysta_tphys.dat"
#define DYNAMIC_dysta_tphys "InternalDynamicParameters/dysta_tphys.dat"
#define DYNAMIC_dysta_fet "InternalDynamicParameters/dysta_fet.dat"
#define DYNAMIC_dysta_gs "InternalDynamicParameters/dysta_gs.dat"
#define DYNAMIC_dysta_den "InternalDynamicParameters/dysta_den.dat"
#define DYNAMIC_dysta_evol "InternalDynamicParameters/dysta_evol.dat"
#define DYNAMIC_dysta_reg "InternalDynamicParameters/dysta_reg.dat"
#define DYNAMIC_dysta_swe "InternalDynamicParameters/dysta_swe.dat"
#define DYNAMIC_dysta_sd "InternalDynamicParameters/dysta_sd.dat"
#define DYNAMIC_dysta_t1836v "InternalDynamicParameters/dysta_t1836v.dat"
#define DYNAMIC_dysta_t1836h "InternalDynamicParameters/dysta_t1836h.dat"
#define DYNAMIC_persistence "InternalDynamicParameters/persistence.dat"


/*** Globals ***/
char		NoDynamicInputFlag;
extern	short int sdhistory[ROWS_E2][COLS_E2][10],tb1836hf_smth[ROWS_E2][COLS_E2][10],tb1836vf_smth[ROWS_E2][COLS_E2][10],persistence[ROWS_E2][COLS_E2][2];
extern	short int dayswe[ROWS_E2][COLS_E2],daysd[ROWS_E2][COLS_E2];
extern	float	t_est[ROWS_E2][COLS_E2][11],gs[ROWS_E2][COLS_E2],fet[ROWS_E2][COLS_E2],denest[ROWS_E2][COLS_E2];
extern float	dysta_tphys[ROWS_E2][COLS_E2][2],dysta_fet[ROWS_E2][COLS_E2][2],dysta_gs[ROWS_E2][COLS_E2][2],dysta_den[ROWS_E2][COLS_E2][2];
extern int	dysta_evol[ROWS_E2][COLS_E2];
extern int	dysta_reg[ROWS_E2][COLS_E2][3];
extern float	dysta_sd[ROWS_E2][COLS_E2][2], dysta_swe[ROWS_E2][COLS_E2][2];
extern float	dysta_1836v[ROWS_E2][COLS_E2][2],dysta_1836h[ROWS_E2][COLS_E2][2];
extern int	regime[ROWS_E2][COLS_E2],evolution[ROWS_E2][COLS_E2];
extern double	thistory[ROWS_E2][COLS_E2][11],scahistory[ROWS_E2][COLS_E2][11];
extern char	debug_retr,debug_read,debug_write;
extern int	outswitch,outswitchtmp;
extern char	sJulian[8];

/*====================================================================================*/

int WriteDynamicOutputs()
{
   int	handle;
    char place1[2],gsfname[50]="InternalDynamicParameters/gs/";
    char place2[2],denfname[50]="InternalDynamicParameters/density/";
    char swefname[50]="InternalDynamicParameters/snow/";
    char sdfname[50]="InternalDynamicParameters/snow/";

/*--------------------------------------------------------*/
/*++++Daily Tb18-36 VVVV-Pol Stats File (1 days - sum and total) ++++*/
/*--------------------------------------------------------*/
   if( (handle=open( DYNAMIC_dysta_t1836v, O_WRONLY | O_CREAT,0644 )) == -1 ) {
      perror("Error creating or opening file InternalDynamicParameters/dysta_1836v.dat.");
   }
   if(write(handle,(char *)dysta_1836v[0],sizeof(dysta_1836v)) != sizeof(dysta_1836v) ) {
      perror("Error writing to file InternalDynamicParameters/dysta_1836v.dat");
   }
   close( handle );
/*--------------------------------------------------------*/
/*++++Daily Tb18-36 HHHH-Pol Stats File (1 days - sum and total) ++++*/
/*--------------------------------------------------------*/
   if( (handle=open( DYNAMIC_dysta_t1836h, O_WRONLY | O_CREAT,0644 )) == -1 ) {
      perror("Error creating or opening file InternalDynamicParameters/dysta_1836h.dat.");
   }
   if(write(handle,(char *)dysta_1836h[0],sizeof(dysta_1836h)) != sizeof(dysta_1836h) ) {
      perror("Error writing to file InternalDynamicParameters/dysta_1836h.dat");
   }
   close( handle );
/*--------------------------------------------------------*/
/*++++Daily TPhys Stats File (1 days - sum and total) ++++*/
/*--------------------------------------------------------*/
   if( (handle=open( DYNAMIC_dysta_tphys, O_WRONLY | O_CREAT,0644 )) == -1 ) {
      perror("Error creating or opening file InternalDynamicParameters/dysta_tphys.dat.");
   }
   if(write(handle,(char *)dysta_tphys[0],sizeof(dysta_tphys)) != sizeof(dysta_tphys) ) {
      perror("Error writing to file InternalDynamicParameters/dysta_tphys.dat");
   }
   close( handle );
/*--------------------------------------------------------*/
/*++++Daily FET Stats File (1 days - sum and total) ++++*/
/*--------------------------------------------------------*/
   if( (handle=open( DYNAMIC_dysta_fet, O_WRONLY | O_CREAT,0644 )) == -1 ) {
      perror("Error creating or opening file InternalDynamicParameters/dysta_fet.dat.");
   }
   if(write(handle,(char *)dysta_fet[0],sizeof(dysta_fet)) != sizeof(dysta_fet) ) {
      perror("Error writing to file InternalDynamicParameters/dysta_fet.dat");
   }
   close( handle );
/*--------------------------------------------------------*/
/*++++Daily GS Stats File (1 days - sum and total) ++++*/
/*--------------------------------------------------------*/
   if( (handle=open( DYNAMIC_dysta_gs, O_WRONLY | O_CREAT,0644 )) == -1 ) {
      perror("Error creating or opening file InternalDynamicParameters/dysta_gs.dat.");
   }
   if(write(handle,(char *)dysta_gs[0],sizeof(dysta_gs)) != sizeof(dysta_gs) ) {
      perror("Error writing to file InternalDynamicParameters/dysta_gs.dat");
   }
   close( handle );
/*--------------------------------------------------------*/
/*++++Daily DENSITY Stats File (1 days - sum and total) ++++*/
/*--------------------------------------------------------*/
   if( (handle=open( DYNAMIC_dysta_den, O_WRONLY | O_CREAT,0644 )) == -1 ) {
      perror("Error creating or opening file InternalDynamicParameters/dysta_den.dat.");
   }
   if(write(handle,(char *)dysta_den[0],sizeof(dysta_den)) != sizeof(dysta_den) ) {
      perror("Error writing to file InternalDynamicParameters/dysta_den.dat");
   }
   close( handle );
/*--------------------------------------------------------*/
/*++++Daily Evol Stats File (1 days - sum and total) ++++*/
/*--------------------------------------------------------*/
   if( (handle=open( DYNAMIC_dysta_evol, O_WRONLY | O_CREAT,0644 )) == -1 ) {
      perror("Error creating or opening file InternalDynamicParameters/dysta_evol.dat.");
   }
   if(write(handle,(char *)dysta_evol[0],sizeof(dysta_evol)) != sizeof(dysta_evol) ) {
      perror("Error writing to file InternalDynamicParameters/dysta_evol.dat");
   }
   close( handle );
/*--------------------------------------------------------*/
/*++++Daily Regime Stats File (1 days - sum and total) ++++*/
/*--------------------------------------------------------*/
   if( (handle=open( DYNAMIC_dysta_reg, O_WRONLY | O_CREAT,0644 )) == -1 ) {
      perror("Error creating or opening file InternalDynamicParameters/dysta_reg.dat.");
   }
   if(write(handle,(char *)dysta_reg[0],sizeof(dysta_reg)) != sizeof(dysta_reg) ) {
      perror("Error writing to file InternalDynamicParameters/dysta_reg.dat");
   }
   close( handle );

/*--------------------------------------------------------*/
/*++++Daily SWE Stats File (1 days - sum and total) ++++*/
/*--------------------------------------------------------*/
   if( (handle=open( DYNAMIC_dysta_swe, O_WRONLY | O_CREAT,0644 )) == -1 ) {
      perror("Error creating or opening file InternalDynamicParameters/dysta_swe.dat.");
   }
   if(write(handle,(char *)dysta_swe[0],sizeof(dysta_swe)) != sizeof(dysta_swe) ) {
      perror("Error writing to file InternalDynamicParameters/dysta_swe.dat");
   }
   close( handle );

/*--------------------------------------------------------*/
/*++++Daily SD Stats File (1 days - sum and total) ++++*/
/*--------------------------------------------------------*/
   if( (handle=open( DYNAMIC_dysta_sd, O_WRONLY | O_CREAT,0644 )) == -1 ) {
      perror("Error creating or opening file InternalDynamicParameters/dysta_sd.dat.");
   }
   if(write(handle,(char *)dysta_sd[0],sizeof(dysta_sd)) != sizeof(dysta_sd) ) {
      perror("Error writing to file InternalDynamicParameters/dysta_sd.dat");
   }
   close( handle );

/*-----------------------------------------*/
/*++++SNOW COVER AREA HISTORY DATA++++*/
/*-----------------------------------------*/
   if( (handle=open( DYNAMIC_SCAhistory, O_WRONLY | O_CREAT,0644 )) == -1 ) {
      perror("Error creating or opening file InternalDynamicParameters/SCAhistory.dat.");
//      return(FAIL);
   }
   if(write(handle,(char *)scahistory[0],sizeof(scahistory)) != sizeof(scahistory) ) {
      perror("Error writing to file InternalDynamicParameters/SCAhistory.dat");
//      return(FAIL);
   }
   close( handle );

/*-----------------------------------------------------*/
/*++++TEMPERATURE HISTORY (JULIAN) 10 DAYS OF OBS  ++++*/
/*-----------------------------------------------------*/
   if( (handle=open( DYNAMIC_thistory, O_WRONLY | O_CREAT,0644 )) == -1 ) {
      perror("Error creating or opening file InternalDynamicParameters/thistory.dat.");
//      return(FAIL);
   }
   if(write(handle,(char *)thistory[0],sizeof(thistory)) != sizeof(thistory) ) {
      perror("Error writing to file InternalDynamicParameters/thistory.dat");
//      return(FAIL);
   }
   close( handle );


/*-----------------------------------------*/
/*++++SNOW Depth (instant) HISTORY DATA (10 DAYS) ++++*/
/*-----------------------------------------*/
   if( (handle=open( DYNAMIC_SDhistoryInst, O_WRONLY | O_CREAT,0644 )) == -1 ) {
      perror("Error creating or opening file InternalDynamicParameters/SDhistoryInst.dat.");
//      return(FAIL);
   }
   if(write(handle,(char *)sdhistory[0],sizeof(sdhistory)) != sizeof(sdhistory) ) {
      perror("Error writing to file InternalDynamicParameters/SDhistoryInst.dat");
//      return(FAIL);
   }
   close( handle );

/*--------------------------------------------------------------*/
/*++++Tb18-Tb36 VV Smoothed Gaussian HISTORY DATA (10 DAYS) ++++*/
/*--------------------------------------------------------------*/
   if( (handle=open( DYNAMIC_t1836v_smth, O_WRONLY | O_CREAT,0644 )) == -1 ) {
      perror("Error creating or opening file InternalDynamicParameters/T1836V_gaussian6.dat.");
//      return(FAIL);
   }
   if(write(handle,(char *)tb1836vf_smth[0],sizeof(tb1836vf_smth)) != sizeof(tb1836vf_smth) ) {
      perror("Error writing to file InternalDynamicParameters/T1836V_gaussian6.dat");
//      return(FAIL);
   }
   close( handle );
/*--------------------------------------------------------------*/
/*++++Tb18-Tb36 HH Smoothed Gaussian HISTORY DATA (10 DAYS) ++++*/
/*--------------------------------------------------------------*/
   if( (handle=open( DYNAMIC_t1836h_smth, O_WRONLY | O_CREAT,0644 )) == -1 ) {
      perror("Error creating or opening file InternalDynamicParameters/T1836H_gaussian6.dat.");
//      return(FAIL);
   }
   if(write(handle,(char *)tb1836hf_smth[0],sizeof(tb1836hf_smth)) != sizeof(tb1836hf_smth) ) {
      perror("Error writing to file InternalDynamicParameters/T1836H_gaussian6.dat");
//      return(FAIL);
   }
   close( handle );

/*-----------------------------------------*/
/*++++ESTIMATED TEMPERATURE HISTORY DATA++++*/
/*-----------------------------------------*/
   if( (handle=open( DYNAMIC_T_est, O_WRONLY | O_CREAT,0644 )) == -1 ) {
      perror("Error creating or opening file InternalDynamicParameters/T_est.dat.");
//      return(FAIL);
   }
   if(write(handle,(char *)t_est[0],sizeof(t_est)) != sizeof(t_est) ) {
      perror("Error writing to file InternalDynamicParameters/T_est.dat");
//      return(FAIL);
   }
   close( handle );

/*-----------------------------------------*/
/*++++ESTIMATED SNOW REGIME HISTORY DATA++++*/
/*-----------------------------------------*/
   if( (handle=open( DYNAMIC_regime, O_WRONLY | O_CREAT,0644 )) == -1 ) {
      perror("Error creating or opening file InternalDynamicParameters/regime.dat.");
//      return(FAIL);
   }
   if(write(handle,(char *)regime[0],sizeof(regime)) != sizeof(regime) ) {
      perror("Error writing to file InternalDynamicParameters/regime.dat");
//      return(FAIL);
   }
   close( handle );

/*-----------------------------------------*/
/*++++ Facet Evolution Time file DATA++++*/
/*-----------------------------------------*/
   if( (handle=open( DYNAMIC_fet, O_WRONLY | O_CREAT,0644 )) == -1 ) {
      perror("Error creating or opening file InternalDynamicParameters/fet.dat.");
//      return(FAIL);
   }
   if(write(handle,(char *)fet[0],sizeof(fet)) != sizeof(fet) ) {
      perror("Error writing to file InternalDynamicParameters/fet.dat");
//      return(FAIL);
   }
   close( handle );

/*-----------------------------------------*/
/*++++ EVOLUTION file DATA++++*/
/*-----------------------------------------*/
   if( (handle=open( DYNAMIC_evolution, O_WRONLY | O_CREAT,0644 )) == -1 ) {
      perror("Error creating or opening file InternalDynamicParameters/evolution.dat.");
//      return(FAIL);
   }
   if(write(handle,(char *)evolution[0],sizeof(evolution)) != sizeof(evolution) ) {
      perror("Error writing to file InternalDynamicParameters/evolution.dat");
//      return(FAIL);
   }
   close( handle );

/*-----------------------------------------*/
/*++++ESTIMATED SNOW GS file DATA++++*/
/*-----------------------------------------*/
   if( (handle=open( DYNAMIC_gs, O_WRONLY | O_CREAT,0644 )) == -1 ) {
      perror("Error creating or opening file InternalDynamicParameters/gs.dat.");
//      return(FAIL);
   }
   if(write(handle,(char *)gs[0],sizeof(gs)) != sizeof(gs) ) {
      perror("Error writing to file InternalDynamicParameters/gs.dat");
//      return(FAIL);
   }
   close( handle );

/*-----------------------------------------*/
/*++++ESTIMATED SNOW PERSISTENCE file DATA++++*/
/*-----------------------------------------*/
   if( (handle=open( DYNAMIC_persistence, O_WRONLY | O_CREAT,0644 )) == -1 ) {
      perror("Error creating or opening file InternalDynamicParameters/persistence.dat.");
//      return(FAIL);
   }
   if(write(handle,(char *)persistence[0],sizeof(persistence)) != sizeof(persistence) ) {
      perror("Error writing to file InternalDynamicParameters/persistence.dat");
//      return(FAIL);
   }
   close( handle );

/*-----------------------------------------*/
/*++++ESTIMATED SNOW DENSITY file DATA++++*/
/*-----------------------------------------*/
   if( (handle=open( DYNAMIC_den, O_WRONLY | O_CREAT,0644 )) == -1 ) {
      perror("Error creating or opening file InternalDynamicParameters/density.dat.");
//      return(FAIL);
   }
   if(write(handle,(char *)denest[0],sizeof(denest)) != sizeof(denest) ) {
      perror("Error writing to file InternalDynamicParameters/density.dat");
//      return(FAIL);
   }
   close( handle );

/*--------------------------------------------------------*/
/*++++         WRITE EACH DAILY GS TO A FILE         ++++*/
/*--------------------------------------------------------*/
   if (outswitchtmp == 1) {
     strcat(gsfname,sJulian);
     strcat(gsfname,"_gs.dat");
     //printf("GS Filename %s \n",gsfname);
     if( (handle=open( gsfname, O_WRONLY | O_CREAT,0644 )) == -1 ) {
        perror("Error creating/opening Daily GS file InternalDynamicParameters/gs/???????_gs.dat.");
     }
     if(write(handle,(char *)gs[0],sizeof(gs)) != sizeof(gs) ) {
        perror("Error writing to daily GS file");
     }
     close( handle );
   }

/*--------------------------------------------------------*/
/*++++         WRITE EACH DAILY DENSITY TO A FILE         ++++*/
/*--------------------------------------------------------*/
   if (outswitchtmp == 1) {
     strcat(denfname,sJulian);
     strcat(denfname,"_den.dat");
     //printf("DENSITY Filename %s \n",denfname);
     if( (handle=open( denfname, O_WRONLY | O_CREAT,0644 )) == -1 ) {
        perror("Error creating/opening Daily DENSITY file InternalDynamicParameters/density/???????_den.dat.");
     }
     if(write(handle,(char *)denest[0],sizeof(denest)) != sizeof(denest) ) {
        perror("Error writing to daily DENSITY file");
     }
     close( handle );
   }

/*--------------------------------------------------------*/
/*++++         WRITE EACH DAILY SNOW DEPTH TO A FILE         ++++*/
/*--------------------------------------------------------*/
   if (outswitchtmp == 1) {
     strcat(sdfname,sJulian);
     strcat(sdfname,"_sd.dat");
     //printf("Daily SnowDepth Filename %s \n",sdfname);
     if( (handle=open( sdfname, O_WRONLY | O_CREAT,0644 )) == -1 ) {
        perror("Error creating/opening Daily SNOW DEPTH file InternalDynamicParameters/snow/???????_sd.dat.");
     }
     if(write(handle,(char *)daysd[0],sizeof(daysd)) != sizeof(daysd) ) {
        perror("Error writing to daily Snow Depth file");
     }
     close( handle );
   }

/*--------------------------------------------------------*/
/*++++         WRITE EACH DAILY SWE TO A FILE         ++++*/
/*--------------------------------------------------------*/
   if (outswitchtmp == 1) {
     strcat(swefname,sJulian);
     strcat(swefname,"_swe.dat");
     //printf("Daily SWE Filename %s \n",swefname);
     if( (handle=open( swefname, O_WRONLY | O_CREAT,0644 )) == -1 ) {
        perror("Error creating/opening Daily SWE file InternalDynamicParameters/snow/???????_swe.dat.");
     }
     if(write(handle,(char *)dayswe[0],sizeof(dayswe)) != sizeof(dayswe) ) {
        perror("Error writing to daily SWE file");
     }
     close( handle );
   }


   return(SUCCESS);
}

