#pragma once

#ifndef __UDP_SERVER_SOCKET_H__
#define __UDP_SERVER_SOCKET_H__

#include "IpProtocol.h"
#include "UdpClientSocket.h"

#include <stdint.h>
#include <string>
#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <memory>
#include <algorithm>

class UdpServerSocket
{
public:
   UdpServerSocket();
   ~UdpServerSocket();
   void reset(void);
   bool init(IpProtocol ipProtocol);
   bool bind(const char* address, const uint16_t port);
   bool sendTo(const std::string& sendBuff, int& bytesSend, const UdpClientSocket* udpClient);
   UdpClientSocket* recvFrom(char* recvBuff, int recvBuffSize, int& bytesReceived);
   void close(void);
   std::string getIpProtocolStr(void) const;
   IpProtocol getIpProtocol(void) const;

private:
   void fillAddrInfoCriteria(addrinfo* hints) const;
   bool fillNetworkAddressStructure(const char* address, sockaddr_in* socketAddr);
   void fillPort(uint16_t port, sockaddr_in* socketAddr);
   void fillIpProtocolFamily(sockaddr_in* socketAddr);
   static std::string convertAddressIpToStr(const sockaddr_in* socketAddr);
   static uint16_t convertPortFromNetworkEndianness(const sockaddr_in* socketAddr);

private:
   SOCKET socketId;
   IpProtocol ipProtocol;
   std::unique_ptr<sockaddr_in> socketAddr;
};

#endif //__UDP_SERVER_SOCKET_H__