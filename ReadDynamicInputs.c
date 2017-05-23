/*   This routine reads in the input dynamic arrays at the beginning of the program.   */
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
extern short int  sdhistory[ROWS_E2][COLS_E2][10],tb1836hf_smth[ROWS_E2][COLS_E2][10],tb1836vf_smth[ROWS_E2][COLS_E2][10],persistence[ROWS_E2][COLS_E2][2];
extern float	t_est[ROWS_E2][COLS_E2][11],gs[ROWS_E2][COLS_E2],fet[ROWS_E2][COLS_E2],denest[ROWS_E2][COLS_E2];
extern float	dysta_tphys[ROWS_E2][COLS_E2][2],dysta_fet[ROWS_E2][COLS_E2][2],dysta_gs[ROWS_E2][COLS_E2][2],dysta_den[ROWS_E2][COLS_E2][2];
extern int	dysta_evol[ROWS_E2][COLS_E2];
extern int	dysta_reg[ROWS_E2][COLS_E2][3];
extern float	dysta_sd[ROWS_E2][COLS_E2][2], dysta_swe[ROWS_E2][COLS_E2][2];
extern float	dysta_1836v[ROWS_E2][COLS_E2][2],dysta_1836h[ROWS_E2][COLS_E2][2];
extern int	regime[ROWS_E2][COLS_E2],evolution[ROWS_E2][COLS_E2];
extern double	thistory[ROWS_E2][COLS_E2][11],scahistory[ROWS_E2][COLS_E2][11];
extern char	debug_retr,debug_read,debug_write;

/*====================================================================================*/

