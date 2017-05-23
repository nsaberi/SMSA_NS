#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void error_end(int ecd, char* spc)
{
  char bas[256] = {'\0'};
  char cts[24]  = {'\0'};
  char ecs[512] = {'\0'};
  /*
    set base error message
  */


  // error end return value (with retry) :  51 - 150
  // error end return value (no retry)   : 151 - 250

  //  51 -  80 : input interface error
  if(ecd == 51)strcpy(bas, "work order error");
  if(ecd == 52)strcpy(bas, "L1B input error");
  if(ecd == 53)strcpy(bas, "L1R input error");
  if(ecd == 54)strcpy(bas, "L2 input error");
  if(ecd == 55)strcpy(bas, "L3 input error");
  if(ecd == 56)strcpy(bas, "GANAL input error");
  if(ecd == 57)strcpy(bas, "GRJTD input error");
  if(ecd == 58)strcpy(bas, "MGDSST input error");
  if(ecd == 59)strcpy(bas, "tie point file error");
  if(ecd == 60)strcpy(bas, "LUT error");

  //  81 -  90 : output interface error
  if(ecd == 81)strcpy(bas, "L2 output error");
  if(ecd == 82)strcpy(bas, "L3 output error");
  if(ecd == 83)strcpy(bas, "log output error");

  //  91 - 100 : internal interface error
  if(ecd == 91)strcpy(bas, "internal file reference error");
  if(ecd == 92)strcpy(bas, "algorithm status error");

  // 101 - 150 : other error
  if(ecd == 101)strcpy(bas, "system error");
  if(ecd == 102)strcpy(bas, "module error");

  // set ecs
  sprintf(ecs, "(%3d)", ecd);

  // message
  set_dt_str_utc_(cts);

  fprintf(stderr, "%s %s %s %s\n", cts, bas, ecs, spc);

  exit(ecd);

}
