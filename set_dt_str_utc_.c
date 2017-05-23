/*
//
// include
//
*/
#include <stdio.h>
#include <time.h>
/*
//
// set date time string by utc
//
*/

void set_dt_str_utc_(char *buf){
        /*
	//                         1         2
	//                123456789 123456789 123
	// output sample: 2010/10/06 11:24:03 UTC
	
	// BY FORTRAN
	// buf is character*23 buf
	
	// BY C
	// buf is char buf[24] and buf[23]='\0'
	*/
	/* get current time */
	time_t ttcu; /* time_t for current time */
	ttcu = time(NULL);
	struct tm *stmp; /* struct tm pointer */
	stmp = gmtime(&ttcu);
	
	/* make string */
	char str[24];

	sprintf(str, "%04d/%02d/%02d %02d:%02d:%02d UTC",
	stmp->tm_year+1900,
	stmp->tm_mon+1,
	stmp->tm_mday,
	stmp->tm_hour,
	stmp->tm_min,
	stmp->tm_sec);

	/* copy */
	int i;
	for(i = 0; i < 23; ++i){
		*(buf + i) = str[i];
	}
}

void set_dt_str_utc(char *buf){
        /*
        //                         1         2
        //                123456789 123456789 123
        // output sample: 2010/10/06 11:24:03 UTC

        // BY FORTRAN
        // buf is character*23 buf

        // BY C
        // buf is char buf[24] and buf[23]='\0'
        */
        /* get current time */
        time_t ttcu; /* time_t for current time */
        ttcu = time(NULL);
        struct tm *stmp; /* struct tm pointer */
        stmp = gmtime(&ttcu);

        /* make string */
        char str[24];
        sprintf(str, "%04d/%02d/%02d %02d:%02d:%02d UTC",
        stmp->tm_year+1900,
        stmp->tm_mon+1,
        stmp->tm_mday,
        stmp->tm_hour,
        stmp->tm_min,
        stmp->tm_sec);
        /* copy */
        int i;
        for(i = 0; i < 23; ++i){
                *(buf + i) = str[i];
        }

        buf[23]='\0';
}
