#include <lib.h> 
void handler1(int num){
    debugf("Reach handler1\n");
    kill(0, 2);
    debugf("Leave handler1\n");
}

void handler2(int num){
    debugf("Reach handler2\n");
    debugf("Leave handler2\n");
}

int main(int argc, char **argv) {
    //注册信号1，要求信号1在处理过程中屏蔽信号2
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, 2);
    act.sa_handler = handler1;
    sigaction(1, &act, NULL);
    //注册信号2
    act.sa_handler = handler2;
    sigemptyset(&act.sa_mask);
    sigaction(2, &act, NULL);

    kill(0, 1);
    return 0;
}       
