#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream>

#if !defined(_WIN32)
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#endif

#define STREAMCLIENT_OK		(0)
#define STREAMCLIENT_FAIL		(-1)

class StreamClient
{

public:

    StreamClient(void) {
    
    }

    ~StreamClient() {
        // close the socket
        if (Socket != INVALID_SOCKET) {
#if !defined(_WIN32)
            close(Socket);
#else
            closesocket(Socket);
#endif
        
        }
#if defined(_WIN32)
        // clean up the windows
        WSACleanup();
#endif
    }

    std::string  ErrorString;

    // methods
    void Init(char* SERVER_ADDRESS, char* TCPPORT, const char* DrvName) {
    
        struct sockaddr_in servaddr_tcp;
        unsigned short tcp_port;
        long arg;
        fd_set myset;
        struct timeval tv;
        socklen_t len;
#if !defined(_WIN32)
        int valopt;
#else
        char valopt;
#endif

#if defined(_WIN32)
        WORD wVersionRequested;
        WSADATA wsaData;
        int err;
        wVersionRequested = MAKEWORD(2, 2);
        err = WSAStartup(wVersionRequested, &wsaData);
        if (err != 0) {
            std::cout << "Cannot find any usable dll !\n";
            return;
        }
#endif

        this->Socket = INVALID_SOCKET;

        // copy name
        Name = std::string(DrvName);

        // convert to short
        std::string str(TCPPORT);
        std::istringstream sstr(str);
        sstr >> tcp_port;

        // init sockaddr
        servaddr_tcp = {};
        servaddr_tcp.sin_family = AF_INET;
        servaddr_tcp.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
        servaddr_tcp.sin_port = htons(tcp_port); // next port

        // connect to socket
        Socket = socket(AF_INET, SOCK_STREAM, 0);


        // Set non-blocking 
#if !defined(_WIN32)
        arg = fcntl(Socket, F_GETFL, NULL);
        arg |= O_NONBLOCK;
        fcntl(Socket, F_SETFL, arg);
#else
        //ioctlsocket(Socket, FIONBIO, (u_long*)(&arg));
#endif

        if (connect(Socket, (struct sockaddr*) & servaddr_tcp, sizeof(servaddr_tcp)) < 0) {
            if (errno == EINPROGRESS) {
                tv.tv_sec = 15;
                tv.tv_usec = 0;
                FD_ZERO(&myset);
                FD_SET(Socket, &myset);
                if (select(Socket + 1, NULL, &myset, NULL, &tv) > 0) {
                    len = sizeof(int);
#if !defined(_WIN32)
                    getsockopt(Socket, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &len);
#else
                    getsockopt(Socket, SOL_SOCKET, SO_ERROR, &valopt, &len);
#endif
                    if (valopt) {
                        Invalidate();
                    }
                }
                else {
                    Invalidate();
                }
            }
            else {
                Invalidate();
            }
        }

        // Set to blocking mode again... 
#if !defined(_WIN32)
        arg = fcntl(Socket, F_GETFL, NULL);
        arg |= O_NONBLOCK;
        fcntl(Socket, F_SETFL, arg);
#else
       // ioctlsocket(Socket, FIONBIO, (u_long*)(&arg));
#endif
    }

    int SendData(unsigned char* data, int length, bool throwexcept) {
    
        if (this->Socket == INVALID_SOCKET) return STREAMCLIENT_FAIL;

        int ir = send(Socket, (const char*)data, length, 0);
        if (ir == SOCKET_ERROR) {
            throw ErrorString.c_str();
        }
        return STREAMCLIENT_OK;
    }


    int ReceiveData(unsigned char* data, int br, int waitseconds, bool throwexcept) {
    
        int ir, Retries;
        struct timeval tv;
        fd_set rfds;

        if (this->Socket == INVALID_SOCKET) return STREAMCLIENT_FAIL;

        FD_ZERO(&rfds);
        FD_SET(Socket, &rfds);

        tv.tv_sec = waitseconds;
        tv.tv_usec = 0;
        int size = br;

        Retries = 32;

        char* zzP = (char*)data;
        while (br) {

            ir = select(Socket + 1, &rfds, NULL, NULL, &tv);

            if (ir < 0) {
                return STREAMCLIENT_FAIL;
            }
            else if (ir == 0) {
                return (size - br);
            }
            else if (FD_ISSET(Socket, &rfds)) {
                ir = recv(Socket, (char*)zzP, br, 0); //MSG_WAITALL);
                if (ir < 0) {
                    Invalidate();
                    throw ErrorString.c_str();
                }
                else if (ir == 0) {
                    if ((Retries--) == 0) {
                        Invalidate();
                        if (throwexcept)
                        return ir;
                    }
                }
            }
            br = br - ir;
            zzP += ir;
        }

        return size;
    }

private:
    int Socket;	//socket num
    std::string  Name;
    char* ServerIP;
    unsigned short Port;



};

