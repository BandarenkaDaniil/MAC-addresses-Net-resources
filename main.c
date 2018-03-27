#include <winsock2.h>
#include <iphlpapi.h>
#include <windows.h>
#include <winnetwk.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "mpr.lib")

bool printMACAddresses()
{
	PIP_ADAPTER_INFO pAdapterInfo = NULL;

	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
	{
		free(pAdapterInfo);

		pAdapterInfo = (PIP_ADAPTER_INFO)malloc(ulOutBufLen);
		if (pAdapterInfo == NULL) 
		{
			printf("Error allocating memory needed to call GetAdaptersinfo\n");
			return 1;
		}
	}

	DWORD dwStatus;
	if ((dwStatus = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
	{
		PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
		while (pAdapter) 
		{
			printf("Adapter Name:  \t%s\n", pAdapter->Description);
			printf("Adapter MAC:    ");

			for (UINT i = 0; i < pAdapter->AddressLength; i++) 
			{
				if (i == (pAdapter->AddressLength - 1))
					printf("%.2X\n", (int)pAdapter->Address[i]);
				else
					printf("%.2X-", (int)pAdapter->Address[i]);
			}

			pAdapter = pAdapter->Next;
			printf("\n");
		}
	}
	else 
	{
		printf("GetAdaptersInfo failed with error: %d\n", dwStatus);
	}

	if (pAdapterInfo)
	{
		free(pAdapterInfo);
	}
}

bool enumerateNetResources(LPNETRESOURCE lpnr)
{
	HANDLE hEnum;
	LPNETRESOURCE lpCurrNetResource;
	DWORD dwBuffer = 16384;
	DWORD dwEntries = -1;

	DWORD dwStatus = WNetOpenEnum(RESOURCE_GLOBALNET, RESOURCETYPE_ANY, 0, lpnr, &hEnum);

	if (dwStatus != NO_ERROR)
	{
		printf("WnetOpenEnum failed with error %d\n", dwStatus);
		return false;
	}

	lpCurrNetResource = (LPNETRESOURCE)GlobalAlloc(GPTR, dwBuffer);
	if (lpCurrNetResource == NULL) 
	{
		printf("WnetOpenEnum failed with error %d\n", dwStatus);

		return false;
	}

	DWORD dwResultEnum;
	do
	{
		dwResultEnum = WNetEnumResource(hEnum, &dwEntries, lpCurrNetResource, &dwBuffer);

		if (dwResultEnum == NO_ERROR)
		{
			for (int i = 0; i < dwEntries; i++)
			{
				printf("%S\n", lpCurrNetResource[i].lpProvider);

				if (RESOURCEUSAGE_CONTAINER == (lpCurrNetResource[i].dwUsage
					& RESOURCEUSAGE_CONTAINER))
				{
					if (!enumerateNetResources(&lpCurrNetResource[i]))
					{
						printf("EnumerateFunc returned FALSE\n");
					}
				}
			}
		}
		else if (dwResultEnum != ERROR_NO_MORE_ITEMS)
		{
			printf("WNetEnumResource failed with error %d\n", dwResultEnum);
			break;
		}
	} while (dwResultEnum != ERROR_NO_MORE_ITEMS);

	GlobalFree((HGLOBAL)lpCurrNetResource);

	dwStatus = WNetCloseEnum(hEnum);
	if (dwStatus != NO_ERROR)
	{
		printf("WNetCloseEnum failed with error %d\n", dwStatus);

		return false;
	}

	return true;
}

int __cdecl main()
{	
	printMACAddresses();

	LPNETRESOURCE lpnr = NULL;

	printf("Netresources: \n");
	if (enumerateNetResources(lpnr) == FALSE) {
		printf("Call to EnumerateFunc failed\n");
	}

	return 0;
}
