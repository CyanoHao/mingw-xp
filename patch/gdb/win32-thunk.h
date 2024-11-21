#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <type_traits>

namespace Win32Thunk {
  inline void WSAAPI freeaddrinfo(
    _In_ PADDRINFOA pAddrInfo
  ) {
    using type = decltype(&::freeaddrinfo);
    static type real = (type)GetProcAddress(GetModuleHandleW(L"ws2_32.dll"), "freeaddrinfo");
    if (real)
      return real(pAddrInfo);

    while (pAddrInfo) {
      addrinfo *next = pAddrInfo->ai_next;
      free(pAddrInfo->ai_addr);
      free(pAddrInfo);
      pAddrInfo = next;
    }
  }

  namespace Detail {
    inline addrinfo *AddrInfoFromHostent(const hostent *host, int port, int idx) {
      // this is the last entry
      if (host->h_addr_list[idx] == nullptr)
        return nullptr;

      addrinfo *next = AddrInfoFromHostent(host, port, idx + 1);

      addrinfo *result = (addrinfo *)malloc(sizeof(addrinfo));
      if (!result) {
        Win32Thunk::freeaddrinfo(next);
        return nullptr;
      }

      result->ai_flags = 0;
      result->ai_family = AF_INET;
      result->ai_socktype = SOCK_STREAM;
      result->ai_protocol = IPPROTO_TCP;
      result->ai_canonname = nullptr;
      result->ai_addrlen = sizeof(sockaddr_in);

      sockaddr_in *addr = (sockaddr_in *)malloc(sizeof(sockaddr_in));
      if (!addr) {
        Win32Thunk::freeaddrinfo(result);
        Win32Thunk::freeaddrinfo(next);
        return nullptr;
      }

      addr->sin_family = AF_INET;
      addr->sin_port = htons(port);
      addr->sin_addr = *(in_addr *)host->h_addr_list[idx];
      memset(addr->sin_zero, 0, sizeof(addr->sin_zero));
      result->ai_addr = (sockaddr *)addr;

      result->ai_next = next;
      return result;
    }
  }

  inline INT WSAAPI getaddrinfo(
    _In_opt_ PCSTR pNodeName,
    _In_opt_ PCSTR pServiceName,
    _In_opt_ const ADDRINFOA *pHints,
    _Out_ PADDRINFOA *ppResult
  ) {
    using type = decltype(&::getaddrinfo);
    static type real = (type)GetProcAddress(GetModuleHandleW(L"ws2_32.dll"), "getaddrinfo");
    if (real)
      return real(pNodeName, pServiceName, pHints, ppResult);

    if (!pNodeName)
      return WSAHOST_NOT_FOUND;
    if (pHints && pHints->ai_family != AF_UNSPEC)
      return WSAEAFNOSUPPORT;

    int port = 0;
    if (pServiceName)
      port = atoi(pServiceName);

    struct hostent *host = gethostbyname(pNodeName);
    if (!host)
      return WSANO_DATA;

    addrinfo *result = Detail::AddrInfoFromHostent(host, port, 0);
    if (!result)
      return WSA_NOT_ENOUGH_MEMORY;
    *ppResult = result;
    return 0;
  }

  inline INT WSAAPI getnameinfo(
    _In_ const SOCKADDR *pSockaddr,
    _In_ socklen_t SockaddrLength,
    _Out_ PCHAR pNodeBuffer,
    _In_ DWORD NodeBufferLength,
    _Out_ PCHAR pServiceBuffer,
    _In_ DWORD ServiceBufferLength,
    _In_ INT Flags
  ) {
    using type = decltype(&::getnameinfo);
    static type real = (type)GetProcAddress(GetModuleHandleW(L"ws2_32.dll"), "getnameinfo");
    if (real)
      return real(pSockaddr, SockaddrLength, pNodeBuffer, NodeBufferLength, pServiceBuffer, ServiceBufferLength, Flags);

    if (pServiceBuffer && ServiceBufferLength > 0)
      snprintf(pServiceBuffer, ServiceBufferLength, "%d", ntohs(((sockaddr_in *)pSockaddr)->sin_port));

    if (SockaddrLength == sizeof(sockaddr_in)) {
      const sockaddr_in *addr = (const sockaddr_in *)pSockaddr;
      hostent *host = gethostbyaddr((const char *)&addr->sin_addr, sizeof(in_addr), AF_INET);
      if (!host)
        return WSANO_DATA;

      if (pNodeBuffer && NodeBufferLength > 0) {
        strncpy(pNodeBuffer, host->h_name, NodeBufferLength);
        pNodeBuffer[NodeBufferLength - 1] = '\0';
      }
      return 0;
    } else if (SockaddrLength == sizeof(sockaddr_in6)) {
      const sockaddr_in6 *addr = (const sockaddr_in6 *)pSockaddr;
      hostent *host = gethostbyaddr((const char *)&addr->sin6_addr, sizeof(in6_addr), AF_INET6);
      if (!host)
        return WSANO_DATA;

      if (pNodeBuffer && NodeBufferLength > 0) {
        strncpy(pNodeBuffer, host->h_name, NodeBufferLength);
        pNodeBuffer[NodeBufferLength - 1] = '\0';
      }
      return 0;
    } else {
      return WSAEFAULT;
    }
  }
}
