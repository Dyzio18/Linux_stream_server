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
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "common.h" 


#define SV_SOCK_PATH "/tmp/us_xfr"
#define BUF_SIZE 100
int REGISTER_FLAG = 0;
struct signalfd_siginfo fdsi;
int filedesc;

/**
 * Handler of server response RT Signal
 * Get and decode data frame then if register succes open socket
 */
void serverResponseSignal_handler(int signum, siginfo_t *siginfo, void *ptrVoid)
{
    unsigned int hostPID = siginfo->si_pid; // Get PID of signal sender
    unsigned int serverValue = siginfo->si_value.sival_int; // Get client value
   // printf("\nServer_DATA::\nPID: %d\nValue: %08x\n ", hostPID, serverValue);

    // Read and convert data from signal
    unsigned char serverData[2];
    short int offset = 0;
    offset = decodeDataFrame_client(serverData,serverValue);
   // printf("\n%d %d %d\n", serverData[0], serverData[1], offset);

    // Read path from info file (if register succes)
    if (serverData[0] == 1)
    {
        char socketPath[serverData[1]]; 
        read(filedesc, socketPath, sizeof(socketPath[0])*serverData[1]);
     //   printf("\nPATH:%s",socketPath);
    }

    REGISTER_FLAG=1;
    printf("-------------// %d", REGISTER_FLAG);
}

void serverResponseSignal_create(int sigNum)
{
    struct sigaction clSig;
    memset(&clSig, '\0', sizeof(clSig));
    clSig.sa_flags = SA_SIGINFO;
    clSig.sa_sigaction = serverResponseSignal_handler;

    if ((sigaction(SIGRTMIN + sigNum, &clSig, NULL)) == -1)
    {
        perror("sigaction");
    }
}


// *************************************************************
// *************************************************************
//                           MAIN
// *************************************************************
// *************************************************************
int main(int argc, char **argv)
{

    int serverPID = 0;
    int signalNum = 0;
    int bookNum = 0;
    int pauseTime = 0;
    char fragmentation = 'l';
    char *infoFile;


    // *************************************************************
    //                      READ PARAMETERS (getopd_long)
    // *************************************************************
    /* Get parameters to specify the initial conditions */
    int next_option;
    int option_index = 0;
    char const *short_options = "hs:r:x:o:f:p:";
    /* Struct to getopt_long {long_name <string>,required_argument(1)/no_argument(0), NULL, short_name <char> )*/
    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"pid", required_argument, NULL, 's'},
        {"signal", required_argument, NULL, 'r'},
        {"num", required_argument, NULL, 'x'},
        {"pause", required_argument, NULL, 'o'},
        {"part", required_argument, NULL, 'f'},
        {"file", required_argument, NULL, 'p'},
        {NULL, 0, NULL, 0}};

    do
    {
        next_option = getopt_long(argc, argv, short_options, long_options, &option_index);
        switch (next_option)
        {
        case 's':
            serverPID = (int)strtol(optarg, (char **)NULL, 10);
            break;
        case 'r':
            signalNum = (int)strtol(optarg, (char **)NULL, 10);
            break;
        case 'h':
            helpDisplay_client();
            return 0;
        case 'x':
            bookNum = (int)strtol(optarg, (char **)NULL, 10);
            break;
        case 'o':
            pauseTime = (int)strtol(optarg, (char **)NULL, 10);
            break;
        case 'f':
            fragmentation = optarg[0];
            break;
        case 'p':
            infoFile = optarg;
            break;
        case '?':
            helpDisplay_client();
            return 0;
        default:
            break;
        }
    } while (next_option != -1);

    if ((serverPID && signalNum && bookNum && pauseTime) == 0)
    {
        printf("Need all arguments\n");
        helpDisplay_client();
        return 1;
    }
    else
    {
        printf("\n\nPID: %d, Signal: %d, Num: %d, Pause: %d, Fragmetation: %c, File: %s", serverPID, signalNum, bookNum, pauseTime,fragmentation, infoFile);
    }

    // Open info file
    if ( (filedesc = open(infoFile, O_RDONLY)) < 0)
    {
        perror("open");
        return -1;
    }

    // Dynamic signal handler
    serverResponseSignal_create(signalNum); 

    // Get client PID
    pid_t clientPID = getpid();
    printf("\nClient PID: %d\n", clientPID);

    // Send SIGNAL with DATA
    union sigval sv;
    sv.sival_int = codeDataFrame_client(signalNum, bookNum, 'l', pauseTime);
    sigqueue(serverPID, SIGRTMIN + 11, sv);



    while (1) {
            // Wait for server response
        printf("\n\tprzed**%d",REGISTER_FLAG);
        pause(); 
        printf("\n\tpo**%d",REGISTER_FLAG);
    }

    pause(); 



    return 0;
}