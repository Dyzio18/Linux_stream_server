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

#include "common.h"

int filedesc;

struct signalfd_siginfo fdsi;



void clientRegisterSignal_handler(int signum, siginfo_t *siginfo, void *ptrVoid)
{
    unsigned int clientPID = siginfo->si_pid; // Get PID of signal sender
    unsigned int clientValue = siginfo->si_value.sival_int; // Get client value

    printf("PID: %d\nValue: %08x\n ", clientPID, clientValue);

    // Decode data frame
    unsigned char clientData[4];
    decodeDataFrame_server(clientData,clientValue);
    printf("%d %d %d %d\n", clientData[0], clientData[1], clientData[2], clientData[3]);

    /**
     * TODO: REGISTER USER
     *  -------------------------
     * 
     *      try create socket
     *      
     * 
     *  ------------------------- 
     */

    int registerStatus=1;
    int pathSize=12;
    int pathPointer=0;

    // Get offset 
    pathPointer = lseek(filedesc, pathPointer, SEEK_CUR);
    printf("\n\t*ptr-->%d\n",pathPointer);


    // Write socket adres to info file
    char socketPath[12];
    sprintf(socketPath, "SOCK_%d\n", clientPID);
    if (write(filedesc, &socketPath, sizeof(socketPath)) <= 0)
    {
        perror("write");
        registerStatus = 0;
    }

    // Send SIGNAL with DATA
    union sigval sv;
    sv.sival_int = codeDataFrame_server(registerStatus, pathSize, pathPointer);
    sigqueue(clientPID, SIGRTMIN + clientData[0], sv); // Respons signal must be RT Signal



}

void clientRegisterSignal_create()
{
    struct sigaction clSig;
    memset(&clSig, '\0', sizeof(clSig));
    clSig.sa_flags = SA_SIGINFO;
    clSig.sa_sigaction = clientRegisterSignal_handler;
    if ((sigaction(SIGRTMIN + 11, &clSig, NULL)) == -1)
        perror("sigaction");
}

// *************************************************************
//                           MAIN
// *************************************************************
int main(int argc, char **argv)
{

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
        {"help", no_argument, NULL, 'h'},
        {"catalog", required_argument, NULL, 'k'},
        {"file", required_argument, NULL, 'p'},
        {NULL, 0, NULL, 0}};

    do
    {
        next_option = getopt_long(argc, argv, short_options, long_options, &option_index);
        switch (next_option)
        {
        case 'k':
            catalog = optarg;
            checkArgs++;
            break;
        case 'p':
            infofile = optarg;
            checkArgs++;
            break;
        case 'h':
            helpDisplay_server();
            return 0;
        case '?':
            helpDisplay_server();
            return 0;
        default:
            break;
        }
    } while (next_option != -1);

    if (checkArgs != 2)
    {
        printf("Need all arguments\n");
        helpDisplay_server();
        return 1;
    }
    else
    {
        printf("\nServe files from: %s \nInfofile: %s \n", catalog, infofile);
    }

    // Get server PID
    pid_t serverPID = getpid();
    printf("\nServer PID: %d\n\n", serverPID);

    clientRegisterSignal_create();

    // Open info file
    filedesc = open(infofile, O_WRONLY | O_TRUNC);
    if (filedesc < 0)
    {
        return -1;
    }

    // *************************************************************
    //                            SOCKET
    // *************************************************************
    struct sockaddr_un
    {
        sa_family_t sun_family;
        char sun_path[108];
    };

    const char *SOCKNAME = "q7xcw";
    struct sockaddr_un addr;

    /** TODO:
 *   [X] - create SOCKET 
 *   [ ] - generate random socket path name
 */
    int socketFd = socket(AF_UNIX, SOCK_DGRAM, 0);                   /* Create SOCKET */
    memset(&addr, 0, sizeof(struct sockaddr_un));                    /* Clear structure */
    addr.sun_family = AF_UNIX;                                       /* UNIX domain address */
    strncpy(&addr.sun_path[1], SOCKNAME, sizeof(addr.sun_path) - 2); /* Abstract namespace */
    if (bind(socketFd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1)
        perror("bind");

    int serverLoop = 1;
    while (serverLoop)
    {

        pause();
    }

    return 0;
}