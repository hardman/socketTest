//
//  main.cpp
//  socketServer
//
//  Created by wanghongyu on 17/5/15.
//  Copyright (c) 2015年 kaso. All rights reserved.
//

#include "socketHeader.h"

int socketFd;
#define max_bind_num 100

int currFds = 0;

void waitAccept(){
    PollfdHandler pfdHandler(max_bind_num);
    pfdHandler.addOneFd(socketFd, POLLIN | POLLOUT | POLLERR);
    
    while (true) {
        poll(pfdHandler.fds, pfdHandler.getCount(), -1);
        for (int i = 0; i < pfdHandler.getCount(); i++) {
            if(pfdHandler.fds[i].fd == socketFd){
                if(pfdHandler.fds[i].revents){
                    int clientFd = accept(socketFd, NULL, NULL);
                    if (clientFd < 0 && errno == EAGAIN) {
                        //errorlog("accept");
                    }else{
                        pfdHandler.addOneFd(clientFd, POLLIN | POLLOUT | POLLERR);
                        debuglog("one client connected! i = %d\n", i);
                    }
                }
            }else{
                if(pfdHandler.fds[i].revents & POLLIN){
                    debuglog("POLLIN! i = %d\n", i);
                    char buf[4096], bf[512];
                    memset(buf, 0, 4096);
                    memset(bf, 0, 512);
                    ssize_t len = -1;
                    char *p = buf;
                    while ((len = read(pfdHandler.fds[i].fd, bf, 512)) > 0) {
                        memcpy(p, bf, len);
                        p += len;
                    }
                    if(len == 0 || (len < 0 && errno == EAGAIN)){
                        debuglog("client receive msg ok, msg=%s \n", buf);
                    }else if(len < 0){
                        debuglog("client receive msg then remove client \n");
                        pfdHandler.removeOneFd(pfdHandler.fds[i].fd);
                    }
                }
                if(pfdHandler.fds[i].revents & POLLOUT){
                    //debuglog("POLLOUT i = %d!\n", i);
                }
                if(pfdHandler.fds[i].revents & POLLERR){
                    debuglog("error i = %d, ", i);
                    errorlog("POLLERR! then remove client!\n");
                    pfdHandler.removeOneFd(pfdHandler.fds[i].fd);
                }
            }
        }
    }
}

void runServer(const char *host, int port){
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd < 0) {
        errorlog("create socket");
        return;
    }
    
    setNonBlock(socketFd);
    
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(host);
    serverAddr.sin_port = htons(port);
    
    if(bindSock(socketFd, &serverAddr, sizeof(serverAddr)) < 0){
        errorlog("bind socket");
        return;
    }
    
    if (listen(socketFd, max_bind_num) < 0) {
        errorlog("listen socket");
        return;
    }
    
    waitAccept();
}

int main(int argc, const char * argv[]) {
    if (checkArgs(argc, argv) && useSig()) {
        runServer(argv[1], atoi(argv[2]));
    }
    return 0;
}
