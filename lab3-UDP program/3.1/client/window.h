//�������ڶ���:
struct ackwindow{
//���ڴ�Ż���
char*pkt;
bool ACK;
bool NAK;
int length;
int serial_num;
//���巢��ʱ��
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
//���ڴ�С������common.h�� : const int WINDOW_LEN = 1000;
queue<ackwindow> sw;
int sendbase;
int nextseqnum;
slide_window(){
    sendbase = 0;
    nextseqnum = sendbase;
}
};