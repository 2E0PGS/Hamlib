
/* 
 * Very simple test program to check locator convertion against some other --SF
 * This is mainly to test longlat2locator and locator2longlat functions.
 *
 * Takes at least one argument, which is a locator. 
 * If two locators are given, then the qrb is also calculated.
 */

#include <stdio.h>
#include <stdlib.h>
#include <hamlib/rotator.h>


int main (int argc, char *argv[])
{
	char recodedloc[8], *loc1, *loc2;
	double lon1, lat1, lon2, lat2;
	double distance, az;
	int deg, min, sec;
	int retcode;

	if (argc < 2) {
			fprintf(stderr,"Usage: %s <locator1> [<locator2>]\n", argv[0]);
			exit(1);
	}

	loc1 = argv[1];
	loc2 = argc > 2 ? argv[2] : NULL;

	printf("Locator1: %s\n", loc1);
	retcode = locator2longlat(&lon1, &lat1, loc1);

	dec2dms(lon1, &deg, &min, &sec);
	printf("  Longitude:   %f, %d� %d' %d\"\n", lon1, deg, min, sec);
	lon1 = dms2dec(deg, min, sec);
	printf("  Recoded lon: %f\n", lon1);

	dec2dms(lat1, &deg, &min, &sec);
	printf("  Latitude:    %f, %d� %d' %d\"\n", lat1, deg, min, sec);
	lat1 = dms2dec(deg, min, sec);
	printf("  Recoded lat: %f\n", lat1);

	longlat2locator(lon1, lat1, recodedloc);
	recodedloc[6] = '\0';
	printf("  Recoded: %s\n", recodedloc);

	if (loc2 == NULL)
			exit(0);

	printf("\nLocator2: %s\n", loc2);
	retcode = locator2longlat(&lon2, &lat2, loc2);

	dec2dms(lon2, &deg, &min, &sec);
	printf("  Longitude:   %f, %d� %d' %d\"\n", lon2, deg, min, sec);
	lon2 = dms2dec(deg, min, sec);
	printf("  Recoded lon: %f\n", lon2);

	dec2dms(lat2, &deg, &min, &sec);
	printf("  Latitude:    %f, %d� %d' %d\"\n", lat2, deg, min, sec);
	lat2 = dms2dec(deg, min, sec);
	printf("  Recoded lat: %f\n", lat2);

	longlat2locator(lon2, lat2, recodedloc);
	recodedloc[6] = '\0';
	printf("  Recoded: %s\n", recodedloc);

	retcode = qrb(lon1, lat1, lon2, lat2, &distance, &az);
	if (retcode != 0) {
			fprintf(stderr, "QRB error: %d\n", retcode);
			exit(2);
	}
	dec2dms(az, &deg, &min, &sec);
	printf("\nDistance: %.2fkm\n", distance);
	printf("Bearing: %f, %d� %d' %d\"\n", az, deg, min, sec);

	return 0;
}
