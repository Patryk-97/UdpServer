#pragma once

#ifndef __WINSOCK_MANAGER_H__
#define __WINSOCK_MANAGER_H__

#include <winsock2.h>
#include <stdint.h>
#include <memory>
#include <string>

class WinsockManager
{
public:
   WinsockManager();
   ~WinsockManager();
   bool startup(uint16_t dllWinsockVersion);
   void cleanup(void);
   std::string getErrorMessage(void);

private:
   std::unique_ptr<WSADATA> wsaData;
};

#endif // __WINSOCK_MANAGER_H__
