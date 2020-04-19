#include "UdpServerSocket.h"

UdpServerSocket::UdpServerSocket()
{
   this->ipProtocol = IpProtocol::UNDEFINED;
   this->socketId = INVALID_SOCKET;
   this->socketAddr = std::make_unique<sockaddr_in>();
}

UdpServerSocket::~UdpServerSocket()
{
   this->reset();
}

void UdpServerSocket::reset(void)
{
   this->close();
   this->socketId = INVALID_SOCKET;
   this->socketAddr.reset(new sockaddr_in());
}

bool UdpServerSocket::init(IpProtocol ipProtocol)
{
   // locals
   int family = AF_INET;
   int type = SOCK_DGRAM;
   int proto = IPPROTO_UDP;
   bool rV = true;

   if (ipProtocol == IpProtocol::IPV6)
   {
      family = AF_INET6;
      this->ipProtocol = IpProtocol::IPV6;
   }

   this->socketId = ::socket(family, type, proto);
   if (this->socketId == INVALID_SOCKET)
   {
      rV = false;
   }
   return rV;
}

bool UdpServerSocket::bind(const char* address, const uint16_t port)
{
   // locals
   bool rV = true;

   this->fillIpProtocolFamily(this->socketAddr.get());
   this->fillPort(port, this->socketAddr.get());
   rV = this->fillNetworkAddressStructure(address, this->socketAddr.get());

   if (true == rV &&
      ::bind(this->socketId, (sockaddr*)this->socketAddr.get(), sizeof(*this->socketAddr)) == SOCKET_ERROR)
   {
      rV = false;
   }

   return rV;
}

bool UdpServerSocket::sendTo(const std::string& sendBuff, int& bytesSend,
   const UdpClientSocket* udpClient)
{
   // locals
   bool rV = true;

   sockaddr_in socketAddr;

   this->fillIpProtocolFamily(&socketAddr);
   this->fillPort(udpClient->getPort(), &socketAddr);
   rV = this->fillNetworkAddressStructure(udpClient->getLocalAddressIp().c_str(), &socketAddr);

   if (true == rV)
   {
      bytesSend = ::sendto(this->socketId, sendBuff.c_str(), sendBuff.length() + 1, 0,
         (sockaddr*)&socketAddr, sizeof(socketAddr));

      if (SOCKET_ERROR == bytesSend || 0 == bytesSend)
      {
         rV = false;
      }
   }

   return rV;
}
UdpClientSocket* UdpServerSocket::recvFrom(char* recvBuff, int recvBuffSize, int& bytesReceived)
{
   // locals
   UdpClientSocket* udpClientSocket = nullptr;

   sockaddr_in socketAddr;
   int socketAddrSize = sizeof(socketAddr);

   memset(recvBuff, 0, recvBuffSize);
   bytesReceived = ::recvfrom(this->socketId, recvBuff, recvBuffSize, 0,
      (sockaddr*)&socketAddr, &socketAddrSize);

   if (bytesReceived > 0)
   {
      udpClientSocket = new UdpClientSocket();
      udpClientSocket->setPort(UdpServerSocket::convertPortFromNetworkEndianness(&socketAddr));
      udpClientSocket->setLocalAddressIp(
         UdpServerSocket::convertAddressIpToStr(&socketAddr).c_str());
   }

   return udpClientSocket;
}

void UdpServerSocket::close()
{
   if (this->socketId != INVALID_SOCKET)
   {
      ::closesocket(this->socketId);
   }
}

std::string UdpServerSocket::getIpProtocolStr(void) const
{
   // locals
   std::string rV;

   rV = this->ipProtocol == IpProtocol::IPV4 ? "IPV4" : "IPV6";
   return rV;
}

IpProtocol UdpServerSocket::getIpProtocol(void) const
{
   return this->ipProtocol;
}

void UdpServerSocket::fillAddrInfoCriteria(addrinfo* hints) const
{
   memset(hints, 0, sizeof(*hints));
   hints->ai_socktype = SOCK_DGRAM;
   hints->ai_family = this->ipProtocol == IpProtocol::IPV4 ? AF_INET : AF_INET6;
}

bool UdpServerSocket::fillNetworkAddressStructure(const char* address, sockaddr_in* socketAddr)
{
   // locals
   bool rV = true;
   std::unique_ptr<addrinfo> hints = std::make_unique<addrinfo>();
   addrinfo* res = nullptr;

   if (address == nullptr)
   {
      socketAddr->sin_addr.s_addr = INADDR_ANY;
   }
   else if (address[0] >= '0' && address[0] <= '9')
   {
      if (::inet_pton(AF_INET, address, &socketAddr->sin_addr) != 1)
      {
         rV = false;
      }
   }
   else
   {
      this->fillAddrInfoCriteria(hints.get());
      if (0 != ::getaddrinfo(address, nullptr, hints.get(), &res))
      {
         rV = false;
      }
      else
      {
         socketAddr->sin_addr.S_un = ((sockaddr_in*)(res->ai_addr))->sin_addr.S_un;
         ::freeaddrinfo(res);
      }
   }

   return rV;
}

void UdpServerSocket::fillPort(uint16_t port, sockaddr_in* socketAddr)
{
   socketAddr->sin_port = htons(port);
}

void UdpServerSocket::fillIpProtocolFamily(sockaddr_in* socketAddr)
{
   if (this->ipProtocol == IpProtocol::IPV4)
   {
      socketAddr->sin_family = AF_INET;
   }
   else
   {
      socketAddr->sin_family = AF_INET6;
   }
}

std::string UdpServerSocket::convertAddressIpToStr(const sockaddr_in* socketAddr)
{
   // locals
   std::string addressIp = "\"";

   addressIp += std::to_string(socketAddr->sin_addr.S_un.S_un_b.s_b1) + ".";
   addressIp += std::to_string(socketAddr->sin_addr.S_un.S_un_b.s_b2) + ".";
   addressIp += std::to_string(socketAddr->sin_addr.S_un.S_un_b.s_b3) + ".";
   addressIp += std::to_string(socketAddr->sin_addr.S_un.S_un_b.s_b4) + "\"";

   return addressIp;
}

uint16_t UdpServerSocket::convertPortFromNetworkEndianness(const sockaddr_in* socketAddr)
{
   // locals
   uint16_t port = (socketAddr->sin_port & 0xFF00) >> 8 | (socketAddr->sin_port & 0x00FF) << 8;

   return port;
}