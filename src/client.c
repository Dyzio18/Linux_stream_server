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
#include <sys/un.h>
#include <sys/socket.h>
#include <stdint.h> // int32_t


#include "common.h"


#define SV_SOCK_PATH "/tmp/us_xfr"
#define BUF_SIZE 100


/**
 * Display help when run with wrong options or -h/--help
    Klient:
    -s <pid> PID procesu serwera,
    -r <sygnał> numer sygnału RT, który ma być zastosowany przy komunikacie zwrotnym,
    -x <nr> numer księgi, którą ma odczytać,
    -p <czas> długość interwału pomiędzy komunikatami.
 */
/*void helpDisplay()
{
    printf("\nKlient:\n\t-s <pid> PID procesu serwera\n\t-r <sygnał> numer sygnału RT, który ma być zastosowany przy komunikacie zwrotnym\n\t-x <nr> numer księgi, którą ma odczytać\n\t-p <czas> długość interwału pomiędzy komunikatami\n\n\n");
}*/


int convertInput (int code, int book, char part, int time);

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

    // *************************************************************
    //                      READ PARAMETERS (getopd_long)
    // *************************************************************
    /* Get parameters to specify the initial conditions */
    int next_option;
    int option_index = 0;
    char const *short_options = "hs:r:x:p:";
    /* Struct to getopt_long {long_name <string>,required_argument(1)/no_argument(0), NULL, short_name <char> )*/
    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"pid", required_argument, NULL, 's'},
        {"signal", required_argument, NULL, 'r'},
        {"num", required_argument, NULL, 'x'},
        {"pause", required_argument, NULL, 'p'},
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
            helpDisplay();
            return 0;
        case 'x':
            bookNum = (int)strtol(optarg, (char **)NULL, 10);
            break;
        case 'p':
            pauseTime = (int)strtol(optarg, (char **)NULL, 10);
            break;
        case '?':
            helpDisplay();
            return 0;
        default:
            break;
        }
    } while (next_option != -1);

    if ((serverPID && signalNum && bookNum && pauseTime) == 0)
    {
        printf("Need all arguments\n");
        helpDisplay();
        return 1;
    }
    else
    {
        printf("\n\nPID: %d, Signal: %d, Num: %d, Pause: %d", serverPID, signalNum, bookNum, pauseTime);
    }






    // Get client PID
    pid_t clientPID = getpid();
    printf("\nClient PID: %d\n", clientPID);

    // Send SIGNAL with DATA
    union sigval sv;
    sv.sival_int = convertInput(signalNum, bookNum, 'l', pauseTime);
    sigqueue(serverPID, SIGRTMIN + 11, sv);

    pause();

    return 0;
}


/**
 * Convert input to data to send
 */
int convertInput (int code, int book, char part, int time){

    unsigned char bitA = (unsigned char)code;
    unsigned char bitB = (unsigned char)book;
    unsigned char bitC = part;
    unsigned char bitD = (unsigned char)time;
    const unsigned char buf[4] = { bitA, bitB, bitC, bitD };
    const uint32_t dataFrame = (buf[0]) | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
    return dataFrame;
}