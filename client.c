#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>

#include <stdlib.h>
#include <limits.h>
#include <getopt.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

/**
 * Display help when run with wrong options or -h/--help
    Klient:
    -s <pid> PID procesu serwera,
    -r <sygnał> numer sygnału RT, który ma być zastosowany przy komunikacie zwrotnym,
    -x <nr> numer księgi, którą ma odczytać,
    -p <czas> długość interwału pomiędzy komunikatami.
 */
void helpDisplay(){
    printf("\nKlient:\n\t-s <pid> PID procesu serwera\n\t-r <sygnał> numer sygnału RT, który ma być zastosowany przy komunikacie zwrotnym\n\t-x <nr> numer księgi, którą ma odczytać\n\t-p <czas> długość interwału pomiędzy komunikatami\n\n\n");
}



// *************************************************************
// *************************************************************
//                           MAIN
// *************************************************************
// *************************************************************
int main(int argc, char **argv) {

    int serverPID = 0;
    int signalNum = 0;
    int bookNum = 0;
    int pauseTime = 0;

    // *************************************************************
    //                      READ PARAMETERS (getopd_long)
    // *************************************************************
    /* Get parameters to specify the initial conditions */
    int next_option;
    int option_index = 0;
    char const *short_options = "hs:r:x:p:";
    /* Struct to getopt_long {long_name <string>,required_argument(1)/no_argument(0), NULL, short_name <char> )*/
    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h' },
        {"pid", required_argument, NULL, 's'},
        {"signal", required_argument, NULL, 'r'},
        {"num", required_argument, NULL, 'x'},
        {"pause", required_argument, NULL, 'p' },
        {NULL, 0, NULL, 0 }
    };

    do {
        next_option = getopt_long(argc, argv, short_options, long_options, &option_index);
        switch (next_option) {
            case 's':
                serverPID = (int)strtol(optarg,(char **)NULL, 10);
                break;
            case 'r':
                signalNum = (int)strtol(optarg,(char **)NULL, 10);
                break;
            case 'h':
                helpDisplay();
                return 0;
            case 'x':
                bookNum = (int)strtol(optarg,(char **)NULL, 10);
                break;
            case 'p':
                pauseTime = (int)strtol(optarg,(char **)NULL, 10);
                break;
            case '?':
                helpDisplay();
                return 0;
            default:
                break;
        }
    } while(next_option != -1);

    if ( (serverPID && signalNum && bookNum && pauseTime) == 0 ) {
        printf("Need all arguments\n");
        helpDisplay();
        return 1;
    } 
    else {
        printf("\n\nPID: %d, Signal: %d, Num: %d, Pause: %d\n\n",  serverPID, signalNum, bookNum, pauseTime );
    }


    kill(serverPID, SIGRTMIN+11);


    pause();

    return 0;
}