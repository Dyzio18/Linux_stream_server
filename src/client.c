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

//#define SV_SOCK_PATH "/tmp/us_xfr"
#define BUF_SIZE 10
#define MAX_PATH 12

int REGISTER_FLAG = 0;
struct signalfd_siginfo fdsi;
int filedesc;
char socketPath[MAX_PATH];

/**
 * Handler of server response RT Signal
 * Get and decode data frame then if register succes open socket
 */
void serverResponseSignal_handler(int signum, siginfo_t *siginfo, void *ptrVoid)
{
    unsigned int hostPID = siginfo->si_pid;                 // Get PID of signal sender
    unsigned int serverValue = siginfo->si_value.sival_int; // Get client value
    printf("\nServer_DATA::\nPID: %d\nValue: %08x\n ", hostPID, serverValue);

    // Read and convert data from signal
    unsigned char serverData[2];
    short int offset = 0;
    offset = decodeDataFrame_client(serverData, serverValue);
    printf("\nResponse_status:%d path_size:%d offset:%d\n", serverData[0], serverData[1], offset);

    // Read path from info file (if register succes)
    if (serverData[0] == 1)
    {
        lseek(filedesc, offset, SEEK_SET);
        if (read(filedesc, socketPath, sizeof(serverData[0]) * serverData[1]) < 0)
            perror("read");
        //sprintf(socketPath, "SOCK_%d\n", getpid());
    }
    REGISTER_FLAG = 1;
}

void serverResponseSignal_create(int sigNum)
{
    struct sigaction clSig;
    memset(&clSig, '\0', sizeof(clSig));
    clSig.sa_flags = SA_SIGINFO;
    clSig.sa_sigaction = serverResponseSignal_handler;

    if ((sigaction(SIGRTMIN + sigNum, &clSig, NULL)) == -1)
        perror("sigaction");
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
        printf("\n\nPID: %d, Signal: %d, Num: %d, Pause: %d, Fragmetation: %c, File: %s", serverPID, signalNum, bookNum, pauseTime, fragmentation, infoFile);
    }

    // Open info file
    if ((filedesc = open(infoFile, O_RDONLY)) < 0)
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
    sv.sival_int = codeDataFrame_client(signalNum, bookNum, fragmentation, pauseTime);
    sigqueue(serverPID, SIGRTMIN + 11, sv);

    pause();

    // ********************************************************
    //                          SOCKET
    // ********************************************************

    while (!REGISTER_FLAG)
    {
        printf("Waiting...");
        sleep(1);
    }


    char *SV_SOCK_PATH = socketPath;
    int socket_fd;
    struct sockaddr_un server_address, client_address;
    int bytes_received, bytes_sent, integer_buffer;
    socklen_t address_length = sizeof(struct sockaddr_un);

    if ((socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
    }

    // CLIENT SOCKET
    memset(&client_address, 0, sizeof(client_address));
    client_address.sun_family = AF_UNIX;
    char clientAddr_path[MAX_PATH];
    char *pathUniq = "cli";
    snprintf(clientAddr_path, sizeof(char)*(MAX_PATH+5) , "%s%s", SV_SOCK_PATH, pathUniq);
    strcpy(client_address.sun_path, clientAddr_path);
    client_address.sun_path[0] = 0; // Abstract namespace

    if (bind(socket_fd, (const struct sockaddr *)&client_address, address_length) < 0){
        perror("bind");
    }

    // SERVER SOCKET
    memset(&server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, SV_SOCK_PATH);
    server_address.sun_path[0] = 0;

    // COMMUNICATION LOOP
    integer_buffer = 1;
    while(REGISTER_FLAG)
    {
        bytes_sent = sendto(socket_fd, &integer_buffer, sizeof(integer_buffer), 0, (struct sockaddr *)&server_address, address_length);
        bytes_received = recvfrom(socket_fd, &integer_buffer, sizeof(integer_buffer), 0, (struct sockaddr *)&server_address, &address_length);
        if (bytes_received != sizeof(integer_buffer)) {
            printf("Error: recvfrom - %d.\n", bytes_received);
        }

        printf("received: %d\n", integer_buffer);

        integer_buffer += 1;
    }

    close(socket_fd);

    return 0;
}