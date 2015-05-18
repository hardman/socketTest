#ifndef __SOCKET_HEADER_H__
#define __SOCKET_HEADER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <sys/poll.h>

int bindSock(int fd, struct sockaddr_in *addr, socklen_t len){
    return bind(fd, (struct sockaddr *)addr, len);
}

#include <iostream>
#include <vector>

using namespace std;

extern int socketFd;

void sighandler(int sig){
    if (sig == SIGINT) {
        printf("receive SIGINT \n");
    }else if(sig == SIGTERM){
        printf("receive SIGTERM \n");
    }else if(sig == SIGQUIT){
        printf("receive SIGQUIT \n");
    }else{
        printf("receive unknow sig= %d\n", sig);
    }
    close(socketFd);
    exit(1);
}

bool useSig(){
    struct sigaction sig;
    sig.sa_handler = sighandler;
    sig.sa_flags = 0;
    sigemptyset(&sig.sa_mask);
    
    sigaction(SIGTERM, &sig, NULL);
    sigaction(SIGINT, &sig, NULL);
    sigaction(SIGQUIT, &sig, NULL);
    
    if (signal(SIGPIPE, SIG_IGN)) {
        printf("ignore sigpipe \n");
        return false;
    }
    
    return true;
}

bool setNonBlock(int &sock){
    int flag = -1;
    if ((flag = fcntl(sock, F_GETFL, 0)) < 0) {
        printf("get sock flag error=%s\n", strerror(errno));
        return false;
    }
    if ((fcntl(sock, F_SETFL, flag | O_NONBLOCK)) < 0) {
        printf("set sock flag error=%s\n", strerror(errno));
        return false;
    }
    return true;
}

bool checkArgs(int argc, const char **argv){
    if (argc != 3) {
        printf("usage: socketClient ip port\n");
        return false;
    }
    
    if (0 == atoi(argv[2])) {
        printf("port error!");
        return false;
    }
    return true;
}

#define debuglog(...) \
    printf(__VA_ARGS__)

void errorlog(const char *msg){
    printf("%s: errno=[%d], err=[%s]\n", msg, errno, strerror(errno));
}

struct PollfdHandler{
private:
    int _num;
    int _count;
public:
    struct pollfd *fds;
    PollfdHandler(int num): _num(num), _count(0){
        fds = new struct pollfd [num];
        memset(fds, 0, sizeof(struct pollfd) * num);
    }
    ~PollfdHandler(){
        delete [] fds;
    }
    bool addOneFd(int fd, int flags){
        for(int i = 0; i < _num; i++){
            if(fds[i].fd == 0 && fds[i].events == 0){
                fds[i].fd = fd;
                fds[i].events = flags;
                _count++;
                return true;
            }
        }
        return false;
    }
    
    bool removeOneFd(int fd){
        int i = 0;
        for(; i < _count; i++){
            if(fds[i].fd == fd){
                break;
            }
        }
        
        if(i >= _count){
            return false;
        }
        
        if(i + 1 < _count){
            memcpy(&fds[i], &fds[i + 1], sizeof(struct pollfd) * (_count - i - 1));
        }
        
        if(i + 1 != _count){
            memset(&fds[_count - 1], 0, sizeof(struct pollfd));
        }
        
        _count--;
        
        return true;
    }
    
    int getCount(){
        return _count;
    }
private:
    PollfdHandler(const PollfdHandler &){}
    PollfdHandler& operator=(const PollfdHandler &){return *this;}
};

#endif
