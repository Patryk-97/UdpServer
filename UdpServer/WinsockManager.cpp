#include "WinsockManager.h"

WinsockManager::WinsockManager()
{
   this->wsaData = std::make_unique<WSAData>();
}

WinsockManager::~WinsockManager()
{

}

bool WinsockManager::startup(uint16_t dllWinsockVersion)
{
   // locals
   bool rV = true;

   if (WSAStartup(dllWinsockVersion, this->wsaData.get()) != NO_ERROR)
   {
      rV = false;
   }

   return rV;
}

void WinsockManager::cleanup(void)
{
   WSACleanup();
}

std::string WinsockManager::getErrorMessage(void)
{
   // locals
   std::string rV;
   int errorNo = WSAGetLastError();

   switch (errorNo)
   {
      case WSAEFAULT:
      {
         rV = "Bad pointer. Probably you passed wrong length of size sockaddr structure and it is smaller ";
         rV += "than the sizeof(sockaddr). Check functions like getsockname().";
         break;
      }
      case WSAEADDRNOTAVAIL:
      {
         rV = "Propably you choose port 0.";
         break;
      }
      case WSAECONNRESET:
      {
         rV = "Connection reset by peer. An existing connection was forcibly closed by the remote host.";
         break;
      }
      case WSAETIMEDOUT:
      {
         rV = "Connection timed out.";
         break;
      }
      case WSAECONNREFUSED:
      {
         rV = "Connection refused. Probably wrong port";
         break;
      }
      case WSAHOST_NOT_FOUND:
      {
         rV = "Host not found. Wrong ip address or DNS address.";
         break;
      }
      default:
      {
         rV = "Unsupported error #" + std::to_string(errorNo);
         break;
      }
   }

   return rV;
}