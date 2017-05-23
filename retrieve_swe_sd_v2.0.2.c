/*   This routine calculates snow depth and SWE from the corrected Tb data and a grain/density model.   */
/* CREATED JANUARY 2016 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>


#define ROWS_E2	2880 
#define COLS_E2	1440
#define FAIL (-1)
#define SUCCESS	0

extern short int sdhistory[ROWS_E2][COLS_E2][10],tb1836vf_smth[ROWS_E2][COLS_E2][10],tb1836hf_smth[ROWS_E2][COLS_E2][10],persistence[ROWS_E2][COLS_E2][2];
extern float	t_est[ROWS_E2][COLS_E2][11],gs[ROWS_E2][COLS_E2],fet[ROWS_E2][COLS_E2],denest[ROWS_E2][COLS_E2];
extern float	dysta_tphys[ROWS_E2][COLS_E2][2],dysta_fet[ROWS_E2][COLS_E2][2],dysta_gs[ROWS_E2][COLS_E2][2],dysta_den[ROWS_E2][COLS_E2][2];
extern int	dysta_evol[ROWS_E2][COLS_E2];
extern int	dysta_reg[ROWS_E2][COLS_E2][3];
extern float	dysta_sd[ROWS_E2][COLS_E2][2],dysta_swe[ROWS_E2][COLS_E2][2];
extern float	dysta_1836v[ROWS_E2][COLS_E2][2],dysta_1836h[ROWS_E2][COLS_E2][2];
extern int	regime[ROWS_E2][COLS_E2],evolution[ROWS_E2][COLS_E2];
extern char	debug_retr,debug_read,debug_write;
extern double	thistory[ROWS_E2][COLS_E2][11],scahistory[ROWS_E2][COLS_E2][11];
extern unsigned char	snowclass[ROWS_E2][COLS_E2];		
extern float	v10,h10,v18, h18,v23, h23, v36, h36, v89,h89;
extern float	v10f,h10f,v18f, h18f,v23f, h23f, v36f, h36f, v89f,h89f;
extern float	minsnowlimit;
extern int	plimit;
extern double	scat, sc1036v,sc1036h, sc1836v,sc1836h, sc89v,sc89h, sc36,sc89,scx;
extern double	snow, albedo, coef, FloatSnow,pol36fact;
extern double	pol18f,pol36f,pol89f,t1836vf,t1836hf,t2389hf,t89v36hf,t18h23vf,t3689vf ;
extern short int DMRT_LUT[25][20][45][200][2];


/* TIMER  vvvvvvvvvvvv*/
uint64_t first_time,second_time,diff_time;
uint64_t get_posix_clock_time ()
{
    struct timespec ts;

    if (clock_gettime (CLOCK_MONOTONIC, &ts) == 0)
        return (uint64_t) (ts.tv_sec * 1000000 + ts.tv_nsec / 1000);
    else
        return 0;
}
/* TIMER ^^^^^^^^^^^^^*/


