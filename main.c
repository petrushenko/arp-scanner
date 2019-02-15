#include <stdio.h>
#include <Windows.h>
#include <Winsock.h>

#pragma comment(lib, "Ws2_32.lib")

#include <Iphlpapi.h>

#pragma comment(lib, "IPHLPAPI.lib")

int printMac(ULONG*);
int incIP(IPAddr*);

int main(void)
{
	struct in_addr   DestIpStruct;

	IPAddr	         DestIp = 0; //Ip для прослушивания
	IPAddr		     SrcIp = 0;  //С какого устройства отправлять arp запрос
	IPAddr			 NetwIp = 0;
	IPAddr			 MaskIp = 0; 

	char			 ip_str[15], maskip_str[15];

	ULONG            MacAddr[2]; // MAC addr = 6 bytes
	ULONG            PhysAddrLen;
	ULONG			 hosts_count = 0;

	IP_ADAPTER_INFO  *pAdapterInfo; 
	ULONG            ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	ULONG            dwRetVal = 0; // Errors

	pAdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));

	if (pAdapterInfo == NULL) {
		return -1;
	}

	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		//Для заполнения информации не хватило размера, выделенного изначально
		//Функция вернула достаточное значение -> перевыделяем память
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);
	}

	if (dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == NO_ERROR) {
		IP_ADAPTER_INFO *tmpPtrAdInf = pAdapterInfo;
		while (tmpPtrAdInf) {
			memset(ip_str, 0, 15);
			memset(maskip_str, 0, 15);

			strcpy_s(ip_str, 15, tmpPtrAdInf->IpAddressList.IpAddress.String);
			strcpy_s(maskip_str, 15, tmpPtrAdInf->IpAddressList.IpMask.String);

			NetwIp = inet_addr(ip_str);
			MaskIp = inet_addr(maskip_str);

			DestIp = NetwIp & MaskIp; 
			hosts_count = ntohl(~MaskIp); // возможное количество хостов исходя из маски подсети

			for (int i = 0; i <= hosts_count - 2; i++) {
				incIP(&DestIp);
				 //SrcIp = 0

				DestIpStruct.S_un.S_addr = DestIp;
				printf_s("IP: %s; MAC: ", inet_ntoa(DestIpStruct));

				PhysAddrLen = 6; //После неудачного ARP запроса PhysAddrLen -> обновляю до 6

				if (dwRetVal = SendARP(DestIp, SrcIp, MacAddr, &PhysAddrLen) == NO_ERROR) {
					getMacString(MacAddr);
					printf_s("\n");
				}
				else {
					printf_s("%d\n", dwRetVal);
				} 
			}
			tmpPtrAdInf = tmpPtrAdInf->Next;
		}
	}
	else {
		free(pAdapterInfo);
		return dwRetVal;
	}
	free(pAdapterInfo);

	getchar();
	return 0;
}

int getMacString(ULONG *mac)
{
	UCHAR *mac_byte = mac;
	for (int i = 0; i < 6; i++) {
		printf_s("%02X-", mac_byte[i]);
	}
	return 0;
}

int incIP(IPAddr *ip)
{
	IPAddr tmp_ip = ntohl(*ip); //Перевод из интернет представления в представление на компьютере

	tmp_ip += 1;

	*ip = htonl(tmp_ip);

	return 0;
}