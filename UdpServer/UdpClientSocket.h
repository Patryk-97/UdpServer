#pragma once

#ifndef __UDP_CLIENT_SOCKET_H__
#define __UDP_CLIENT_SOCKET_H__

#include "IpProtocol.h"

#include <stdint.h>
#include <string>
#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <memory>

class UdpClientSocket
{
public:
   UdpClientSocket();
   ~UdpClientSocket();
   void reset(void);
   bool init(IpProtocol ipProtocol);
   bool sendTo(const char* address, const uint16_t port, const std::string& sendBuff,
      int& bytesSend);
   int recvFrom(const char* address, const uint16_t port, char* recvBuff, int recvBuffSize);
   void close(void);

   std::string getIpProtocolStr(void) const;
   IpProtocol getIpProtocol(void) const;

   void setPort(uint16_t port);
   uint16_t getPort(void) const;
   void setLocalAddressIp(const char* localAddressIp);
   std::string getLocalAddressIp(void) const;

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
   uint16_t port;
   std::string localAddressIp;
};

#endif //__UDP_CLIENT_SOCKET_H__