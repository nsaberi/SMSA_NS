// Calculate EASE-X and EASE-Y COORDINATE
// References:

//  Brodzik, M. J., B. Billingsley, T. Haran, B. Raup, M. H. Savoie. 2012. EASE-Grid 2.0: Incremental but Significant Improvements for Earth-Gridded Data Sets. ISPRS International Journal of Geo-Information, 1(1):32-45, doi:10.3390/ijgi1010032. http://www.mdpi.com/2220-9964/1/1/32/.

//  Brodzik, M. J., B. Billingsley, T. Haran, B. Raup, M. H. Savoie. 2014. Correction: Brodzik, M. J. et al. EASE-Grid 2.0: Incremental but Significant Improvements for Earth-Gridded Data Sets. ISPRS International Journal of Geo-Information 2012, 1, 32-45. ISPRS International Journal of Geo-Information, 3(3):1154-1156, doi:10.3390/ijgi3031154. http://www.mdpi.com/2220-9964/3/3/1154/

#include <stdio.h>
#include <math.h>

void lltoease2(float *plat, float *plon)
{
    /* local variable declaration */
    float ylat,xlon;
    int grid;
    
// Set up map paremeters: using the EASE Grid2.0 12.5km Grid
    float map_scale_m= 12500.0;
    long cols= 1440;
    long rows= 1440;
    float r0= 719.5;
    float s0= 719.5;
    // epsilon for test in neighborhood of pole for azimuthal projections
    double epsilon = 1.0E-06;
    // WGS84 parameter
    double map_equatorial_radius_m = 6378137.0 ;
    double map_eccentricity = 0.081819190843 ;
    // Map variables
    double e2 = pow(map_eccentricity,2.0);
    double map_reference_latitude,map_reference_longitude;
    double dlon, phi, rho,lam,sin_phi, pi=4*atan(1.0),q,qp;
    double x,y,r,s;
    
    ylat=*plat;
    xlon=*plon;
    
    /* grid=1 (N. Hemisphere), grid=2 (S. Hemisphere) */
    grid=1 ;
    if (ylat < 0.0 ) grid=2;
    
    /* Set up map tie point */
    if (grid == 1){
        map_reference_latitude = 90.0;
        map_reference_longitude = 0.0;
    }
    if (grid == 2){
        map_reference_latitude = -90.0;
        map_reference_longitude = 0.0;
    }
    
    dlon = xlon - map_reference_longitude;
    //normalize longitude (-180 to +180)
    if (xlon < -180.) dlon=xlon+360. ;
    if (xlon > 180.0) dlon=xlon-360. ;
    //Convert deg 2 radians
    phi =  ylat * pi / 180.0 ;
    lam =  dlon * pi / 180.0 ;
    sin_phi = sin( phi );
    
    // Projection variables
    q = ( 1.0 - e2 ) * ( ( sin_phi / ( 1.0 - e2 * sin_phi * sin_phi ) )
                        - ( 1.0 / ( 2.0 * map_eccentricity ) )
                         * log( ( 1.0 - map_eccentricity * sin_phi )
                                / ( 1.0 + map_eccentricity * sin_phi ) ) );
    
    qp = 1.0 - ( ( 1.0 - e2 ) / ( 2.0 * map_eccentricity )
                 * log( ( 1.0 - map_eccentricity )
                        / ( 1.0 + map_eccentricity ) ) );
    //Calculate arcs
    if (grid == 1){
        if ( fabs( qp - q ) < epsilon )
            rho = 0.0;
        else
            rho = map_equatorial_radius_m * pow((qp-q),0.5);
        x = rho * sin( lam );
        y = -1.0 * rho * cos( lam );
    }
    if (grid == 2){
        if ( fabs( qp + q ) < epsilon )
            rho = 0.0;
        else
            rho = map_equatorial_radius_m * pow((qp+q),0.5);
        x = rho * sin( lam );
        y = rho * cos( lam );
    }
    
    // Convert xy coordinates to row/col (r,s) for this particular grid and pass back to main program
    *plon = r0 + ( x / map_scale_m );
    *plat = s0 - ( y / map_scale_m );


}
