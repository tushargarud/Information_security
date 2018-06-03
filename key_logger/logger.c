#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Restore the effective UID to its original value. */

void do_setuid ()
{
  int status;

#ifdef _POSIX_SAVED_IDS
  status = seteuid (owner);
#else
  status = setreuid (runner, owner);
#endif
  if (status < 0) {
    fprintf (stderr, "Couldn't set uid.\n");
    exit (status);
    }
}


/* Set the effective UID to the real UID. */

void undo_setuid ()
{
  int status;

#ifdef _POSIX_SAVED_IDS
  status = seteuid (runner);
#else
  status = setreuid (owner, runner);
#endif
  if (status < 0) {
    fprintf (stderr, "Couldn't set uid.\n");
    exit (status);
    }
}


void initialize() {
    // Lots of other stuff could be happening here
    printf("Initializing the program.\n");
}

FILE *secure_open(char *filename, char *mode) {
    // This should really do things like:
    //   - check the format filename (if input from the user)
    //   - check the file descriptor number (not stdout, say)
    // BUT you don't need to for this assignment.

    FILE *file;
	do_setuid ();
    file = fopen(filename, mode);
	undo_setuid ();
    return file;
}

void error() {
    printf("Error opening logfile. Exiting.\n");
    exit(1);
}

void operate(FILE *logfile) {
    uid_t runner, owner;
    struct timeval *tp;
    time_t secs;
    char *dt, *p;

    // MODIFY BELOW
    // useful commands: getuid, geteuid
    runner = getuid();
    owner = geteuid();

    printf("Operating. Owner: %u, Runner: %u\n", owner, runner);

    secs = time(NULL);
    dt = ctime(&secs);
    if ((p = strchr(dt, '\n')) != NULL) {
        *p = '\0';
    }

    fprintf(logfile, "%s uid %d: Nothing happened.\n", dt, runner);
    fclose(logfile);
}

int main(int argc, char *argv[]) {
    // ADD BELOW
    // useful commands: getuid, geteuid, setuid, setruid, seteuid, setreuid
	runner = getuid ();
	owner = geteuid ();
	undo_setuid ();
	
    FILE *logfile;

    initialize();           // arbitrary

    logfile = secure_open(argv[1], "a+");

    if (!logfile) {
        error();            // report and exit
    }

    // write to the logfile, close the file
    operate(logfile);
    return 1;
}
