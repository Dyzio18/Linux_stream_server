#include <stdio.h>
#include <stdint.h> // int32_t

void helpDisplay_client();
void helpDisplay_server();

int codeDataFrame_client (int code, int book, char part, int time);
int codeDataFrame_server (int status, int size, int pointer);
void decodeDataFrame_server(unsigned char *dataArr, int value);
void decodeDataFrame_client(unsigned char *dataArr, int value);
