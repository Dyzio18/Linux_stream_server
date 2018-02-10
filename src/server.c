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
char *catalog;

struct signalfd_siginfo fdsi;
struct threadData
{
    int clientPID;
    int book;
    char fragmentation;
    int interval;
    char *path;
};

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
    //                  LOAD EXPECTED BOOK
    // -----------------------------------------------
    char *source = NULL;
    FILE *fp = fopen("store/Pan_Tadeusz/0.txt", "r");

    long bufsize;
    if (fp != NULL)
    {
        /* Go to the end of the file. */
        if (fseek(fp, 0L, SEEK_END) == 0)
        {
            /* Get the size of the file. */
            bufsize = ftell(fp);
            if (bufsize == -1) {
                perror("ftell");
            }

            /* Allocate our buffer to that size. */
            source = (char *)malloc(sizeof(char) * (bufsize + 1));

            /* Go back to the start of the file. */
            if (fseek(fp, 0L, SEEK_SET) != 0) {
                perror("fseek");
            }

            /* Read the entire file into memory. */
            size_t newLen = fread(source, sizeof(char), bufsize, fp);
            if (ferror(fp) != 0) {
                fputs("Error reading file", stderr);
            }
            else {
                source[newLen++] = '\0'; /* Just to be safe. */
            }
        }
        fclose(fp);
    }



    // -----------------------------------------------
    //                  SOCKETS
    // -----------------------------------------------

    char *SV_SOCK_PATH = myData->path;
    //printf("\n:::%s\n", SV_SOCK_PATH);

    int socket_fd;
    struct sockaddr_un server_address, client_address;
    int bytes_received = 0;
    int bytes_sent = 0;
    socklen_t address_length = sizeof(struct sockaddr_un);

    if ((socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket");
    }

    // SERVER SOCKET
    memset(&server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, SV_SOCK_PATH);
    server_address.sun_path[0] = 0;

    if (bind(socket_fd, (const struct sockaddr *)&server_address, address_length) < 0)
    {
        close(socket_fd);
        perror("bind");
    }


    // DISPLAY TEXT (for test only)
        char tempChar;
        for (int i = 0; i < bufsize; i++)
        {
            tempChar =  source[i];
            printf("%c",tempChar);
        }
    // ---------


    int INITIALIZATION_FLAG = 0; // Send first msg with PID to authorize reguest
    int i = 0;
    char msg = 'x';
    
    // -----------------------------------------------
    //           COMMUNITACTION LOOP
    // -----------------------------------------------
    while (CONNECTED)
    {
        // CHECK FIRST MSG WITH CLIENT PID
        if (INITIALIZATION_FLAG == 0) {
            bytes_received = recvfrom(socket_fd, &INITIALIZATION_FLAG, sizeof(INITIALIZATION_FLAG), 0, (struct sockaddr *)&client_address, &address_length);
            if (INITIALIZATION_FLAG == myData->clientPID) {           
                printf("\n\nCLIENT with PID %d authorized\n",INITIALIZATION_FLAG);
            }
            else {
                printf("\n\nCLIENT with PID %d cannot be authorized\n", myData->clientPID);
                CONNECTED = 0;
            }
            printf("\n\n**Init succes*\n\n");
        }
        else {
            // SEND
            msg = source[i++];
            //printf("%c", msg);
            write(STDOUT_FILENO, &msg, sizeof(msg));
            bytes_sent = sendto(socket_fd, &msg, sizeof(msg), 0, (struct sockaddr *)&client_address, address_length);
            // RECEIVE
            bytes_received = recvfrom(socket_fd, &msg, sizeof(msg), 0, (struct sockaddr *)&client_address, &address_length);
            
        }
        //printf("\nSEND: %d RECV: %d",bytes_sent, bytes_received);
        sleep(1);
    }

    free(source);
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
    printf("\nNew client connected (PID: %d)", clientPID);
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
