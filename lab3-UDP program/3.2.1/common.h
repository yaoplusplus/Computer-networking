#include <iostream>
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <time.h>
#include <string>
#include <bitset>
#include <queue>
#include <cstdlib>
#include <ctime>

#pragma comment(lib, "Ws2_32.lib")
using namespace std;

const int MAXLEN = 509;
char buffer[100000000];
const unsigned char ACK = 0x01;
const unsigned char NAK = 0x02;
const unsigned char FIRST_SHAKE = 0x03;
const unsigned char SECOND_SHAKE = 0x04;
const unsigned char THIRD_SHAKE = 0x05;
const unsigned char FIRST_WAVE = 0x06;
const unsigned char SECOND_WAVE = 0x07;
const unsigned char LAST = 0x08;
const unsigned char NOTLAST = 0x18;
const int TIMEOUT = 500;
static int seq = 0;
int Window_Size = 5;
int SSTH = 100;