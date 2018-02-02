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

int filedesc;


/**
 * Display help when run with wrong options or -h/--help
    Serwer:
    -k <katalog> katalog, w którym są przechowywane pliki do udostępniania,
    -p <plik> ścieżka do pliku pełniącego rolę tablicy ogłoszeniowej.
 */
void helpDisplay(){
    printf("\nSerwer:\n\t-k <katalog> katalog, w którym są przechowywane pliki do udostępniania\n\t-p <plik> ścieżka do pliku pełniącego rolę tablicy ogłoszeniowej.\n\n");
}

void clientRegisterSignal_handler(int signum, siginfo_t *siginfo, void *ptrVoid)
{
    char* adressConf;
    adressConf = "12345\n";
    if (!write(filedesc, adressConf, sizeof(char)*7)) {
        perror("write");
    }}

void clientRegisterSignal_create()
{
    struct sigaction usr;
    memset( &usr, '\0', sizeof(usr));
    usr.sa_flags = SA_SIGINFO;
    usr.sa_sigaction = clientRegisterSignal_handler;

    if( (sigaction(SIGRTMIN+11, &usr, NULL)) == -1) {
        perror("sigaction");
    }

}


// *************************************************************
// *************************************************************
//                           MAIN
// *************************************************************
// *************************************************************
int main(int argc, char **argv) {

    char *catalog;
    char *infofile;


    // *************************************************************
    //                      READ PARAMETERS (getopd_long)
    // *************************************************************
    /* Get parameters to specify the initial conditions */
    int next_option;
    int option_index = 0;
    char const *short_options = "hk:p:";
    int checkArgs = 0;
    /* Struct to getopt_long {long_name <string>,required_argument(1)/no_argument(0), NULL, short_name <char> )*/
    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h' },
        {"catalog", required_argument, NULL, 'k'},
        {"file", required_argument, NULL, 'p'},
        {NULL, 0, NULL, 0 }
    };

    do {
        next_option = getopt_long(argc, argv, short_options, long_options, &option_index);
        switch (next_option) {
            case 'k':
                catalog = optarg;
                checkArgs++;
                break;
            case 'p':
                infofile = optarg;
                checkArgs++;
                break;
            case 'h':
                helpDisplay();
                return 0;
            case '?':
                helpDisplay();
                return 0;
            default:
                break;
        }
    } while(next_option != -1);

    if ( checkArgs != 2 ) {
        printf("Need all arguments\n");
        helpDisplay();
        return 1;
    } 
    else {
        printf("\nServe files from: %s \nInfofile: %s \n", catalog, infofile);
    }

    // Get server PID
    pid_t serverPID = getpid();
    printf("\nServer PID: %d\n\n", serverPID );

    clientRegisterSignal_create();

    // Open info file
    filedesc = open(infofile, O_WRONLY);
    if (filedesc < 0) {
        return -1;
    }
 

 

    
    int serverLoop = 1;
    while(serverLoop){

        pause();
 
    }

    return 0;
}