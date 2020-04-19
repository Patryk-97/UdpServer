#include "UdpClientSocket.h"

UdpClientSocket::UdpClientSocket()
{
   this->ipProtocol = IpProtocol::IPV4;
   this->socketId = INVALID_SOCKET;
   this->port = 0;
}

UdpClientSocket::~UdpClientSocket()
{
   this->reset();
}

void UdpClientSocket::reset(void)
{
   this->close();
   this->socketId = INVALID_SOCKET;
   this->port = 0;
}

bool UdpClientSocket::init(IpProtocol ipProtocol)
{
   // locals
   int family = AF_INET;
   int type = SOCK_DGRAM;
   int proto = IPPROTO_UDP;
   bool rV = true;
   std::unique_ptr<sockaddr_in> localSocketAddr = std::make_unique<sockaddr_in>();
   int localSocketAddrSize = sizeof(localSocketAddr);

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

   if (true == rV &&
      ::getsockname(this->socketId, (sockaddr*)localSocketAddr.get(),
         &localSocketAddrSize) == SOCKET_ERROR)
   {
      rV = false;
   }

   if (true == rV)
   {
      this->port = UdpClientSocket::convertPortFromNetworkEndianness(localSocketAddr.get());
      this->localAddressIp = UdpClientSocket::convertAddressIpToStr(localSocketAddr.get());
   }

   return rV;
}

bool UdpClientSocket::sendTo(const char* address, const uint16_t port, 
   const std::string& sendBuff, int& bytesSend)
{
   // locals
   bool rV = true;

   sockaddr_in socketAddr;

   this->fillIpProtocolFamily(&socketAddr);
   this->fillPort(port, &socketAddr);
   rV = this->fillNetworkAddressStructure(address, &socketAddr);

   bytesSend = ::sendto(this->socketId, sendBuff.c_str(), sendBuff.length() + 1, 0,
      (sockaddr*)&socketAddr, sizeof(socketAddr));

   if (bytesSend == SOCKET_ERROR)
   {
      rV = false;
   }

   return rV;
}
int UdpClientSocket::recvFrom(const char* address, const uint16_t port, char* recvBuff,
   int recvBuffSize)
{
   // locals
   int rV = 0;

   sockaddr_in socketAddr;
   int socketAddrSize = sizeof(socketAddr);

   this->fillIpProtocolFamily(&socketAddr);
   this->fillPort(port, &socketAddr);
   rV = this->fillNetworkAddressStructure(address, &socketAddr);

   memset(recvBuff, 0, recvBuffSize);
   rV = ::recvfrom(this->socketId, recvBuff, recvBuffSize, 0,
      (sockaddr*)&socketAddr, &socketAddrSize);

   return rV;
}

void UdpClientSocket::close()
{
   if (this->socketId != INVALID_SOCKET)
   {
      ::closesocket(this->socketId);
   }
}

std::string UdpClientSocket::getIpProtocolStr(void) const
{
   // locals
   std::string rV;

   rV = this->ipProtocol == IpProtocol::IPV4 ? "IPV4" : "IPV6";
   return rV;
}

IpProtocol UdpClientSocket::getIpProtocol(void) const
{
   return this->ipProtocol;
}

void UdpClientSocket::setPort(uint16_t port)
{
   this->port = port;
}

uint16_t UdpClientSocket::getPort(void) const
{
   return this->port;
}

void UdpClientSocket::setLocalAddressIp(const char* localAddressIp)
{
   this->localAddressIp = localAddressIp;
}

std::string UdpClientSocket::getLocalAddressIp(void) const
{
   return this->localAddressIp;
}

void UdpClientSocket::fillAddrInfoCriteria(addrinfo* hints) const
{
   memset(hints, 0, sizeof(*hints));
   hints->ai_socktype = SOCK_DGRAM;
   hints->ai_family = this->ipProtocol == IpProtocol::IPV4 ? AF_INET : AF_INET6;
}

bool UdpClientSocket::fillNetworkAddressStructure(const char* address, sockaddr_in* socketAddr)
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

void UdpClientSocket::fillPort(uint16_t port, sockaddr_in* socketAddr)
{
   socketAddr->sin_port = htons(port);
}

void UdpClientSocket::fillIpProtocolFamily(sockaddr_in* socketAddr)
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

std::string UdpClientSocket::convertAddressIpToStr(const sockaddr_in* socketAddr)
{
   // locals
   std::string addressIp = "\"";

   addressIp += std::to_string(socketAddr->sin_addr.S_un.S_un_b.s_b1) + ".";
   addressIp += std::to_string(socketAddr->sin_addr.S_un.S_un_b.s_b2) + ".";
   addressIp += std::to_string(socketAddr->sin_addr.S_un.S_un_b.s_b3) + ".";
   addressIp += std::to_string(socketAddr->sin_addr.S_un.S_un_b.s_b4) + "\"";

   return addressIp;
}

uint16_t UdpClientSocket::convertPortFromNetworkEndianness(const sockaddr_in* socketAddr)
{
   // locals
   uint16_t port = (socketAddr->sin_port & 0xFF00) >> 8 | (socketAddr->sin_port & 0x00FF) << 8;

   return port;
}