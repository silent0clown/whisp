#pragma once

#include <arpa/inet.h>
#include <stdint.h>

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
inline uint64_t htonll(uint64_t value) {
    return ((uint64_t)htonl(value & 0xFFFFFFFF) << 32) | htonl(value >> 32);
}

inline uint64_t ntohll(uint64_t value) {
    return ((uint64_t)ntohl(value & 0xFFFFFFFF) << 32) | ntohl(value >> 32);
}
#else
inline uint64_t htonll(uint64_t value) { return value; }
inline uint64_t ntohll(uint64_t value) { return value; }
#endif


namespace network
{
	namespace sockets
	{
		inline uint64_t hostToNetwork64(uint64_t host64)
		{
#ifdef WIN32
            return htonll(host64);
#else
			return htonll(host64);
#endif
		}

		inline uint32_t hostToNetwork32(uint32_t host32)
		{
#ifdef WIN32
            return htonl(host32);
#else
            return htonl(host32);
#endif
		}

		inline uint16_t hostToNetwork16(uint16_t host16)
		{
#ifdef WIN32		
			return htons(host16);
#else
            return htons(host16);
#endif
		}

		inline uint64_t networkToHost64(uint64_t net64)
		{
#ifdef WIN32
            return ntohll(net64);
#else
			return ntohll(net64);
#endif
		}

		inline uint32_t networkToHost32(uint32_t net32)
		{
#ifdef WIN32
            return ntohl(net32);
#else
			return ntohl(net32);
#endif
		}

		inline uint16_t networkToHost16(uint16_t net16)
		{
#ifdef WIN32
			return ntohs(net16);
#else
            return ntohs(net16);
#endif
		}
	}// end namespace sockets
}// end namespace net
