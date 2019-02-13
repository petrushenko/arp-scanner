#include <stdio.h>
#include <Windows.h>

#include <Iphlpapi.h>

#pragma comment(lib, "IPHLPAPI.lib")

int main(void)
{
	IP_ADAPTER_INFO  *pAdapterInfo;
	ULONG            ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	ULONG            dwRetVal = 0;

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
			printf_s("%s\n", tmpPtrAdInf->Description, 260);
			printf_s("%s\n", tmpPtrAdInf->IpAddressList.IpAddress.String);
			
			
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