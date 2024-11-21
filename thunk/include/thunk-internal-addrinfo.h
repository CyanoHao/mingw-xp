#include "thunk-internal-memory.h"
#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

namespace mingw_thunk::internal
{
	inline addrinfo *addrinfo_from_hostent(const hostent *host,
	                                       int flags,
	                                       int family,
	                                       int socktype,
	                                       int protocol,
	                                       int port,
	                                       int &wsa_error)
	{
		wsa_error = 0;

		int n_addr = 0;
		while (host->h_addr_list[n_addr] != nullptr)
			n_addr++;

		addrinfo *head = nullptr;
		addrinfo *next = nullptr;
		for (int i = n_addr - 1; i >= 0; i--) {
			head = (addrinfo *)internal::_win32_malloc(sizeof(addrinfo));
			if (!head) {
				freeaddrinfo(next);
				wsa_error = WSA_NOT_ENOUGH_MEMORY;
				return nullptr;
			}
			head->ai_flags = flags;
			head->ai_family = family;
			head->ai_socktype = socktype;
			head->ai_protocol = protocol;
			head->ai_canonname = nullptr;
			head->ai_addrlen = sizeof(sockaddr_in);
			sockaddr_in *addr =
			    (sockaddr_in *)internal::_win32_malloc(sizeof(sockaddr_in));
			if (!addr) {
				internal::_win32_free(head);
				freeaddrinfo(next);
				wsa_error = WSA_NOT_ENOUGH_MEMORY;
				return nullptr;
			}
			addr->sin_family = AF_INET;
			addr->sin_port = htons(port);
			addr->sin_addr = *(in_addr *)host->h_addr_list[n_addr];
			memset(addr->sin_zero, 0, sizeof(addr->sin_zero));
			head->ai_addr = (sockaddr *)addr;
			head->ai_next = next;
			next = head;
		}
		return head;
	}
} // namespace mingw_thunk::internal
