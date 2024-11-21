#include "thunk-common.h"
#include "thunk-internal-memory.h"

#include <ws2tcpip.h>

namespace mingw_thunk
{
	__DEFINE_THUNK(kernel32,
	               16,
	               INT,
	               WSAAPI,
	               getaddrinfo,
	               _In_opt_ PCSTR pNodeName,
	               _In_opt_ PCSTR pServiceName,
	               _In_opt_ const ADDRINFOA *pHints,
	               _Out_ PADDRINFOA *ppResult)
	{
		if (auto pfreeaddrinfo = try_get_getaddrinfo())
			return pfreeaddrinfo(pNodeName, pServiceName, pHints, ppResult);

		if (!pNodeName)
			return WSAHOST_NOT_FOUND;
		if (pHints &&
		    (pHints->ai_family != AF_UNSPEC && pHints->ai_family != AF_INET))
			return WSAEAFNOSUPPORT;

		int port = 0;
		if (pServiceName)
			port = atoi(pServiceName);

		struct hostent *host = gethostbyname(pNodeName);
		if (!host)
			return WSANO_DATA;

		addrinfo *result = internal::addrinfo_from_hostent(host, port);
		if (!result)
			return WSA_NOT_ENOUGH_MEMORY;
		*ppResult = result;
		return 0;
	}
} // namespace mingw_thunk
