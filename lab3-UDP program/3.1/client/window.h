struct ackwindow{
char*pkt;//���÷��͵İ� ������ʱ�������� ���������ֺͻ��� 
bool ACK;
bool NAK;
int length;
bool last_flag;
int serial_num;
ackwindow(){
    ACK = false;
    NAK = false;
    last_flag = false;
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