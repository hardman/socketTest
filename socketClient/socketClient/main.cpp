//
//  main.cpp
//  socket
//
//  Created by wanghongyu on 17/5/15.
//  Copyright (c) 2015年 kaso. All rights reserved.
//

#include "socketHeader.h"

int socketFd;

void waitToWrite(){
    struct pollfd pfd;
    memset(&pfd, 0, sizeof(pfd));
    pfd.fd = socketFd;
    pfd.events = POLLIN | POLLOUT | POLLERR;
    while (true) {
        int nfds = poll(&pfd, 1, -1);
        if (nfds == 1) {
            if(pfd.revents & POLLIN){
                debuglog("POLLIN!!\n");
                char buf[4096], bf[512];
                memset(buf, 0, 4096);
                memset(bf, 0, 512);
                ssize_t len = -1;
                char *p = buf;
                while ((len = read(socketFd, bf, 512)) > 0) {
                    memcpy(p, bf, len);
                    p += len;
                }
                if(len < 0 && errno == EAGAIN){
                    debuglog("client receive msg ok, msg=%s \n", buf);
                }else{
                    errorlog("client receive msg");
                    sleep(1);
                }
            }else if(pfd.revents & POLLOUT){
                debuglog("POLLOUT!!\n");
                string s;
                cin>>s;
                ssize_t len = write(socketFd, s.c_str(), s.size());
                debuglog("write ret=%zd\n", len);
                errorlog("write has some error?");
            }else if(pfd.revents & POLLERR){
                errorlog("POLLERROR!!");
            }
        }else if(nfds != 0){
            errorlog("some error when poll");
            break;
        }
    }
}

void connectServer(const char * ip, int port){
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd < 0) {
        errorlog("create socket error");
        return;
    }
    setNonBlock(socketFd);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &addr.sin_addr) < 0) {
        errorlog("inet_pton");
        return;
    }
    if (connect(socketFd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        if (errno != EINPROGRESS) {
            errorlog("connect socketFd");
            return;
        }
        errno = 0;
    }
    waitToWrite();
}

int main(int argc, const char * argv[]) {
    if(checkArgs(argc, argv) && useSig()){
        connectServer(argv[1], atoi(argv[2]));
    }
    
    return 0;
}
