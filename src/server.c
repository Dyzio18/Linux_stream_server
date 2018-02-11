/**
 * Name: Book streaming (SERVER)
 * @Author: Patryk Nizio
 * Date: 2018.02
 * https://github.com/Dyzio18/Linux_stream_server
 */

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
#include <time.h>
#include "common.h"

int filedesc;
char *catalog;
int CLIENTS_COUNT = 0;

struct signalfd_siginfo fdsi;
struct timespec req, rem;
struct threadData
{
    int clientPID;
    int book;
    char fragmentation;
    int interval;
    char *path;
    pthread_t client_thread;
};
int nsleep(int time);

#define MAX_PATH 16
#define MAX_CLIENTS 32
#define LINE 256
#define WORD 24

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

    char sourcePath[128];
    snprintf(sourcePath, sizeof(sourcePath), "%s%d.txt", catalog, myData->book);
    printf("-->%s", sourcePath);

    // -----------------------------------------------
    //                  LOAD EXPECTED BOOK
    // -----------------------------------------------
    char *source = NULL;
    FILE *fp = fopen(sourcePath, "r");

    if (fp == NULL){
        snprintf(sourcePath, sizeof(sourcePath), "%s/%d.txt", catalog, myData->book);
        fp = fopen(sourcePath, "r");
    }

    long bufsize;
    if (fp != NULL)
    {
        // Check end of the file
        if (fseek(fp, 0L, SEEK_END) == 0)
        {
            // Get the size of the file
            bufsize = ftell(fp);
            if (bufsize == -1)
            {
                perror("ftell");
            }

            // Allocate buffer to file size
            source = (char *)malloc(sizeof(char) * (bufsize + 1));
            // Go back to the start of the file.
            if (fseek(fp, 0L, SEEK_SET) != 0)
            {
                perror("fseek");
            }

            // Read the entire file into memory
            size_t newLen = fread(source, sizeof(char), bufsize, fp);
            if (ferror(fp) != 0)
            {
                fputs("Error reading file", stderr);
            }
            else
            {
                source[newLen++] = '\0';
            }
        }
        fclose(fp);
    }
    else
    {
        printf("\nResources not found...\n ");
        pthread_cancel(myData->client_thread);
        pthread_exit(NULL);
    }

    // -----------------------------------------------
    //                  SOCKETS
    // -----------------------------------------------

    char *SV_SOCK_PATH = myData->path;
    int socket_fd;
    struct sockaddr_un server_address, client_address;
    int bytes_received = 0;
    //int bytes_sent = 0;
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
        kill(myData->clientPID, 10);
        close(socket_fd);
        perror("bind");
    }

    // DISPLAY TEXT (for test only)
    /*  
    char tempChar;
    printf("\nTest display: (intro) \n");
    for (int i = 0; i < 100; i++) {
        tempChar = source[i];
        printf("%c", tempChar);
    }
    */

    int INITIALIZATION_FLAG = 0; // Send first msg with PID to authorize reguest
    int i = 0;
    char msg = 'x';
    char msgCheck = 'x';
    char wordBuff[WORD];
    char wordBuffCheck[WORD];
    char lineBuff[LINE];
    char lineBuffCheck[LINE];

    // -----------------------------------------------
    //           COMMUNITACTION LOOP
    // -----------------------------------------------
    while (CONNECTED)
    {
        // CHECK FIRST MSG WITH CLIENT PID
        if (INITIALIZATION_FLAG == 0)
        {
            recvfrom(socket_fd, &INITIALIZATION_FLAG, sizeof(INITIALIZATION_FLAG), 0, (struct sockaddr *)&client_address, &address_length);
            if (INITIALIZATION_FLAG == myData->clientPID)
            {
                printf("\n\nCLIENT with PID %d authorized\n", INITIALIZATION_FLAG);

                /*  TODO:   Tutaj moge uruchomić timer co "interval" 
                    EDIT:   Wykonałem to na nanosleep wiec juz nieaktualne...
                            ...ale gdybym zrobił pisanie na timerach to tutaj zainicjowałbym timer i wyzwalał zapisywanie do socketa.
                */
            }
            else
            {
                printf("\n\nCLIENT with PID %d cannot be authorized\n", myData->clientPID);
                CONNECTED = 0;
            }
        }
        else
        {
            if (myData->fragmentation == 'l')
            {
                int lineLen = 0;
                int nextFlag = 1;
                char sign;
                memset(&lineBuff[0], 0, sizeof(lineBuff));

                for (int j = 0; nextFlag == 1 && j < LINE; j++)
                {
                    if ((source[i] == '\0') || (source[i] == '\n'))
                        nextFlag = 0;

                    sign = source[i++];
                    lineBuff[j] = sign;
                    lineLen++;
                }
                lineBuff[lineLen + 1] = '\0';
                sendto(socket_fd, &lineBuff, sizeof(char) * LINE, 0, (struct sockaddr *)&client_address, address_length);
                bytes_received = recvfrom(socket_fd, &lineBuffCheck, sizeof(char) * LINE, 0, (struct sockaddr *)&client_address, &address_length);
                //write(STDOUT_FILENO, lineBuffCheck, sizeof(char) * LINE);
                if (!rot13_arr_check(lineBuff, lineBuffCheck, WORD))
                {
                    kill(myData->clientPID, 10);
                    CONNECTED = 0;
                }
            }
            else if (myData->fragmentation == 's')
            {
                /* SEND WORD */
                int wordLen = 0;
                int nextFlag = 1;
                char sign;
                memset(&wordBuff[0], 0, sizeof(wordBuff));
                for (int j = 0; nextFlag == 1 && j < WORD; j++)
                {
                    if ((source[i] == '\0') || (source[i] == '\n') || (source[i] == ' '))
                        nextFlag = 0;

                    sign = source[i++];
                    wordBuff[j] = sign;
                    wordLen++;
                }
                wordBuff[wordLen + 1] = '\0';
                sendto(socket_fd, &wordBuff, sizeof(char) * WORD, 0, (struct sockaddr *)&client_address, address_length);
                bytes_received = recvfrom(socket_fd, &wordBuffCheck, sizeof(char) * WORD, 0, (struct sockaddr *)&client_address, &address_length);
                //write(STDOUT_FILENO, wordBuff, sizeof(char) * WORD);
                if (!rot13_arr_check(wordBuff, wordBuffCheck, WORD))
                {
                    kill(myData->clientPID, 10);
                    CONNECTED = 0;
                }
            }
            else
            {
                /* SEND LETTER */
                msg = source[i++];
                sendto(socket_fd, &msg, sizeof(msg), 0, (struct sockaddr *)&client_address, address_length);
                bytes_received = recvfrom(socket_fd, &msgCheck, sizeof(msgCheck), 0, (struct sockaddr *)&client_address, &address_length);

                if (rot13(msg) != msgCheck)
                {
                    kill(myData->clientPID, 10);
                    CONNECTED = 0;
                }
            }
            if (bytes_received < 1)
            {
                CONNECTED = 0;
            }
        }
        //write(STDOUT_FILENO, &msg, sizeof(msg));
        //printf("\nSEND: %d RECV: %d",bytes_sent, bytes_received);
        nsleep(myData->interval);
    }

    free(source);
    close(socket_fd); 
    pthread_exit(NULL);
    pthread_cancel(myData->client_thread);
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
    struct threadData currUser[MAX_CLIENTS];
    currUser[CLIENTS_COUNT].clientPID = clientPID;
    currUser[CLIENTS_COUNT].book = clientData[1];
    currUser[CLIENTS_COUNT].fragmentation = clientData[2];
    currUser[CLIENTS_COUNT].interval = clientData[3];
    currUser[CLIENTS_COUNT].path = socketPath;

    if (CLIENTS_COUNT > MAX_CLIENTS)
    {
        printf("\nUser limit has been reached\n\n");
        registerStatus = 0;
    }
    else
    {
        // CREATE THREAD
        pthread_t client_thread;
        currUser[CLIENTS_COUNT].client_thread = client_thread;
        if (pthread_create(&client_thread, NULL, clientThread_handler, &currUser[CLIENTS_COUNT++]) < 0)
        {
            perror("pthread_create");
            registerStatus = 0;
        }
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

/**
 * Sleep <int> * 1/64[s]
 */
int nsleep(int time)
{
    long interval = (((long)time * 1000) / 64);

    if (interval > 999)
    {
        req.tv_sec = (int)(interval / 1000);
        req.tv_nsec = (interval - ((long)req.tv_sec * 1000)) * 1000000;
    }
    else
    {
        req.tv_sec = 0;
        req.tv_nsec = interval * 1000000;
    }
    return nanosleep(&req, &rem);
}