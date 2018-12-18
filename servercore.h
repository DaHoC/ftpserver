#ifndef _SERVERCORE_H
#define	_SERVERCORE_H

#include "serverconnection.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

class servercore {
public:
    servercore(uint port,std::string dir, unsigned short commandOffset = 1);
    ~servercore();

private:
    int start();
    void initSockets(int port);
    void setNonBlocking(int &sock);
    void buildSelectList();
    void readSockets();
    int handleNewConnection();
    void freeAllConnections();
    unsigned int maxConnectionsInQuery; // number of connections in query
    int s; // The main listening socket file descriptor
    int sflags; // Socket fd flags
    std::vector<serverconnection*> connections;// Manage the connected sockets / connections in a list with an iterator
    int highSock; // Highest #'d file descriptor, needed for select()
    fd_set socks; // set of socket file descriptors we want to wake up for, using select()
    std::string dir;
    unsigned int connId;
    bool shutdown;
    sockaddr_in addr;
    struct sockaddr_storage addrStorage;
    socklen_t addrLength;
    sockaddr_in cli;
    socklen_t cli_size;
    unsigned short commandOffset;
};

#endif	/* _SERVERCORE_H */

