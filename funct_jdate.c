/*function returning the julian date: input year month date */
int jdate(int num1, int num2, int num3) 
{
   /* local variable declaration */
   long result;
   float a,y,m;

   a=(14-num2)/12.;
   y=num1+4800.-a;
   m=num2+12.*a-3.;
   result=num3 + (153 * m + 2)/5 + 365*y + y/4 - y/100 + y/400 - 32045 - 0.5;
 
   return result; 
}


