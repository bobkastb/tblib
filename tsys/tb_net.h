#pragma once

#include "gdata/t_string.h"

namespace crsys{

struct Ip4Address{
	uint32 ip;
	operator uint32& () { return ip; }
	operator uint32() const { return ip; }
	Ip4Address& operator = (uint32 x) { ip=x; return *this; }
};
struct EthernetAddress{
	uint64 data;
	operator uint64& () { return data; }
	operator uint64() const { return data; }
	EthernetAddress& operator = (uint64 x) { data=x; return *this; }
};

struct r_net_interface_info{
	uint initmask;
	int inum;
	tbgeneral::stringA name;
	Ip4Address ip4;
	Ip4Address ip4mask;
	EthernetAddress ethernet_addr;
	int weight;
	r_net_interface_info() { inum=0;ip4=ip4mask=0;ethernet_addr=0; initmask=0;weight=0; };
};
typedef tbgeneral::darray<r_net_interface_info> d_net_interface_list;
d_net_interface_list get_netiflist();

tbgeneral::stringA ethernet_addr2a( const EthernetAddress & ethernet_addr );
tbgeneral::stringA ip4addr2a( const Ip4Address & ip_addr );

}; // namespace crsys