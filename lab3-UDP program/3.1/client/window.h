//滑动窗口定义:
struct ackwindow{
//用于存放缓存
char*pkt;
bool ACK;
bool NAK;
int length;
int serial_num;
//定义发送时间
int sendtime;
ackwindow(){
    ACK = false;
    NAK = false;
}
ackwindow(char*pkt,int length,int serial_num){
    this->pkt = pkt;
    this->length = length;
    this->serial_num = serial_num;
} 
};
struct slide_window{
//窗口大小定义在common.h中 : const int WINDOW_LEN = 1000;
queue<ackwindow> sw;
int sendbase;
int nextseqnum;
slide_window(){
    sendbase = 0;
    nextseqnum = sendbase;
}
};