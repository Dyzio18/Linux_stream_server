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
#include <pthread.h>

#include "common.h"

int filedesc;

struct signalfd_siginfo fdsi;
struct threadData
{
    int clientPID;
    int book;
    char fragmentation;
    int interval;
    char *path;
};

#define BUF_SIZE 10
#define MAX_PATH 20

/*****************************************
 *              THREAD                   *
 *****************************************/
void *clientThread_handler(void *thread_arg)
{
    int CONNECTED = 1;
    struct threadData *myData;
    myData = (struct threadData *)thread_arg;
    struct sockaddr_un
    {
        sa_family_t sun_family;
        char sun_path[108];
    };

    // -----------------------------------------------
    //                  SOCKETS
    // -----------------------------------------------

    char *SV_SOCK_PATH = myData->path;
    //printf("\n:::%s\n", SV_SOCK_PATH);

    int socket_fd;
    struct sockaddr_un server_address, client_address;
    int bytes_received, bytes_sent, integer_buffer;
    socklen_t address_length = sizeof(struct sockaddr_un);

    if ((socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
    }

    // SERVER SOCKET
    memset(&server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, SV_SOCK_PATH);
    server_address.sun_path[0] = 0;

    if (bind(socket_fd, (const struct sockaddr *)&server_address, address_length) < 0) {
        close(socket_fd);
        perror("bind");
    }

    while(CONNECTED)
    {
        // RECEIVE
        bytes_received = recvfrom(socket_fd, &integer_buffer, sizeof(integer_buffer), 0, (struct sockaddr *)&client_address, &address_length);
        if (bytes_received != sizeof(integer_buffer)) {
            printf("Error: recvfrom - %d.\n", bytes_received);
        }

        // SEND
        printf("received: %d.\n", integer_buffer);
        integer_buffer += 10;
        bytes_sent = sendto(socket_fd, &integer_buffer, sizeof(integer_buffer), 0, (struct sockaddr *)&client_address, address_length);
        
        sleep(5);
    }

    close(socket_fd);

    // -----------------------------------------------
    // -----------------------------------------------

    pthread_exit(NULL);
}

/*****************************************
 *         REAL TIME SIGNAL              *
 *****************************************/
/**
 * Handle RT signal - recieve data from signal then try register client.
 * Send RT signal to client with response.
 * If register succesfull, save adres to "info" file and run new thread to handling client
 */
void clientRegisterSignal_handler(int signum, siginfo_t *siginfo, void *ptrVoid)
{
    unsigned int clientPID = siginfo->si_pid;               // Get PID of signal sender
    unsigned int clientValue = siginfo->si_value.sival_int; // Get client value
    printf("\nNew client connected (PID: %d)", clientPID, clientValue);
    // Decode data frame
    unsigned char clientData[4];
    decodeDataFrame_server(clientData, clientValue);
    printf(" with request: (book) %d, (frag) %c, (interval) %d ", clientData[1], clientData[2], clientData[3]);

    int registerStatus = 1;
    int pathSize = MAX_PATH;
    int pathPointer = 0;

    // Get offset
    pathPointer = lseek(filedesc, pathPointer, SEEK_CUR);

    // Write socket adres to info file
    char socketPath[MAX_PATH];
    sprintf(socketPath, "SOCK_%d\n", clientPID);
    if (write(filedesc, &socketPath, sizeof(socketPath)) <= 0)
    {
        perror("write");
        registerStatus = 0;
    }

    // Save data to structure and pass it to new THREAD
    struct threadData currUser;
    currUser.clientPID = clientPID;
    currUser.book = clientData[1];
    currUser.fragmentation = clientData[2];
    currUser.interval = clientData[3];
    currUser.path = socketPath;


    // CREATE THREAD
    pthread_t client_thread;
    if (pthread_create(&client_thread, NULL, clientThread_handler, &currUser) < 0)
    {
        perror("pthread_create");
        registerStatus = 0;
    }


    // Send SIGNAL with DATA
    union sigval sv;
    sv.sival_int = codeDataFrame_server(registerStatus, pathSize, pathPointer);
    sigqueue(clientPID, SIGRTMIN + clientData[0], sv); // Respons signal must be RT Signal
}

/**
 * Create handler for RT signal 
 */
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
    printf("\nServer PID: %d\nWaiting for clients....\n", serverPID);

    clientRegisterSignal_create();

    // Open info file
    filedesc = open(infofile, O_WRONLY | O_TRUNC);
    if (filedesc < 0)
    {
        return -1;
    }

    // *************************************************************
    //                   REGISTER LOOP (waiting for clients)
    // *************************************************************

    int serverLoop = 1;
    while (serverLoop)
    {

        pause();
    }

    return 0;
}
