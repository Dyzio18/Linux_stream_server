
#include <stdio.h>
#include <stdint.h> // int32_t
#include <stddef.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

void helpDisplay_client();
void helpDisplay_server();

int codeDataFrame_client (int code, int book, char part, int time);
int codeDataFrame_server (int status, int size, int pointer);
void decodeDataFrame_server(unsigned char *dataArr, int value);
short int decodeDataFrame_client(unsigned char *dataArr, int value);

int rot13(int c);
int rot13b(int c, int basis);

