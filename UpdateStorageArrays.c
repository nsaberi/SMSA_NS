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

/*** Globals ***/
char		NoDynamicInputFlag;
extern	short int sdhistory[ROWS_E2][COLS_E2][10],tb1836vf_smth[ROWS_E2][COLS_E2][10],tb1836hf_smth[ROWS_E2][COLS_E2][10],persistence[ROWS_E2][COLS_E2][2];
extern  short int dayswe[ROWS_E2][COLS_E2],daysd[ROWS_E2][COLS_E2];
extern	float	t_est[ROWS_E2][COLS_E2][11],gs[ROWS_E2][COLS_E2],fet[ROWS_E2][COLS_E2],denest[ROWS_E2][COLS_E2];
extern float	dysta_tphys[ROWS_E2][COLS_E2][2],dysta_fet[ROWS_E2][COLS_E2][2],dysta_gs[ROWS_E2][COLS_E2][2],dysta_den[ROWS_E2][COLS_E2][2];
extern int	dysta_evol[ROWS_E2][COLS_E2];
extern int	dysta_reg[ROWS_E2][COLS_E2][3];
extern float	dysta_sd[ROWS_E2][COLS_E2][2], dysta_swe[ROWS_E2][COLS_E2][2];
extern float	dysta_1836v[ROWS_E2][COLS_E2][2],dysta_1836h[ROWS_E2][COLS_E2][2];
extern int	regime[ROWS_E2][COLS_E2],evolution[ROWS_E2][COLS_E2];
extern double	thistory[ROWS_E2][COLS_E2][11],scahistory[ROWS_E2][COLS_E2][11];
extern char	debug_retr,debug_read,debug_write,debug_upd;
extern int	plimit;

/*====================================================================================*/

int UpdateStorageArrays()
{
int dt,col,row,sr;
/*-----------------------------*/
/* UPDATE ARRAYS */
for (row=0;row<ROWS_E2;row++){
  for (col=0;col<COLS_E2;col++){
    for (dt=10;dt>0;--dt){
      t_est[row][col][dt]=t_est[row][col][dt-1];
      scahistory[row][col][dt]=scahistory[row][col][dt-1];
      thistory[row][col][dt]=thistory[row][col][dt-1];
      if (dt < 10) sdhistory[row][col][dt]=sdhistory[row][col][dt-1];
      if (dt < 10) tb1836vf_smth[row][col][dt]=tb1836vf_smth[row][col][dt-1];
      if (dt < 10) tb1836hf_smth[row][col][dt]=tb1836hf_smth[row][col][dt-1];
    }
/* CALCULATE DAILY AVERAGES FOR ALL DYSTA'S */
    if (dysta_tphys[row][col][1] > 0) t_est[row][col][0]=dysta_tphys[row][col][0]/dysta_tphys[row][col][1];	/* Calculate average Tphys for the day (K)*/
    if (dysta_gs[row][col][1] > 0) gs[row][col]=dysta_gs[row][col][0]/dysta_gs[row][col][1];	/* CALCULATE AVERAGE GS FOR THE PIXEL mm */
    if (dysta_den[row][col][1] > 0) denest[row][col]=dysta_den[row][col][0]/dysta_den[row][col][1];	/* CALCULATE AVERAGE DENSITY FOR THE PIXEL g/cm^3*/
    if (dysta_fet[row][col][1] > 0) fet[row][col]=dysta_fet[row][col][0]/dysta_fet[row][col][1];	/* CALCULATE AVERAGE FET FOR THE PIXEL (day)*/
    if (dysta_sd[row][col][1] > 0) daysd[row][col]=1000*dysta_sd[row][col][0]/dysta_sd[row][col][1];	/* CALCULATE AVERAGE daily SNOW DEPTH FOR THE PIXEL IN MM */
      else if (dysta_sd[row][col][1] == 0 && dysta_tphys[row][col][1] > 0) daysd[row][col]=0.;
    if (dysta_swe[row][col][1] > 0) dayswe[row][col]=10*dysta_swe[row][col][0]/dysta_swe[row][col][1];	/* CALCULATE AVERAGE daily SWE FOR THE PIXEL (in mm x 10) */
      else if (dysta_swe[row][col][1] == 0 && dysta_tphys[row][col][1] > 0) dayswe[row][col]=0.;
    if (dysta_1836v[row][col][1] > 0) tb1836vf_smth[row][col][0]=(int)(10*dysta_1836v[row][col][0]/dysta_1836v[row][col][1]);	/* ADD GAUSSIAN WEIGHTED Tb18-36V to history */
    if (dysta_1836h[row][col][1] > 0) tb1836hf_smth[row][col][0]=(int)(10*dysta_1836h[row][col][0]/dysta_1836h[row][col][1]);	/* ADD GAUSSIAN WEIGHTED Tb18-36V to history */

/* CALCULATE MODAL REGIME FOR THE PIXEL ELSE SET TO EQ GROWTH (2) IF A TIE OR 0 IF NO SNOW */
    regime[row][col]=0;	
    sr=dysta_reg[row][col][0]+dysta_reg[row][col][1]+dysta_reg[row][col][2];
    if (sr > 0) {
      if (dysta_reg[row][col][0] > dysta_reg[row][col][1] && dysta_reg[row][col][0] > dysta_reg[row][col][2]) regime[row][col]=1;
        else if (dysta_reg[row][col][1] > dysta_reg[row][col][0] && dysta_reg[row][col][1] > dysta_reg[row][col][2]) regime[row][col]=2;
        else if (dysta_reg[row][col][2] > dysta_reg[row][col][0] && dysta_reg[row][col][2] > dysta_reg[row][col][1]) regime[row][col]=3;
        else regime[row][col]=2;
    }

/* UPDATE PERSISTENCE COUNTER */
    if (persistence[row][col][0] > 0) { 
       persistence[row][col][1]=persistence[row][col][1]+1;
       if (persistence[row][col][1] > plimit) persistence[row][col][1]=plimit;
    }
    if (persistence[row][col][0] == 0) {
       persistence[row][col][1]=persistence[row][col][1]-1;
       if (persistence[row][col][1] < 0) persistence[row][col][1]=0;
    }
    persistence[row][col][0]=0;

/* UPDATE SDHISTORY FILE */
    if (dysta_sd[row][col][1] > 0) sdhistory[row][col][0]=1000*dysta_sd[row][col][0]/dysta_sd[row][col][1];	/* ADD CALCULATED AVERAGE depth TO THE PIXEL HISTORY */
       else if (dysta_sd[row][col][1] == 0 && dysta_tphys[row][col][1] > 0 && persistence[row][col][1] == 0) sdhistory[row][col][0]=0.;
       else if (dysta_sd[row][col][1] == 0 && dysta_tphys[row][col][1] > 0 && persistence[row][col][1] > 0) sdhistory[row][col][0]=sdhistory[row][col][1];
       else if (dysta_sd[row][col][1] == 0 && dysta_tphys[row][col][1] == 0 && sdhistory[row][col][1] > 0) sdhistory[row][col][0]=sdhistory[row][col][1];
 }
}
/*------------------------------*/
/* UPDATE ARRAYS FOR THE PREVIOUS DAY - after it has been done and at the start of the new day */
  if (debug_upd == 1) printf("DEBUG   finished updating arrays  \n");


   return(SUCCESS);
}

