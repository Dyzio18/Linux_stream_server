#include "common.h"

// ******************************
//  HELPERS FUNCTIONS
// ******************************

/**
 * Display help when run with wrong options or -h/--help
    Klient:
    -s <pid> PID procesu serwera,
    -r <sygnał> numer sygnału RT, który ma być zastosowany przy komunikacie zwrotnym,
    -x <nr> numer księgi, którą ma odczytać,
    -p <czas> długość interwału pomiędzy komunikatami.
 */
void helpDisplay_client()
{
    printf("\nKlient:\n\t-s <pid> PID procesu serwera\n\t-r <signal> numer sygnału RT, który ma być zastosowany przy komunikacie zwrotnym\n\t-x <int> numer księgi, którą ma odczytać\n\t-o <int> długość interwału pomiędzy komunikatami\n\t-f <char> (fragmentacja tekstu)\n\t\t z - slowa\n\t\t l - linia \n\t\t s - slowa\n\t-p <file> sciezka do tablicy ogloszeniowej\n\n\n");
}

/**
 * Display help when run with wrong options or -h/--help
    Serwer:
    -k <katalog> katalog, w którym są przechowywane pliki do udostępniania,
    -p <plik> ścieżka do pliku pełniącego rolę tablicy ogłoszeniowej.
 */
void helpDisplay_server()
{
    printf("\nSerwer:\n\t-k <katalog> katalog, w którym są przechowywane pliki do udostępniania\n\t-p <plik> ścieżka do pliku pełniącego rolę tablicy ogłoszeniowej.\n\n");
}

// ******************************
// CODE/DECODE DATA TO/FROM SIGNALS
// ******************************

int codeDataFrame_client(int code, int book, char part, int time)
{
    unsigned char bitA = (unsigned char)code;
    unsigned char bitB = (unsigned char)book;
    unsigned char bitC = part;
    unsigned char bitD = (unsigned char)time;
    const unsigned char buf[4] = {bitA, bitB, bitC, bitD};
    const uint32_t dataFrame = (buf[0]) | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
    return dataFrame;
}

int codeDataFrame_server(int status, int size, int pointer)
{
    unsigned char bitA = (unsigned char)status;
    unsigned char bitB = (unsigned char)size;
    unsigned char bitC = (short int)pointer;
    const unsigned char buf[3] = {bitA, bitB, bitC};
    const uint32_t dataFrame = ((buf[0]) | (buf[1] << 8) | (buf[2] << 16));
    return dataFrame;
}

void decodeDataFrame_server(unsigned char *dataArr, int value)
{
    dataArr[3] = (value >> 24) & 0xFF;
    dataArr[2] = (value >> 16) & 0xFF;
    dataArr[1] = (value >> 8) & 0xFF;
    dataArr[0] = value & 0xFF;
}

short int decodeDataFrame_client(unsigned char *dataArr, int value)
{
    dataArr[1] = (value >> 8) & 0xFF;
    dataArr[0] = value & 0xFF;
    short int offset = (value >> 16 | value >> 24) & 0xFF;
    return offset;
    //(short int)(value >> 16) & 0xFF;
}

// ******************************
// ROT13
// ******************************

int rot13(int c)
{
    if ('a' <= c && c <= 'z')
    {
        return rot13b(c, 'a');
    }
    else if ('A' <= c && c <= 'Z')
    {
        return rot13b(c, 'A');
    }
    else
    {
        return c;
    }
}

int rot13b(int c, int basis)
{
    c = (((c - basis) + 13) % 26) + basis;
    return c;
}

void rot13_arr(char *origin, char *check, int len)
{
    int next = 1;
    for(int i = 0; i<len && next == 1; i++)
    {
        if(origin[i+1] == '\0' || origin[i+1] == '\n')
            next = 0;
        check[i] = rot13(origin[i]);
    }
}

int rot13_arr_check(char *origin, char *check, int len)
{
    int next = 1;
    for(int i = 0; i<len && next == 1; i++)
    {
        if( (origin[i+1] == '\0') || (origin[i+1] == '\n') )
            next = 0;
        if (rot13(check[i]) != origin[i])
            return 0;
    }
    return 1;
}
