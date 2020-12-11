struct ackwindow{
char*pkt;//放置发送的包 发包的时候更新这个 不考虑握手和挥手 
bool ACK;
bool NAK;
int length;
bool last_flag;
ackwindow(){
    ACK = false;
    NAK = false;
}
};
struct slide_window{
ackwindow slide_w[1000];
int sendbase;
int nextseqnum;
slide_window(){
    sendbase = 0;
    nextseqnum = sendbase;
}
};