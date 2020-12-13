#include <iostream>
#include <winsock2.h>
#include <stdio.h>
#include <fstream>
#include <vector>
#include <time.h>
#include <thread>

using namespace std;
const int MAXLEN = 509;
char buffer[200000000];
const unsigned char ACK = 0x01;
const unsigned char NAK = 0x02;
const unsigned char LAST = 0x08; //0000 0100
const unsigned char NOTLAST = 0x18;
const unsigned char FSHAKE = 0x03;
const unsigned char SSHAKE = 0x04;
const unsigned char TSHAKE = 0x05;
const unsigned char FWAKE = 0x06;
const unsigned char SWAKE = 0x07;
const int MAX_WAIT_TIME = 500;