float retrieve_swe_sd( int *easex, int *easey, double *Julian, int *retrievalflag, float *EstSnowdepth, float *EstSnowWE)
{
  float 	tgrad,tdiff,tphys,meantgrad,meantgrad5,meansnow10,sumsnow3,sumsnow4,sumsnow10,taccum,taccum5,d_gs,d_fet;
  float		meantphys,tb1836vf_10daytot,tb1836hf_10daytot,tb1836vf_10daysmth,tb1836hf_10daysmth,tb1836v_store[6],tb1836h_store[6];
  int 		nds,nd_obs10,dt,nd_snow5,d_evol,nd_obs5;
  double	deltadays;
  int		d_reg,d_gph,dsnowdj_flag,dsnowdj;
  int		xysc;
  int 		row,col,rflag;
  float		ctemp,aveswe,zc,bias;
  float		gszero=0.2;
  float		gsinfinity=0.98;
  float		alpha=0.02;
  float		etgr=0.0001;

  float		volgr;
  float		freshsnowden=0.09067;	/* Based on Hedstrom and Pomeroy (1998). ps=67.92+51.25exp(Ta/2.59) kg/m^3  assumes Ta = -2C */
  float		initden,maxden,snowden;
  float		Tb18V_est,Tb36V_est,Tb18H_est,Tb36H_est;
  int		sdscale=100;
  int		swescale=10;
  float		minSD,maxSD,iSD,dSD,sd0,t1836vf_est,t1836hf_est,minDelta,Delta,nincr=15.;
  float		SD,SWE; 
  int		Ld,Lg,Lt,Ls,ipos;
  float		LUT_minD=0.2, LUT_minG=0.1, LUT_minT=225.0,LUT_minS=0.01;
  float		LUT_incrD=0.02, LUT_incrG=0.02, LUT_incrT=2.0,LUT_incrS=0.01;
  float		soilDtbOffset=3.0;	/* Accounts for the fact that the DMRTML was for an infinite background and no stickiness */

  float		tmpv1836,tmph1836,tmpv1836EST,tmph1836EST;
  float		pi=3.141592654;
  float		mean6_v,mean6_h,sdstdev6_v,sdstdev6_h,value1,value2,xtmpv,xtmph,xtmpv2,xtmph2,wgtv6[6],wgth6[6],waccumv,waccumh,sclwgtv6,sclwgth6,toth,totv;
  int		frstges_Lt,xi;
  float		frstges_dTb,frstges_diff,sweeprange;

  float		invlogpol18f,invlogpol36f;

		/* Sturm Classes Tundra, Taiga,  Maritime, Prairie,  Alpine    ....for density g/cm^3*/
  float		denminmax[2][5]={0.3630, 0.4785, 0.5979,   0.5940,   0.5975,	/* Maximum density */
				 0.2425, 0.2379, 0.2578,   0.2332,   0.2237};	/* Initial density */

  row=*easey;
  col=*easex;
  rflag=*retrievalflag;

/* GET INITIAL FACTORS FOR V1 SD ESTIMATES IF NEEDED */
  invlogpol18f=1./(log10(pol18f));
  invlogpol36f=1./(log10(pol36f));


/*--------------------------------*/
/*** GRAIN SIZE MODEL - PRELIMS****/
/*--------------------------------*/

/*  Get TPhys Data  */
  tphys=dysta_tphys[row][col][0]/dysta_tphys[row][col][1];

/* Calculate temp grad */
  tdiff= tphys - 273.15;
  tgrad=tdiff;

/* If snow present (ref > 0) do retrieval */
  if (rflag > 0 && rflag < 41) {		/*IF RETRIEVAL FLAG GREATER THAN 0 (SNOW PRESENT) DO RETRIEVAL */

/*------------------------------*/
/* Snow is Present: UPDATE DETECTION ARRAYS  */
  scahistory[row][col][0]=*Julian;
  thistory[row][col][0]=*Julian;

/* Initialise gs array value if it does not exist from yesterday */
  if (gs[row][col] == 0) gs[row][col]=gszero;
/* These are dummy variables that will not get used unless a retrieval is called - rflag = 1-5 */
  minSD=0.0;		/* Dummy */
  maxSD=0.1;		/* Dummy */
  sweeprange=0.0001;	/* Dummy */
  d_gs=gszero;		/* Real */
  d_reg=2;		/* Real */
  maxden=0.1;		/* Dummy */
  initden=0.005;	/* Dummy */
  volgr=0.0001;		/* Dummy */

/*-----------------------------*/
/* CONDITION: 1. Test to see how many days in the previous 10 time slots had observations */
/* Calculate the average temperature for the previous 10 days */
/* dt=0 is today, dt=1 is yesterday and dt=10 is 11 days ago for scahistory, t_est and thistory - [11] stores julian time */
/* dt=0 is yesterday, dt=9 is 10 days ago for sdhistory and other arrays with [10] time stores */
/* nds is the number of days in the 10-slot storage array with snow */
/* nd_snow5 is the number of observations of snow in the most recent five day period */
/* nd_obs10 is the number of temperature (klvn) estimates in a 10 slot storage array */

  deltadays=*Julian-thistory[row][col][1];		/* Number of days since last Tphys*/
  nd_snow5=0;
  nd_obs10=0;
  nd_obs5=0;
  dsnowdj=0;
  dsnowdj_flag=0;
  nds=0;
  taccum=0;
  taccum5=0;
  tb1836vf_10daytot=t1836vf;
  tb1836hf_10daytot=t1836hf;
  tb1836h_store[0]=t1836hf;
  tb1836v_store[0]=t1836vf;
  *EstSnowdepth=0;
  *EstSnowWE=0;

    for (dt=0; dt<11; dt++){
 	if (t_est[row][col][dt] > 0.) {
	  nd_obs10=nd_obs10+1; 		  		/* count the number of temp estimates in the last 10 days */
          taccum=taccum+t_est[row][col][dt];  		/* accumulate for average recent temperature */
	}
 	if (t_est[row][col][dt] > 0. && dt < 6) {
	  nd_obs5=nd_obs5+1; 		  		/* count the number of temp estimates in the last 5 days */
          taccum5=taccum5+t_est[row][col][dt];  		/* accumulate for average recent temperature */
	}
        if (dt > 0 && dt < 6 && scahistory[row][col][dt] > 0.) {
	  nd_snow5=nd_snow5+1; 	 			/* count the number of times there was snow in the storage places */
          if (dsnowdj_flag == 0) {			/* store the number of days back to last snow obs within the 5 days - look for first instance*/
	     dsnowdj=dt;
	     dsnowdj_flag =1;
	  }
	  tb1836vf_10daytot=tb1836vf_10daytot+tb1836vf_smth[row][col][dt-1]/10.;	/* Accumulate up to 10 previous days of dTb (18-36V) */
	  tb1836hf_10daytot=tb1836hf_10daytot+tb1836hf_smth[row][col][dt-1]/10.;	/* Accumulate up to 10 previous days of dTb (18-36H) */
  	  tb1836h_store[dt]=tb1836hf_smth[row][col][dt-1]/10.;
  	  tb1836v_store[dt]=tb1836vf_smth[row][col][dt-1]/10.;
	  nds=nd_snow5;
	}
    }

/* Get recent 10+1 days temperature Gradient and number of days with snow*/
  /* Case 1. Fresh snow and no recent obs*/
  if (nd_obs10 == 0) {
     /* get mean tgrad */
     meantgrad=tphys-273.15;
     meantphys=tphys;
  } 
  /* Case 2. There are recent obs*/
  else if (nd_obs10 > 0) {
     /* get mean tgrad */
     meantgrad=(taccum/(float)nd_obs10)-273.15;
     meantphys=(taccum/(float)nd_obs10);
  }

/* Same as above but get 5 day temp gradient for gs trajectory */
  if (nd_obs5 == 0) {
     /* get mean tgrad (5 days) */
     meantgrad5=tphys-273.15;
  } 
  else if (nd_obs5 > 0) {
     /* get mean tgrad */
     meantgrad5=(taccum5/(float)nd_obs5)-273.15;
  }
  
  if (debug_retr == 1) printf("DEBUG   meantgrad, tphys, nds, nd_obs10  taccum %f %f %f %f %d %d\n",tphys,meantphys,taccum,meantgrad,nds,nd_obs10);

/*** GRAIN SIZE MODEL - EVOLUTION  - HOW MANY DAYS OF CONTINUOUS SNOW? ****/
  if (rflag == 6) nds = 0; 	/* ENSURE NDS IS ZERO IF USING PERSISTENCE */
  if (rflag == 6 && persistence[row][col][1] > 0 || nds > 0.) {
    if (dysta_evol[row][col] == 0) {
      d_evol=evolution[row][col]+dsnowdj;
      dysta_evol[row][col] =1;
    }
    else d_evol=evolution[row][col];
  }
  else d_evol=0;
  evolution[row][col]=d_evol;

  if (debug_retr == 1) printf("DEBUG   d_evol  %f \n",d_evol);

/*******************************/
/* A. DO PERSISTENCE RETRIEVAL */
/*******************************/
  if (rflag == 6) {
     SD=sdhistory[row][col][0]/1000.;
     snowden=denest[row][col];
  }

/***********************************/
/* B. DO FULL ON DMRT-ML RETRIEVAL */
/***********************************/
  if (rflag < 6) {


/*** GRAIN SIZE MODEL - GROWTH REGIME BASED ON HOW LONG SNOW HAS BEEN AROUND AND HOW MANY DAYS BELOW -10C ****/
/* regime=1 (fresh snow)
   regime=2 (EQ growth phase snow)
   regime=3 (Kinetic/facet growth phase snow) */
   d_reg=2;
   if (d_evol< 2) d_reg=1;
   if (meantgrad5 <= -5. && d_evol > 4)d_reg=3;		/* 29/10/2016 Changed 'meantgrad5 <= -10. && d_evol > 4' to 'meantgrad5 <= -5. && d_evol > 4' */
   dysta_reg[row][col][d_reg-1]=dysta_reg[row][col][d_reg-1]+1;
   dysta_reg[row][col][1]=dysta_reg[row][col][1]+1;

/*** GRAIN SIZE MODEL - GROWTH PHASE HISTORY (GPH) 1= NEW REGIME, >1 = SAME REGIME****/
   d_gph=1;
   if (d_reg == regime[row][col]) d_gph=regime[row][col]+1;

/*** GRAIN SIZE MODEL - FACET EVOLUTION TIME (FET) - CALCULATES HOW MANY DAYS UNDER KINETIC GROWTH FROM THE START ***/
   if (d_reg == 3){
     d_fet=fet[row][col]+dsnowdj;	/* NO CHANGE IN REGIME: SET FET TO TIME FROM LAST SNOW - <dsnowdj> */
     if (d_gph ==1)d_fet=1+(log((gs[row][col]-gsinfinity)/(gszero-gsinfinity)))/(-1*alpha); /* CHANGE IN REGIME: CALCULATE HOW FAR INTO KINETIC GROWTH PHASE WE MIGHT BE */
     dysta_fet[row][col][0]=dysta_fet[row][col][0]+d_fet;
     dysta_fet[row][col][1]=dysta_fet[row][col][1]+1;
   }

/*** GRAIN SIZE MODEL - GRAIN SIZE ESTIMATE ***/
   d_gs=gszero;	
   if (nds > 0) {
     if (d_reg == 1 && d_gph < 10) d_gs=gszero;						/* Regime 1: GSZERO */
     if (d_reg == 2) d_gs=gs[row][col]+etgr;						/* Regime 2: (EQT GROWTH) is etgr */
     if (d_reg == 3) d_gs=gsinfinity-(gsinfinity-gszero)*exp(-1*alpha*d_fet);		/* Regime 3: (Kinetic GROWTH) is Sturm & Benson (1997) */
     dysta_gs[row][col][0]=dysta_gs[row][col][0]+d_gs;
     dysta_gs[row][col][1]=dysta_gs[row][col][1]+1;
   }
/* FOR DEBUGGING */
  if (debug_retr == 1 && row == 534 && col == 292) {
    printf("DEBUG   rflag, tphys,tgrad, nd_obs10,nd_snow5 %d %f %f %d %d\n",rflag,tphys,tgrad,nd_obs10,nd_snow5);
    printf("DEBUG   *Julian scahistory[row][col][1] deltadays dsnowdj nds meantgrad  %f %f %f  %f %d %f \n",*Julian,scahistory[row][col][1],deltadays,dsnowdj,nds,meantgrad);
    printf("DEBUG   d_evol d_reg d_gph d_fet d_gs %d %d %d %f %f \n\n",d_evol,d_reg,d_gph,d_fet,d_gs);
  }

/*--------------------------------*/
/*** SNOW DENSITY MODEL ****/
/*--------------------------------*/
/*COMPUTE SNOW DENSITY BASED ON TIME FROM INITIAL SNOW
  SNOW DENSITY CHANGES WITH SEASONAL SNOW CLASS AND AGE OF SNOW 
  (Sturm,  M.,  B.  Taras,  G.  Liston,  C. Derksen,  T.  Jonas,  and  J.  Lea.  2010.  Estimating  snow  water equivalent using snow  depth data and climate  classes. Journal of Hydrometeorpwdology. 11: 1380 -1394. */
   xysc=snowclass[row][col];
   initden=denminmax[1][2];	/* COVER ALL EVENTUALITIES AND SET TO MARITIME */
   maxden=denminmax[0][2];

   if (xysc == 1) {		/* SET MIN/MAX DENSITY TO TUNDRA */
     initden=denminmax[1][0];
     maxden=denminmax[0][0];
     volgr=0.007;
   }
   if (xysc == 2) {		/* SET MIN/MAX DENSITY TO TAIGA */
     initden=denminmax[1][1];
     maxden=denminmax[0][1];
     volgr=0.005;
   }
   if (xysc == 4 || xysc == 5) {	/* SET MIN/MAX DENSITY TO MARITIME */
     initden=denminmax[1][2];
     maxden=denminmax[0][2];
     volgr=0.007;
   }
   if (xysc == 6) {		/* SET MIN/MAX DENSITY TO PRAIRIE */
     initden=denminmax[1][3];
     maxden=denminmax[0][3];
     volgr=0.003;
   }
   if (xysc == 7) {		/* SET MIN/MAX DENSITY TO ALPINE */
     initden=denminmax[1][4];
     maxden=denminmax[0][4];
     volgr=0.005;
   }

/*COMPUTE SNOW DENSITY - ESSENTIALLY A LOGISTIC CURVE LIKE THE GRAIN GROWTH CURVE*/
   if (d_evol < 5) {
     snowden=freshsnowden+(((initden-freshsnowden)/5)*(d_evol));
   }
   else snowden=maxden-(maxden-initden)*exp(-1*volgr*(d_evol-4));

   snowden=0.3;			/* Let's use a density of 0.3 g/cm^3 for argument 30/10/2016  */

   dysta_den[row][col][0]=dysta_den[row][col][0]+snowden;
   dysta_den[row][col][1]=dysta_den[row][col][1]+1;

/*DMRT-ML RETRIEVAL: USE COST FUNCTION TO CONDUCT RETRIEVAL*/
/*G. Picard, L. Brucker, A. Roy, F. Dupont, M. Fily, and A. Royer, Simulation of the microwave emission of multi-layered snowpacks using the dense media radiative transfer theory: the DMRT-ML model, Geoscientific Model Development, 6, 1061-1078, 2013, doi:10.5194/gmd-6-1061-2013 */

  if (debug_retr ==1)  first_time=get_posix_clock_time (); 		// GET START TIME

/* rflag 42: System and Geophysical Screens Rule out Retrieval */
/* rflag 41: land no snow */
/* rflag  1: snow (V-Pol) */
/* rflag  2: snow (H-Pol) */
/* rflag  3: snow high latitude*/
/* rflag  4: shallow snow */
/* rflag  5: Wet snow */
/* rflag  6: Dry snow persistence*/

/* Prelims */
/* Get the 6 day weighted average Tb18V-Tb36V and Tb18H-Tb36H differences */
/* Use a gaussian weighted value */

  /* Case 1. Fresh snow and no recent obs*/
  if (nds == 0) {
    tb1836vf_10daysmth=t1836vf;
    tb1836hf_10daysmth=t1836hf;
  } 
  /* Case 2. There are recent obs*/
  else if (nds < 6) {
     tb1836hf_10daysmth=(tb1836hf_10daytot)/((float)nds+1.0);	/* Get the arithmetic average Tb18-Tb36 H difference for the previous 5 days of snow */
     tb1836vf_10daysmth=(tb1836vf_10daytot)/((float)nds+1.0);	/* Get the arithmetic average Tb18-Tb36 V difference for the previous 5 days of snow */
  }
  else if (nds == 6) {
  /* Get Standard Deviation of Tb18-Tb36 H/V data for t=0  ==> t=-5*/
    mean6_v=(tb1836vf_10daytot)/((float)nds+1.0);
    mean6_h=(tb1836hf_10daytot)/((float)nds+1.0);
    sdstdev6_v=0.;
    sdstdev6_h=0.;
    value1=0;
    value2=0;
    for (xi=0; xi<6; xi++){
       value1=fabs(mean6_v-tb1836v_store[xi]);
       sdstdev6_v=sdstdev6_v+pow(value1,2.);
       value2=fabs(mean6_h-tb1836h_store[xi]);
       sdstdev6_h=sdstdev6_h+pow(value2,2.);
    }
    if(sdstdev6_v > 0.) sdstdev6_v=pow( ((sdstdev6_v)/((float)nds+1.0)) ,0.5);
    if(sdstdev6_h > 0.) sdstdev6_h=pow( ((sdstdev6_h)/((float)nds+1.0)) ,0.5);

  /* Get Weightings*/
    totv=0.;
    toth=0.;
    for (xi=0; xi<6; xi++){
      xtmpv=-2.*(((float)xi*(float)xi)/(pow(sdstdev6_v,2.)) );
      xtmph=-2.*(((float)xi*(float)xi)/(pow(sdstdev6_h,2.)));
      xtmpv2=sdstdev6_v*(pow((0.5*pi),0.5));
      xtmph2=sdstdev6_h*(pow((0.5*pi),0.5));
      wgtv6[xi]=(1/(xtmpv2)) * exp(xtmpv);
      wgth6[xi]=(1/(xtmph2)) * exp(xtmph);
      totv=totv+wgtv6[xi];
      toth=toth+wgth6[xi];
    }

  /* Cal Gaussian Weighted Average Tb18-Tb36 V/H */
    waccumv=0.;
    waccumh=0.;
    for (xi=0; xi<6; xi++){
      sclwgtv6=wgtv6[xi]/totv;
      waccumv=waccumv+(sclwgtv6*tb1836v_store[xi]);
      sclwgth6=wgth6[xi]/toth;
      waccumh=waccumh+(sclwgth6*tb1836h_store[xi]);
//      print,wgt5(xi),sclwgt5,waccum,series(i-xi);
    }
    tb1836vf_10daysmth=waccumv;
    tb1836hf_10daysmth=waccumh;
  }

/* ** LUT MINIMIZATION ROUTINE ** */
/* In last 5 days there was no snow ) */
//  if (sdhistory[row][col][0]/1000. == 0){   /* Update 21 July 2016 */
  if (nds == 0){
    if (rflag == 4) SD=minsnowlimit;		/*Set FRESH SNOW TO 0.02 m */
    if (rflag == 5) SD=minsnowlimit;		/*Set FRESH SNOW TO 0.02 m */
    if (rflag == 1 || rflag == 2 || rflag == 3){
/* LOCATE POSITION IN DMRT-ML LUT OF GS, DENSITY, Tphys */
      Ld=(snowden-LUT_minD)/LUT_incrD;
      if (snowden < LUT_minD) Ld = 0;
      Lg=(d_gs-LUT_minG)/LUT_incrG;
      if (Lg < LUT_minG) Lg=0;
      Lt=(meantphys-LUT_minT)/LUT_incrT;
      if (meantphys < LUT_minT) Lt=0;
 /* FIRST GUESS IS FROM Kelly 2009 SD = invlogpol36f*t1836vf/100 cm*/     
      if (rflag == 1 || rflag == 3) sd0=invlogpol36f*t1836vf/100.;
      if (rflag == 2) sd0=invlogpol36f*t1836hf/100.;
      if (sd0 < minsnowlimit) sd0=minsnowlimit;
//      if (sd0 > 0.2) sd0=0.2;

      frstges_Lt=(sd0-LUT_minS)/LUT_incrS;
      if (rflag == 2){
         frstges_dTb=soilDtbOffset+DMRT_LUT[Lt][Ld][Lg][frstges_Lt][1]/10.;
         frstges_diff=fabsf(frstges_dTb-tb1836hf_10daysmth);
      }
      if (rflag == 1 || rflag == 3){
         frstges_dTb=soilDtbOffset+DMRT_LUT[Lt][Ld][Lg][frstges_Lt][0]/10.;
         frstges_diff=fabsf(frstges_dTb-tb1836vf_10daysmth);
      }
      sweeprange=(1.0/(0.4-(0.4-0.2)*exp(-1.0*frstges_diff)))/100.;
      if (sweeprange < 0.025) sweeprange = 0.025;
      if (sweeprange > 0.05) sweeprange = 0.05;

      minSD= (sd0-(0.10*sweeprange)) > minsnowlimit ? sd0-(0.10*sweeprange) : minsnowlimit;	/* change range from (minSD - 0.25*sweeprange) to (minSD-0.10*sweeprange)  */
      maxSD= (sd0+(0.90*sweeprange));								/* change range from (minSD + 0.75*sweeprange) to (minSD-0.90*sweeprange)  */

      if (maxSD > 1.98) maxSD = 1.98;
      dSD=(maxSD-minSD)/nincr;
      minDelta=999.0;
      ipos=0;
      for (iSD=minSD; iSD<maxSD; iSD=iSD+dSD){
        Ls=(iSD-LUT_minS)/LUT_incrS;
        if (rflag == 2){
          t1836hf_est=soilDtbOffset+DMRT_LUT[Lt][Ld][Lg][Ls][1]/10.;		/* LUT Data stored as (Tb18H-Tb36H) *10 */
          Delta=fabsf(t1836hf_est-tb1836hf_10daysmth);
	}
        if (rflag == 1 || rflag == 3){
          t1836vf_est=soilDtbOffset+DMRT_LUT[Lt][Ld][Lg][Ls][0]/10.;		/* LUT Data stored as (Tb18V-Tb36V) *10 */
          Delta=fabsf(t1836vf_est-tb1836vf_10daysmth);
	}
        if (Delta < minDelta){
          minDelta=Delta;
          SD=iSD;
	  ipos=ipos+1;
	  tmpv1836=t1836vf;
	  tmph1836=t1836hf;
	  tmpv1836EST=t1836vf_est;
	  tmph1836EST=t1836hf_est;
        }
      }
    }
  }

/* Previous 5 days had SNOW) */
//  if (sdhistory[row][col][0]/1000. > 0){		Edit Out 21 July
  if (nds > 0){
     if (rflag == 5 | rflag == 4) SD=sdhistory[row][col][0]/1000.;
     if (rflag == 1 || rflag == 2 || rflag == 3 ){
/* LOCATE ADDRESSES  IN DMRT-ML LUT OF GS, DENSITY, Tphys */
       Ld=(snowden-LUT_minD)/LUT_incrD;
       if (snowden < LUT_minD) Ld = 0;
       Lg=(d_gs-LUT_minG)/LUT_incrG;
       if (Lg < LUT_minG) Lg=0;
       Lt=(meantphys-LUT_minT)/LUT_incrT;
       if (meantphys < LUT_minT) Lt=0;
       sd0=sdhistory[row][col][0]/1000.;
       if (sd0 < minsnowlimit) sd0=minsnowlimit;

       frstges_Lt=(sd0-LUT_minS)/LUT_incrS;
       if (rflag == 2){
          frstges_dTb=soilDtbOffset+DMRT_LUT[Lt][Ld][Lg][frstges_Lt][1]/10.;
          frstges_diff=fabsf(frstges_dTb-tb1836hf_10daysmth);
       }
       if (rflag == 1 || rflag == 3){
          frstges_dTb=soilDtbOffset+DMRT_LUT[Lt][Ld][Lg][frstges_Lt][0]/10.;
          frstges_diff=fabsf(frstges_dTb-tb1836vf_10daysmth);
       }
       sweeprange=(1.0/(0.4-(0.4-0.2)*exp(-1.0*frstges_diff)))/100.;
       if (sweeprange < 0.025) sweeprange = 0.025;
       if (sweeprange > 0.05) sweeprange = 0.05;

      minSD= (sd0-(0.10*sweeprange)) > minsnowlimit ? sd0-(0.10*sweeprange) : minsnowlimit;	/* change range from (minSD - 0.25*sweeprange) to (minSD-0.10*sweeprange)  */
      maxSD= (sd0+(0.90*sweeprange));								/* change range from (minSD + 0.75*sweeprange) to (minSD-0.90*sweeprange)  */
 
       if (maxSD > 1.98) maxSD = 1.98;
       dSD=(maxSD-minSD)/nincr;
       minDelta=999.0;
       ipos=0;
       for (iSD=minSD; iSD<maxSD; iSD=iSD+dSD){
         Ls=(iSD-LUT_minS)/LUT_incrS;
        if (rflag == 2){
          t1836hf_est=soilDtbOffset+DMRT_LUT[Lt][Ld][Lg][Ls][1]/10.;		/* LUT Data stored as (Tb18H-Tb36H) *10 */
          Delta=fabsf(t1836hf_est-tb1836hf_10daysmth);
	}
        if (rflag == 1 || rflag == 3){
          t1836vf_est=soilDtbOffset+DMRT_LUT[Lt][Ld][Lg][Ls][0]/10.;		/* LUT Data stored as (Tb18V-Tb36V) *10 */
          Delta=fabsf(t1836vf_est-tb1836vf_10daysmth);
	}
        if (Delta < minDelta){
           minDelta=Delta;
           SD=iSD;
	   ipos=ipos+1;
 	   tmpv1836=t1836vf;
	   tmph1836=t1836hf;
	   tmpv1836EST=t1836vf_est;
	   tmph1836EST=t1836hf_est;
        }
       }
     }
   }
  
  if (debug_retr ==1) {		// GET END TIME
	second_time=get_posix_clock_time ();
	diff_time=second_time-first_time;
	printf("Row Col Rflag %d %d %d Computation time (in microseconds): %jd \n",row,col,rflag,diff_time);
  }				//^^^^^^^^^^^


  }
  SWE=SD*snowden*1000;		/* SD is in m, SWE is in mm | SD and SWE are stored in sdhistory */
  *EstSnowdepth=SD*sdscale;			/* this goes to L2 HDF data SD is stored as cm*10 */ 
  *EstSnowWE=SWE*swescale;			/* this goes to L2 HDF data SWE is stored as mm*10 */

/* Update daily Stats Files */
  dysta_swe[row][col][0]=dysta_swe[row][col][0]+SWE;
  dysta_swe[row][col][1]=dysta_swe[row][col][1]+1;
  dysta_sd[row][col][0]=dysta_sd[row][col][0]+SD;
  dysta_sd[row][col][1]=dysta_sd[row][col][1]+1;
  dysta_1836v[row][col][0]=dysta_1836v[row][col][0]+tb1836vf_10daysmth;
  dysta_1836v[row][col][1]=dysta_1836v[row][col][1]+1;
  dysta_1836h[row][col][0]=dysta_1836h[row][col][0]+tb1836hf_10daysmth;
  dysta_1836h[row][col][1]=dysta_1836h[row][col][1]+1;

  if (debug_retr ==1) {
	if (row == 952 && col == 938) printf("\nrflag, SWE, SD ...   %1d %6.4f %6.4f %6.4f %6.4f %6.4f %6.4f %2d %4d \n",rflag,SWE, SD, minSD,maxSD,sd0,sweeprange,ipos-1,sdhistory[row][col][0]);
	if (row == 952 && col == 938) printf("Tbs...                 %5.1f %5.1f %5.1f %5.1f %5.1f %5.1f %5.1f %5.1f %5.2f %5.2f %5.2f  %5.1f %5.1f %5.1f \n",v18f, h18f,v23f, h23f, v36f, h36f, 			v89f,h89f,t1836vf,t1836vf_est,tb1836vf_10daysmth,meantphys,meantgrad5,tphys);
	if (row == 952 && col == 938) printf("Times ...         %2d %7.2f %10.1f %10.1f %2d \n",nds,deltadays,*Julian,scahistory[row][col][0],dsnowdj);
	if (row == 952 && col == 938) printf("Model params...   %2d %3d %7.5f %7.5f \n",d_reg,d_evol,d_gs,snowden);
	if (row == 952 && col == 938) printf("Density params...   %3d %6.4f %7.5f %7.5f %7.5f \n\n",d_evol,maxden,initden,volgr,snowden);
  }

  }		/* END OF SNOW RETRIEVAL */

  if (rflag == 41){		/*IF RETRIEVAL FLAG EQUALS 0 THEN SNOW NOT PRESENT - NO RETRIEVAL - UPDATE SCAHISTORY etc.*/
    scahistory[row][col][0]=0;
    thistory[row][col][0]=*Julian;
    gs[row][col]=0.;   
    denest[row][col]=0.;
    evolution[row][col]=0.;
  }


}

