#include <eflib/include/diagnostics/logrout.h>

#include <stdlib.h>
#include <stdio.h>
#include <cassert>

int main (int argc, char **argv)
{
	if (argc != 2) {
		printf ("Usage: logrout logfile\n");
		exit (1);
	}

	FILE *logfile = fopen (argv[1], "w");

	if (!logfile)
	{
		printf ("Can not open the logfile for write.\n");
		exit (1);
	}

	bool screen_on = true;
	bool file_on = true;
	
	char buf[16000];
	char *line = fgets (buf, 16000, stdin);
	
	while (line)
	{
		if( strstr( line, "$screen$:$on$" ) == line ){
			screen_on = true;
		} else if( strstr( line, "$screen$:$off$" ) == line ){
			screen_on = false;
		} else if( strstr( line, "$file$:$on$" ) == line ){
			file_on = true;
		} else if( strstr( line, "$file$:$off$" ) == line ){
			file_on = false;
		} else {
			if ( screen_on ){
				fputs (line, stdout);
			}
			if( file_on ){
				fputs (line, logfile);
			}
			fflush (stdout);
			fflush (logfile);
		}
		
		line = fgets (buf, 16000, stdin);
	}

	fclose (logfile);
    return 0;
}

