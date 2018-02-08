#include "common.h"

/**
 * Display help when run with wrong options or -h/--help
    Klient:
    -s <pid> PID procesu serwera,
    -r <sygnał> numer sygnału RT, który ma być zastosowany przy komunikacie zwrotnym,
    -x <nr> numer księgi, którą ma odczytać,
    -p <czas> długość interwału pomiędzy komunikatami.
 */
void helpDisplay()
{
    printf("\nKlient:\n\t-s <pid> PID procesu serwera\n\t-r <sygnał> numer sygnału RT, który ma być zastosowany przy komunikacie zwrotnym\n\t-x <nr> numer księgi, którą ma odczytać\n\t-p <czas> długość interwału pomiędzy komunikatami\n\n\n");
}