int ReadDynamicInputs()
{
   int	handle;
    char place[2];    


/*--------------------------------------------------------*/
/*++++Daily Tb18-36 VVVV-Pol Stats File (1 days - sum and total) ++++*/
/*--------------------------------------------------------*/
   if( (handle=open( DYNAMIC_dysta_t1836v, O_RDONLY )) == -1 ) {
      perror("InternalDynamicParameters/dysta_1836v.dat does not exist. ");
      printf(" Using new array.\n");
   }
   else if(read(handle,(char *)dysta_1836v[0],sizeof(dysta_1836v)) != sizeof(dysta_1836v) ) {
      perror("Error reading dysta_1836v.dat");
      return(FAIL);
   }
   close( handle );
  if (debug_read == 1) printf("DEBUG   read dysta_t1836v \n");
/*--------------------------------------------------------*/
/*++++Daily Tb18-36 HHHH-Pol Stats File (1 days - sum and total) ++++*/
/*--------------------------------------------------------*/
   if( (handle=open( DYNAMIC_dysta_t1836h, O_RDONLY )) == -1 ) {
      perror("InternalDynamicParameters/dysta_1836h.dat does not exist. ");
      printf(" Using new array.\n");
   }
   else if(read(handle,(char *)dysta_1836h[0],sizeof(dysta_1836h)) != sizeof(dysta_1836h) ) {
      perror("Error reading dysta_1836h.dat");
      return(FAIL);
   }
   close( handle );
  if (debug_read == 1) printf("DEBUG   read dysta_1836h \n");
/*--------------------------------------------------------*/
/*++++Daily TPhys Stats File (1 days - sum and total) ++++*/
/*--------------------------------------------------------*/
   if( (handle=open( DYNAMIC_dysta_tphys, O_RDONLY )) == -1 ) {
      perror("InternalDynamicParameters/dysta_tphys.dat does not exist. ");
      printf(" Using new array.\n");
   }
   else if(read(handle,(char *)dysta_tphys[0],sizeof(dysta_tphys)) != sizeof(dysta_tphys) ) {
      perror("Error reading dysta_tphys.dat");
      return(FAIL);
   }
   close( handle );
  if (debug_read == 1) printf("DEBUG   read dysta_tphys \n");
/*--------------------------------------------------------*/
/*++++Daily FET Stats File (1 days - sum and total) ++++*/
/*--------------------------------------------------------*/
   if( (handle=open( DYNAMIC_dysta_fet, O_RDONLY )) == -1 ) {
      perror("InternalDynamicParameters/dysta_fet.dat does not exist.");
      printf(" Using new array.\n");
   }
   else if(read(handle,(char *)dysta_fet[0],sizeof(dysta_fet)) != sizeof(dysta_fet) ) {
      perror("Error reading dysta_fet.dat");
      return(FAIL);
   }
   close( handle );
  if (debug_read == 1) printf("DEBUG   read dysta_fet \n");
/*--------------------------------------------------------*/
/*++++Daily GS Stats File (1 days - sum and total) ++++*/
/*--------------------------------------------------------*/
   if( (handle=open( DYNAMIC_dysta_gs, O_RDONLY )) == -1 ) {
      perror("InternalDynamicParameters/dysta_gs.dat does not exist. ");
      printf(" Using new array.\n");
   }
   else if(read(handle,(char *)dysta_gs[0],sizeof(dysta_gs)) != sizeof(dysta_gs) ) {
      perror("Error reading dysta_gs.dat");
      return(FAIL);
   }
   close( handle );
  if (debug_read == 1) printf("DEBUG   read dysta_gs \n");
/*--------------------------------------------------------*/
/*++++Daily DENSITY Stats File (1 days - sum and total) ++++*/
/*--------------------------------------------------------*/
   if( (handle=open( DYNAMIC_dysta_den, O_RDONLY )) == -1 ) {
      perror("InternalDynamicParameters/dysta_den.dat does not exist. ");
      printf(" Using new array.\n");
   }
   else if(read(handle,(char *)dysta_den[0],sizeof(dysta_den)) != sizeof(dysta_den) ) {
      perror("Error reading dysta_den.dat");
      return(FAIL);
   }
   close( handle );
  if (debug_read == 1) printf("DEBUG   read dysta_den \n");
//*--------------------------------------------------------*/
/*++++Daily Evol Stats File (1 days - sum and total) ++++*/
/*--------------------------------------------------------*/
   if( (handle=open( DYNAMIC_dysta_evol, O_RDONLY )) == -1 ) {
      perror("InternalDynamicParameters/dysta_evol.dat does not exist. ");
      printf(" Using new array.\n");
   }
   else if(read(handle,(char *)dysta_evol[0],sizeof(dysta_evol)) != sizeof(dysta_evol) ) {
      perror("Error reading dysta_evol.dat");
      return(FAIL);
   }
   close( handle );
  if (debug_read == 1) printf("DEBUG   read dysta_evol \n");
/*--------------------------------------------------------*/
/*++++Daily Regime Stats File (1 days - sum and total) ++++*/
/*--------------------------------------------------------*/
   if( (handle=open( DYNAMIC_dysta_reg, O_RDONLY )) == -1 ) {
       perror("InternalDynamicParameters/dysta_reg.dat does not exist. ");
      printf(" Using new array.\n");
   }
   else if(read(handle,(char *)dysta_reg[0],sizeof(dysta_reg)) != sizeof(dysta_reg) ) {
      perror("Error reading dysta_reg.dat");
      return(FAIL);
   }
   close( handle );
  if (debug_read == 1) printf("DEBUG   read dysta_reg \n");
/*--------------------------------------------------------*/
/*++++Daily SWE Stats File (1 days - sum and total) ++++*/
/*--------------------------------------------------------*/
   if( (handle=open( DYNAMIC_dysta_swe, O_RDONLY )) == -1 ) {
       perror("InternalDynamicParameters/dysta_swe.dat does not exist. ");
      printf(" Using new array.\n");
   }
   else if(read(handle,(char *)dysta_swe[0],sizeof(dysta_swe)) != sizeof(dysta_swe) ) {
      perror("Error reading dysta_swe.dat");
      return(FAIL);
   }
   close( handle );
  if (debug_read == 1) printf("DEBUG   read dysta_swe \n");
/*--------------------------------------------------------*/
/*++++Daily SD Stats File (1 days - sum and total) ++++*/
/*--------------------------------------------------------*/
   if( (handle=open( DYNAMIC_dysta_sd, O_RDONLY )) == -1 ) {
       perror("InternalDynamicParameters/dysta_sd.dat does not exist. ");
      printf(" Using new array.\n");
   }
   else if(read(handle,(char *)dysta_sd[0],sizeof(dysta_sd)) != sizeof(dysta_sd) ) {
      perror("Error reading dysta_sd.dat");
      return(FAIL);
   }
   close( handle );
  if (debug_read == 1) printf("DEBUG   read dysta_sd \n");
/*-----------------------------------------*/
/*++++SNOW COVER AREA HISTORY DATA (10 days) ++++*/
/*-----------------------------------------*/
   if( (handle=open( DYNAMIC_SCAhistory, O_RDONLY )) == -1 ) {
      perror("InternalDynamicParameters/SCAhistory.dat does not exist. ");
      printf(" Using new array.\n");
   }
   else if(read(handle,(char *)scahistory[0],sizeof(scahistory)) != sizeof(scahistory) ) {
      perror("Error reading scahistory.dat");
      return(FAIL);
   }
   close( handle );
  if (debug_read == 1) printf("DEBUG   read SCAhistory \n");

/*-----------------------------------------*/
/*++++Temperature History (JULIAN) of 10 passes  ++++*/
/*-----------------------------------------*/
   if( (handle=open( DYNAMIC_thistory, O_RDONLY )) == -1 ) {
      perror("InternalDynamicParameters/thistory.dat does not exist. ");
      printf(" Using new array.\n");
   }
   else if(read(handle,(char *)thistory[0],sizeof(thistory)) != sizeof(thistory) ) {
      perror("Error reading thistory.dat");
      return(FAIL);
   }
   close( handle );

/*-----------------------------------------*/
/*++++SNOW Depth (10 DAYS) HISTORY DATA++++*/
/*-----------------------------------------*/
   if( (handle=open( DYNAMIC_SDhistoryInst, O_RDONLY )) == -1 ) {
      perror("InternalDynamicParameters/SDhistoryInst.dat does not exist. ");
      printf(" Using new array.\n");
   }
   else if(read(handle,(char *)sdhistory[0],sizeof(sdhistory)) != sizeof(sdhistory) ) {
      perror("Error reading SDhistoryINST.dat");
      return(FAIL);
   }
   close( handle );

/*------------------------------------------------------------*/
/*++++Tb18-36 VVV GAUSSIAN SMOOTHED (10 DAYS) HISTORY DATA++++*/
/*------------------------------------------------------------*/
   if( (handle=open( DYNAMIC_t1836v_smth, O_RDONLY )) == -1 ) {
      perror("InternalDynamicParameters/T1836V-10days-GaussianSmoothed.dat does not exist. ");
      printf(" Using new array.\n");
   }
   else if(read(handle,(char *)tb1836vf_smth[0],sizeof(tb1836vf_smth)) != sizeof(tb1836vf_smth) ) {
      perror("Error reading T1836V-10days-GaussianSmoothed.dat");
      return(FAIL);
   }
   close( handle );
/*------------------------------------------------------------*/
/*++++Tb18-36 HHH GAUSSIAN SMOOTHED (10 DAYS) HISTORY DATA++++*/
/*------------------------------------------------------------*/
   if( (handle=open( DYNAMIC_t1836h_smth, O_RDONLY )) == -1 ) {
      perror("InternalDynamicParameters/T1836H-10days-GaussianSmoothed.dat does not exist. ");
      printf(" Using new array.\n");
   }
   else if(read(handle,(char *)tb1836hf_smth[0],sizeof(tb1836hf_smth)) != sizeof(tb1836hf_smth) ) {
      perror("Error reading T1836H-10days-GaussianSmoothed.dat");
      return(FAIL);
   }
   close( handle );

/*-----------------------------------------*/
/*++++ESTIMATED TEMPERATURE HISTORY DATA++++*/
/*-----------------------------------------*/
   if( (handle=open( DYNAMIC_T_est, O_RDONLY )) == -1 ) {
      perror("InternalDynamicParameters/T_est.dat does not exist. ");
      printf(" Using new array.\n");
   }
   else if(read(handle,(char *)t_est[0],sizeof(t_est)) != sizeof(t_est) ) {
      perror("Error reading T_est.dat");
      return(FAIL);
   }
   close( handle );

/*-----------------------------------------*/
/*++++ESTIMATED SNOW REGIME HISTORY DATA++++*/
/*-----------------------------------------*/
   if( (handle=open( DYNAMIC_regime, O_RDONLY )) == -1 ) {
      perror("InternalDynamicParameters/regime.dat does not exist. ");
      printf(" Using new array.\n");
   }
   else if(read(handle,(char *)regime[0],sizeof(regime)) != sizeof(regime) ) {
      perror("Error reading regime.dat");
      return(FAIL);
   }
   close( handle );

/*-----------------------------------------*/
/*++++ Facet Evolution Time file DATA++++*/
/*-----------------------------------------*/
   if( (handle=open( DYNAMIC_fet, O_RDONLY )) == -1 ) {
      perror("InternalDynamicParameters/fet.dat does not exist. ");
      printf(" Using new array.\n");
   }
   else if(read(handle,(char *)fet[0],sizeof(fet)) != sizeof(fet) ) {
      perror("Error reading fet.dat");
      return(FAIL);
   }
   close( handle );

/*-----------------------------------------*/
/*++++ EVOLUTION file DATA++++*/
/*-----------------------------------------*/
   if( (handle=open( DYNAMIC_evolution, O_RDONLY )) == -1 ) {
      perror("InternalDynamicParameters/evolution.dat does not exist. ");
      printf(" Using new array.\n");
   }
   else if(read(handle,(char *)evolution[0],sizeof(evolution)) != sizeof(evolution) ) {
      perror("Error reading evolution.dat");
      return(FAIL);
   }
   close( handle );

/*-----------------------------------------*/
/*++++ESTIMATED SNOW GS file DATA++++*/
/*-----------------------------------------*/
   if( (handle=open( DYNAMIC_gs, O_RDONLY )) == -1 ) {
      perror("InternalDynamicParameters/gs.dat does not exist. ");
      printf(" Using new array.\n");
   }
   else if(read(handle,(char *)gs[0],sizeof(gs)) != sizeof(gs) ) {
      perror("Error reading gs.dat");
      return(FAIL);
   }
   close( handle );

/*-----------------------------------------*/
/*++++ESTIMATED SNOW Density file DATA++++*/
/*-----------------------------------------*/
   if( (handle=open( DYNAMIC_den, O_RDONLY )) == -1 ) {
      perror("InternalDynamicParameters/density.dat does not exist. ");
      printf(" Using new array.\n");
   }
   else if(read(handle,(char *)denest[0],sizeof(denest)) != sizeof(denest) ) {
      perror("Error reading density.dat");
      return(FAIL);
   }
   close( handle );

/*-----------------------------------------*/
/*++++PERSISTENCE file DATA++++*/
/*-----------------------------------------*/
   if( (handle=open( DYNAMIC_persistence, O_RDONLY )) == -1 ) {
      perror("InternalDynamicParameters/persistence.dat does not exist. ");
      printf(" Using new array.\n");
   }
   else if(read(handle,(char *)persistence[0],sizeof(persistence)) != sizeof(persistence) ) {
      perror("Error reading persistence.dat");
      return(FAIL);
   }
   close( handle );


   return(SUCCESS);
}

