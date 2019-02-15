#include <winsock.h> //древняя библеотека
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "IPHLPAPI.lib")

#define LOG_FILE "log.txt"
#define UCHAR BYTE
#define END_PROG_STR "Scanning was succesfull! Press any key to exit..."

//Convert MAC from BYTES to string
int getMacString(BYTE *mac, char *mac_str);

//Increment IP
int incIP(IPAddr*);

int main(void)
{
	struct in_addr DestIpStruct;

	IPAddr DestIp = 0; //Ip для прослушивания
	IPAddr SrcIp = 0;  //С какого устройства отправлять arp запрос
	IPAddr NetwIp = 0;
	IPAddr MaskIp = 0; 

	char ip_str[15], maskip_str[15], mac_str[18];

	ULONG MacAddr[2]; // MAC addr = 6 bytes
	ULONG PhysAddrLen;
	ULONG hosts_count = 0;

	IP_ADAPTER_INFO *pAdapterInfo; 
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	ULONG dwRetVal = 0; // Errors

	FILE *f; //file for loggin

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

		fopen_s(&f, LOG_FILE, "w+");

		IP_ADAPTER_INFO *tmpPtrAdInf = pAdapterInfo;
		while (tmpPtrAdInf) {
			memset(ip_str, 0, 15);
			memset(maskip_str, 0, 15);
			strcpy_s(ip_str, 15, tmpPtrAdInf->IpAddressList.IpAddress.String);
			strcpy_s(maskip_str, 15, tmpPtrAdInf->IpAddressList.IpMask.String);
			getMacString(tmpPtrAdInf->Address, mac_str);
			printf_s("Adapter: %s   MAC: %s\n", tmpPtrAdInf->Description, mac_str);
			fprintf_s(f, "Adapter: %s   MAC: %s\n", tmpPtrAdInf->Description, mac_str);
			
			NetwIp = inet_addr(ip_str);
			MaskIp = inet_addr(maskip_str);
			
			DestIp = NetwIp & MaskIp; 
			if (DestIp != 0) {
				printf_s("IP: %s   Mask: %s\nInterfaces:\n", ip_str, maskip_str);
				fprintf_s(f, "IP: %s   Mask: %s\nInterfaces:\n", ip_str, maskip_str);
				hosts_count = ntohl(~MaskIp); // возможное количество хостов исходя из маски подсети
				
				for (ULONG i = 0; i <= hosts_count - 2; i++) {
					incIP(&DestIp);

					PhysAddrLen = 6;
					//SrcIp = 0 
					if (SendARP(DestIp, SrcIp, MacAddr, &PhysAddrLen) == NO_ERROR) {
						DestIpStruct.S_un.S_addr = DestIp;
						printf_s("For IP: %s", inet_ntoa(DestIpStruct));
						fprintf_s(f, "For IP: %s", inet_ntoa(DestIpStruct));
						getMacString((BYTE *)MacAddr, mac_str);
						printf_s(" MAC: %s\n", mac_str);
						fprintf_s(f, " MAC: %s\n", mac_str);
					}
				} 
			}
			else {
				printf_s("Adapter is not operational\n");
				fprintf_s(f, "Adapter is not operational\n");
			} 
			tmpPtrAdInf = tmpPtrAdInf->Next;
		}
		fclose(f);
	}
	else {
		free(pAdapterInfo);
		return dwRetVal;
	}
	free(pAdapterInfo);
	printf_s(END_PROG_STR);
	getchar();
	return 0;
} 

int getMacString(BYTE *mac, char *mac_str)
{ 
	UCHAR *mac_byte = (UCHAR *)mac;

	sprintf_s(mac_str, 18, "%02x-%02x-%02x-%02x-%02x-%02x", mac_byte[0], mac_byte[1], mac_byte[2], mac_byte[3],
		mac_byte[4], mac_byte[5]);

	return 0;
}

int incIP(IPAddr *ip)
{
	IPAddr tmp_ip = ntohl(*ip); //Перевод из интернет представления в представление на компьютере
	tmp_ip += 1;
	*ip = htonl(tmp_ip);

	return 0;
}