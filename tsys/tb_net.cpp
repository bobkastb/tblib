#define _CROSS_SYS_COD

#include "tsys/tb_net.h"
#include "gdata/tb_algo.h"
#include "tsys/tb_sysdecls.h"

#ifdef _WIN32
#define LEAN_AND_MEAN
//#include "windows.h"
#include "winsock2.h"
#include "Iphlpapi.h"

#pragma comment(lib, "Iphlpapi.lib" )
#pragma comment(lib, "Ws2_32.lib" )

#else 
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <unistd.h>
#endif

warning_MSVC(disable,4996)


typedef tbgeneral::stringA string;
namespace crsys{

using namespace tbgeneral;

tbgeneral::stringA ethernet_addr2a( const EthernetAddress & ethernet_addr );
tbgeneral::stringA ip4addr2a( const Ip4Address & ip_addr );

tbgeneral::stringA ethernet_addr2a( const EthernetAddress & ethernet_addr){
	tbgeneral::stringA res;
	uint8 * p=(uint8 *)&ethernet_addr ; //, *e=p+6;
	char buff[100];
	sprintf(buff,"%02x:%02x:%02x:%02x:%02x:%02x" , p[0],p[1],p[2],p[3],p[4],p[5] );
	return buff;
}
tbgeneral::stringA ip4addr2a( const Ip4Address & ip_addr ){
	tbgeneral::stringA res;
	uint8 * p=(uint8 *)&ip_addr ; //, *e=p+6;
	char buff[100];
	sprintf(buff,"%d.%d.%d.%d" , p[0],p[1],p[2],p[3] );
	return buff;
};
/*
void print_r_net_interface_info( const r_net_interface_info & rinf ){
	printf("Device %d.%s (%x) -> Ethernet %s IP4=%s mask=%s \n", rinf.inum, rinf.name.c_str(), rinf.initmask,
		ethernet_addr2a(rinf.ethernet_addr).c_str() , ip4addr2a(rinf.ip4).c_str() , ip4addr2a(rinf.ip4mask).c_str() );
};
*/

#ifdef _WIN32


d_net_interface_list get_netiflist(){
	d_net_interface_list res; DWORD cnt; tbgeneral::darray<byte> buff;
	if (GetNumberOfInterfaces( &cnt )) return res;

	
	PIP_ADAPTER_INFO pAdapterInfo ;	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	buff.resize(ulOutBufLen); pAdapterInfo = (PIP_ADAPTER_INFO) &buff[0];
	if (GetAdaptersInfo( pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		buff.resize(ulOutBufLen); pAdapterInfo = (PIP_ADAPTER_INFO) &buff[0];
	};
	if (GetAdaptersInfo( pAdapterInfo, &ulOutBufLen) == NO_ERROR) {

		for (uint n=0;pAdapterInfo;n++,pAdapterInfo=pAdapterInfo->Next) {
			if (pAdapterInfo->Type != MIB_IF_TYPE_ETHERNET) continue;
			r_net_interface_info rinf;
			rinf.initmask = 0xF;
			rinf.inum = n;
			char * defgate=pAdapterInfo->GatewayList.IpAddress.String;
			if (defgate[0] && (0!=strcmp(defgate,"0.0.0.0"))) rinf.weight += 0x10;
			memcpy( &rinf.ethernet_addr , pAdapterInfo->Address , 
				tbgeneral::min<uint>(sizeof(rinf.ethernet_addr),sizeof(pAdapterInfo->Address)) );
			rinf.ip4 = inet_addr((char*) pAdapterInfo->IpAddressList.IpAddress.String );
			rinf.ip4mask = inet_addr((char*) pAdapterInfo->IpAddressList.IpMask.String );
			res.push_back(rinf);
		};
	};
	return res;
};

#else 


d_net_interface_list get_netiflist(){
	d_net_interface_list res; tbgeneral::darray<byte> buff; // DWORD cnt; 
    struct ifreq ifs[64];
    struct ifconf ifc;
    struct ifreq *ifr, *ifend;
    struct ifreq ifreq;
    int SockFD;

    SockFD = socket(AF_INET, SOCK_DGRAM, 0);

	ifc.ifc_len = sizeof(ifs);   ifc.ifc_req = ifs;
    if (ioctl(SockFD, SIOCGIFCONF, &ifc) < 0)
    {   fprintf(stderr,"ioctl(SIOCGIFCONF): %m\n");return res;  }

    ifend = ifs + (ifc.ifc_len / sizeof(struct ifreq));
    for (ifr = ifc.ifc_req; ifr < ifend; ifr++) if (ifr->ifr_addr.sa_family == AF_INET)
    {  
        r_net_interface_info rinf;
		rinf.initmask = 1;
		rinf.inum = ifr - ifc.ifc_req;
		rinf.name = ifr->ifr_name;
		rinf.ethernet_addr=0;
		strncpy(ifreq.ifr_name, ifr->ifr_name,sizeof(ifreq.ifr_name));
        if (ioctl (SockFD, SIOCGIFHWADDR, &ifreq) >= 0)
            { memcpy( &rinf.ethernet_addr , ifreq.ifr_hwaddr.sa_data , sizeof(struct ether_addr) );
			  rinf.initmask	|=2;
            };
        if (ioctl (SockFD, SIOCGIFADDR , &ifreq) >= 0)
			{ rinf.initmask	|=4; rinf.ip4 = ((sockaddr_in*) &ifreq.ifr_addr)->sin_addr.s_addr;};
        if (ioctl (SockFD, SIOCGIFNETMASK , &ifreq) >= 0)
			{ rinf.initmask	|=8; rinf.ip4mask = ((sockaddr_in*) &ifreq.ifr_addr)->sin_addr.s_addr;};

		//print_r_net_interface_info( rinf );
		res.push_back( rinf );
	};
	return res;
};

#endif

}; // namespace crsys
