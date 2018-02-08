// ZAPIS

#define _POSIX_C_SOURCE 199309
#include <signal.h>
#include "tlpi_hdr.h"
int main(int argc, char *argv[])
{
    int sig, numSigs, j, sigData;
    union sigval sv;
            sv.sival_int = sigData + j;

    if (argc < 4 || strcmp(argv[1], "--help") == 0)
        usageErr("%s pid sig-num data [num-sigs]\n", argv[0]);

    /*  Display our PID and UID, so that they can be compared with the
        corresponding fields of the siginfo_t argument supplied to the
        handler in the receiving process */
    printf("%s: PID is %ld, UID is %ld\n", argv[0],
           (long)getpid(), (long)getuid());

    sig = getInt(argv[2], 0, "sig-num");
    sigData = getInt(argv[3], GN_ANY_BASE, "data");
    numSigs = (argc > 4) ? getInt(argv[4], GN_GT_0, "num-sigs") : 1;
    for (j = 0; j < numSigs; j++)
    {
        sv.sival_int = sigData + j;
        if (sigqueue(getLong(argv[1], 0, "pid"), sig, sv) == -1)
            errExit("sigqueue %d", j);
    }
    exit(EXIT_SUCCESS);
}

// **********************
// *********************
// ODCZYT

#include <sys/signalfd.h>
#include <signal.h>
#include "tlpi_hdr.h"
int main(int argc, char *argv[])
{
    sigset_t mask;
    int sfd, j;
    struct signalfd_siginfo fdsi;
    ssize_t s;
    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s sig-num...\n", argv[0]);
    printf("%s: PID = %ld\n", argv[0], (long)getpid());
    sigemptyset(&mask);
    for (j = 1; j < argc; j++)
        sigaddset(&mask, atoi(argv[j]));
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
        errExit("sigprocmask");
    sfd = signalfd(-1, &mask, 0);
    if (sfd == -1)
        errExit("signalfd");
    for (;;)
    {
        s = read(sfd, &fdsi, sizeof(struct signalfd_siginfo));
        if (s != sizeof(struct signalfd_siginfo))
            errExit("read");
        printf("%s: got signal %d", argv[0], fdsi.ssi_signo);
        if (fdsi.ssi_code == SI_QUEUE)
        {
            printf("; ssi_pid = %d; ", fdsi.ssi_pid);
            printf("ssi_int = %d", fdsi.ssi_int);
        }
        printf("\n");
    }
}