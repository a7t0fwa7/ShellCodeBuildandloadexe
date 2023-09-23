#include "TOOL.h"

 
#include <iostream>
#include<windows.h>


//�ַ�����ϣ�㷨
DWORD Hash_GetDigest(char* strFunName)
{
	//�ַ������ȴ���3��֤Hashֵ�в�����0
	DWORD dwDigest = 0;
	while (*strFunName)
	{
		dwDigest = (dwDigest << 25 | dwDigest >> 7);
		dwDigest += *strFunName;
		strFunName++;
	}
	return dwDigest;
}

bool Hash_CmpString(char* strFunName, int nHash)
{
	unsigned int nDigest = 0;
	while (*strFunName)
	{
		nDigest = ((nDigest << 25) | (nDigest >> 7));
		nDigest = nDigest + *strFunName;
		strFunName++;
	}
	return nHash == nDigest ? true : false;
}



bool _Is_64Peformat(char* lpPeBufer)
{
	PIMAGE_FILE_HEADER _File = (PIMAGE_FILE_HEADER) & ((PIMAGE_NT_HEADERS)((((PIMAGE_DOS_HEADER)lpPeBufer)->e_lfanew) + lpPeBufer))->FileHeader;
	if (_File->Machine == IMAGE_FILE_MACHINE_AMD64 ||
		_File->Machine == IMAGE_FILE_MACHINE_IA64) // may be IMAGE_FILE_MACHINE_I386
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


DWORD64 GetFunAddrByHash(int nHashDigest, HMODULE hModule)
{

	// 1. ��ȡDOSͷ��NTͷ
	PIMAGE_DOS_HEADER pDos_Header;
	// 2. ��ȡ��������
	PIMAGE_DATA_DIRECTORY   pDataDir;
	PIMAGE_EXPORT_DIRECTORY pExport;

	pDos_Header = (PIMAGE_DOS_HEADER)hModule;
	if (_Is_64Peformat((char*)(hModule)) == TRUE)
	{
		PIMAGE_NT_HEADERS64 pNt_Header;
		pNt_Header = (PIMAGE_NT_HEADERS64)((DWORD64)hModule + pDos_Header->e_lfanew);
		pDataDir = pNt_Header->OptionalHeader.DataDirectory + IMAGE_DIRECTORY_ENTRY_EXPORT;

	}
	else
	{
		PIMAGE_NT_HEADERS pNt_Header;
		pNt_Header = (PIMAGE_NT_HEADERS)((DWORD64)hModule + pDos_Header->e_lfanew);
		pDataDir = pNt_Header->OptionalHeader.DataDirectory + IMAGE_DIRECTORY_ENTRY_EXPORT;
	}




	pExport = (PIMAGE_EXPORT_DIRECTORY)((DWORD64)hModule + pDataDir->VirtualAddress);



	// 3. ��ȡ��������ϸ��Ϣ
	PDWORD pAddrOfFun = (PDWORD)(pExport->AddressOfFunctions + (DWORD64)hModule);
	PDWORD pAddrOfNames = (PDWORD)(pExport->AddressOfNames + (DWORD64)hModule);
	PWORD  pAddrOfOrdinals = (PWORD)(pExport->AddressOfNameOrdinals + (DWORD64)hModule);

	// 4. �����Ժ��������Һ�����ַ������ѭ����ȡENT�еĺ���������Ϊ���Ժ�����
	//    Ϊ��׼����˲������޺�����������������봫��ֵ�Աȣ�����ƥ��������EAT
	//    ����ָ�������Ϊ��������ȡ�����ֵַ��
	DWORD64 dwFunAddr;
	for (DWORD i = 0; i < pExport->NumberOfNames; i++)
	{
		PCHAR lpFunName = (PCHAR)(pAddrOfNames[i] + (DWORD64)hModule);
		if (Hash_CmpString(lpFunName, nHashDigest))
		{
			dwFunAddr = pAddrOfFun[pAddrOfOrdinals[i]] + (DWORD64)hModule;
			break;
		}
		if (i == pExport->NumberOfNames - 1)
			return 0;
	}

	return dwFunAddr;
}


char* RetX32RunExeAdr(char* pExeAddr, HMODULE Kernel32)
{
	// 1. �ֲ��ַ���
	CHAR szUser32[] = { 'u','s','e','r','3','2','.','d','l','l','\0' };
	CHAR szKernel32[] = { 'k','e','r','n','e','l','3','2','.','d','l','l','\0' };
	CHAR szntdll[] = { 'n','t','d','l','l','.','d','l','l','\0' };
	CHAR szCMD[] = { 'c','m','d','.','e','x','e','\0' };
	CHAR csVirtualAlloc[] = { 'V','i','r','t','u','a','l','A','l','l','o','c' ,'\0' };



	CHAR csVirtualProtect[] = { 'V','i','r','t','u','a','l','P','r','o','t','e','c','t','\0' };
	CHAR csGetProcAddress[] = { 'G','e','t','P','r','o','c','A','d','d','r','e','s','s','\0' };
	CHAR csCreateThread[] = { 'C','r','e','a','t','e','T','h','r','e','a','d' ,'\0' };
	CHAR csWaitForSingleObject[] = { 'W','a','i','t','F','o','r','S','i','n','g','l','e','O','b','j','e','c','t','\0' };


	// 2. ��ȡ�ؼ�ģ���ַ
	HMODULE hKernel32By64 = 0;
	HMODULE hNtDll = 0;
	DWORD ImageBase = 0;
	HMODULE hUser32 = 0;



	// 3. ��ȡ�ؼ�ģ���ַ
	DefineFuncPtr(LoadLibraryExA, Kernel32);
	hKernel32By64 = My_LoadLibraryExA(szKernel32, 0, 0);
	hNtDll = My_LoadLibraryExA(szntdll, 0, 0);
	hUser32 = My_LoadLibraryExA(szUser32, 0, 0);

	
	DefineFuncPtr(VirtualAlloc, hKernel32By64);
	DefineFuncPtr(VirtualProtect, hKernel32By64);
	DefineFuncPtr(CreateThread, hKernel32By64);
	DefineFuncPtr(WaitForSingleObject, hKernel32By64);
	DefineFuncPtr(GetProcAddress, hKernel32By64);


	//User32
	DefineFuncPtr(MessageBoxA, hUser32);

	//ntdll
	DefineFuncPtr(memcpy, hNtDll);
	DefineFuncPtr(sprintf, hNtDll);



	HANDLE hFile = NULL;
	HANDLE hFileMaping = NULL;
	//��ȡһЩ��������
	//PE��ʼ��
	// 	   PIMAGE_NT_HEADERS
	PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)pExeAddr;
	PIMAGE_NT_HEADERS pNtHdr = NULL;
	PIMAGE_SECTION_HEADER pSectionHdr = NULL;
	int SectionCount = 0;
	int SectionAlignment = 0;
	int FileAlignment = 0;


	//--------------------------------------------------------------------------------------------------------------------------------------�������ڴ�
	pNtHdr = (PIMAGE_NT_HEADERS)((DWORD)pExeAddr + ((PIMAGE_DOS_HEADER)pExeAddr)->e_lfanew);
	pSectionHdr = (PIMAGE_SECTION_HEADER)((DWORD)&pNtHdr->OptionalHeader + pNtHdr->FileHeader.SizeOfOptionalHeader);	

	SectionCount = pNtHdr->FileHeader.NumberOfSections;				//������
	SectionAlignment = pNtHdr->OptionalHeader.SectionAlignment;		//�ڴ���䷽ʽ
	FileAlignment = pNtHdr->OptionalHeader.FileAlignment;			//�ļ����䷽ʽ


	//����һ������ �ڴ���С�Ŀռ�
	PVOID pNewPe = My_VirtualAlloc(NULL, pNtHdr->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);		
	if (pNewPe == NULL)
	{
		return 0;
	}

	//����PE ��չ��
	My_memcpy((void*)pNewPe, (void*)pDosHdr, pNtHdr->OptionalHeader.SizeOfHeaders);
	for (int i = 0; i < SectionCount; i++)
	{
		My_memcpy(
			(void*)((DWORD)pNewPe + pSectionHdr[i].VirtualAddress),			//VirtualAddress Rva ֱ�� ��PE Va����
			(void*)((DWORD)pExeAddr + pSectionHdr[i].PointerToRawData),		//ֱ�Ӵ��ļ����ζ�ȡ����
			pSectionHdr[i].SizeOfRawData);									//�������δ�С
	}



	//-------------------------------------------------------------------------------------------------------------------------------------�޸������

	PIMAGE_IMPORT_DESCRIPTOR pImportTable = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)pNewPe + pNtHdr->OptionalHeader.DataDirectory[1].VirtualAddress);

	do
	{
		const char* pDllPath = (const char*)(pImportTable->Name + (DWORD)pNewPe);
		PDWORD pIat = (PDWORD)(pImportTable->FirstThunk + (DWORD)pNewPe);
		if (*pIat == NULL)
		{
			pImportTable++;
			continue;
		}
		HMODULE hModule = My_LoadLibraryExA(pDllPath, 0, 0);
		if (hModule == NULL)
		{
			return 0;
		}
		else
		{
			do
			{
				if (((*pIat) >> 31) != 1)
				{
					PIMAGE_IMPORT_BY_NAME Name = (PIMAGE_IMPORT_BY_NAME)((DWORD)pNewPe + *pIat);
					*pIat = (DWORD)My_GetProcAddress(hModule, Name->Name);
				}
				else
				{
					*pIat = (DWORD)My_GetProcAddress(hModule, (const char*)((*pIat) & 0x7fffffff));
				}
				pIat++;
			} while (*pIat);
		}
		pImportTable++;
	} while (pImportTable->Name != NULL);



	//--------------------------------------------------------------------------------------------------------------------------------------�޸��ض�λ��
	DWORD Offset = NULL;
	int BaseTableSize = NULL;
	int NowSize = NULL;
	PIMAGE_BASE_RELOCATION pBaseTable = NULL;
	Offset = (DWORD)pNewPe - pNtHdr->OptionalHeader.ImageBase;//������Ҫ�ض�λ��ֵ
	if (pNtHdr->OptionalHeader.DataDirectory[5].VirtualAddress == 0)
	{
		return 0;
	}
	pBaseTable = (PIMAGE_BASE_RELOCATION)((DWORD)pNewPe + pNtHdr->OptionalHeader.DataDirectory[5].VirtualAddress);
	BaseTableSize = pNtHdr->OptionalHeader.DataDirectory[5].Size;

	while ((NowSize < BaseTableSize) && (pBaseTable->VirtualAddress != 0))
	{
		PWORD pBase = (PWORD)((DWORD)pBaseTable + sizeof(IMAGE_BASE_RELOCATION));
		PDWORD pAageBase = (PDWORD)((DWORD)pNewPe + pBaseTable->VirtualAddress);
		int Count = (pBaseTable->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
		for (int i = 0; i < Count; i++)
		{
			if (pBase[i] >= 0x3000)//1�޸ĸ�16λ 2�޸ĵ�16λ
			{
				PDWORD UpdataAddr = (PDWORD)((pBase[i] & 0xfff) + (DWORD)pAageBase);
				*UpdataAddr = Offset + *UpdataAddr;
			}
		}
		NowSize += pBaseTable->SizeOfBlock;
		pBaseTable = (PIMAGE_BASE_RELOCATION)((DWORD)pBaseTable + pBaseTable->SizeOfBlock);
	}

	char* mapadd = ((char*)pNewPe + pNtHdr->OptionalHeader.AddressOfEntryPoint);

	return mapadd;
}



char* RetX64RunExeAdr(char* pExeAddr, HMODULE Kernel32)
{
	// 1. �ֲ��ַ���
	CHAR szUser32[] = { 'u','s','e','r','3','2','.','d','l','l','\0' };
	CHAR szKernel32[] = { 'k','e','r','n','e','l','3','2','.','d','l','l','\0' };
	CHAR szntdll[] = { 'n','t','d','l','l','.','d','l','l','\0' };
	CHAR szCMD[] = { 'c','m','d','.','e','x','e','\0' };
	CHAR csVirtualAlloc[] = { 'V','i','r','t','u','a','l','A','l','l','o','c' ,'\0' };



	CHAR csVirtualProtect[] = { 'V','i','r','t','u','a','l','P','r','o','t','e','c','t','\0' };
	CHAR csGetProcAddress[] = { 'G','e','t','P','r','o','c','A','d','d','r','e','s','s','\0' };
	CHAR csCreateThread[] = { 'C','r','e','a','t','e','T','h','r','e','a','d' ,'\0' };
	CHAR csWaitForSingleObject[] = { 'W','a','i','t','F','o','r','S','i','n','g','l','e','O','b','j','e','c','t','\0' };


	// 2. ��ȡ�ؼ�ģ���ַ
	HMODULE hKernel32By64 = 0;
	HMODULE hNtDll = 0;
	DWORD ImageBase = 0;
	HMODULE hUser32 = 0;

	// 3. ��ȡ�ؼ�ģ���ַ
	DefineFuncPtr(LoadLibraryExA, (HMODULE)Kernel32);
	hKernel32By64 = My_LoadLibraryExA(szKernel32, 0, 0);
	hNtDll = My_LoadLibraryExA(szntdll, 0, 0);
	hUser32 = My_LoadLibraryExA(szUser32, 0, 0);
	DefineFuncPtr(VirtualAlloc, hKernel32By64);
	DefineFuncPtr(VirtualProtect, hKernel32By64);
	DefineFuncPtr(CreateThread, hKernel32By64);
	DefineFuncPtr(WaitForSingleObject, hKernel32By64);
	DefineFuncPtr(GetProcAddress, hKernel32By64);


	//User32
	DefineFuncPtr(MessageBoxA, hUser32);

	//ntdll
	DefineFuncPtr(memcpy, hNtDll);
	DefineFuncPtr(sprintf, hNtDll);


	DWORD64 SectionCount = 0;
	DWORD64 SectionAlignment = 0;
	DWORD64 FileAlignment = 0;

	PIMAGE_NT_HEADERS64 pNtHdr = NULL;
	//�������ڴ�
	pNtHdr = (PIMAGE_NT_HEADERS64)((char*)pExeAddr + ((PIMAGE_DOS_HEADER)pExeAddr)->e_lfanew);
	SectionCount = pNtHdr->FileHeader.NumberOfSections;
	SectionAlignment = pNtHdr->OptionalHeader.SectionAlignment;
	FileAlignment = pNtHdr->OptionalHeader.FileAlignment;


	LPVOID pNewPe = My_VirtualAlloc(NULL, pNtHdr->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	if (pNewPe == NULL)
	{
		return 0;
	}

	//����PE
	My_memcpy((void*)pNewPe, (void*)pExeAddr, pNtHdr->OptionalHeader.SizeOfHeaders);


	PIMAGE_SECTION_HEADER pSectionHdr = (PIMAGE_SECTION_HEADER)IMAGE_FIRST_SECTION(pNtHdr);
	for (int i = 0; i < SectionCount; i++)
	{
		My_memcpy((char*)((DWORD64)pNewPe + pSectionHdr[i].VirtualAddress),
			(char*)((DWORD64)pExeAddr + pSectionHdr[i].PointerToRawData),
			pSectionHdr[i].SizeOfRawData);

	}




	//�޸������
	PIMAGE_DOS_HEADER pDos64 = (PIMAGE_DOS_HEADER)pNewPe;

	PIMAGE_NT_HEADERS64 pNt64 = (PIMAGE_NT_HEADERS64)(pDos64->e_lfanew + (char*)pNewPe);

	PIMAGE_IMPORT_DESCRIPTOR ImportTable = (PIMAGE_IMPORT_DESCRIPTOR)((char*)pNewPe + pNt64->OptionalHeader.DataDirectory[1].VirtualAddress);
	//printf("Nt%x---ImportTable=%x---%x\n", pNt64, ImportTable, pNt64->OptionalHeader.DataDirectory[1].VirtualAddress);

	//��������� ��ȫ0�ṹ���β
	while (ImportTable->Name != 0)
	{
		//�����DLL����ͨ��LoadLibrary
		CHAR* DllName = (CHAR*)(ImportTable->Name + ((char*)pNewPe));
		//����DLL
		HMODULE hModule = My_LoadLibraryExA(DllName, 0, 0);

		//��ȡIAT��
		PIMAGE_THUNK_DATA64 Iat = (PIMAGE_THUNK_DATA64)(ImportTable->FirstThunk + ((char*)pNewPe));

		//����Ƿ���Ч
		while (Iat->u1.AddressOfData)
		{
			ULONGLONG  FunctionAddr = 0;
			
			
			//���λΪ0
			if (!IMAGE_SNAP_BY_ORDINAL(Iat->u1.AddressOfData))
			{
				PIMAGE_IMPORT_BY_NAME Name = (PIMAGE_IMPORT_BY_NAME)(Iat->u1.Function + ((char*)pNewPe));
				FunctionAddr = (ULONGLONG)My_GetProcAddress(hModule, Name->Name);
			}
			else
			{
				//���û�����֣���16λ�������
				FunctionAddr = (ULONGLONG)My_GetProcAddress(hModule, (LPCSTR)(Iat->u1.Ordinal & 0xffff));

			}
			Iat->u1.Function = (ULONGLONG)FunctionAddr;
			++Iat;
		}
		ImportTable++;
	}



	//-------------------------------------------------�޸��ض�λ��
	DWORD64 Offset = NULL;
	int BaseTableSize = NULL;
	int NowSize = NULL;
	PIMAGE_BASE_RELOCATION pBaseTable = NULL;
	Offset = (DWORD64)pNewPe - pNt64->OptionalHeader.ImageBase;//������Ҫ�ض�λ��ֵ
	if (pNt64->OptionalHeader.DataDirectory[5].VirtualAddress == 0)
	{
		return 0 ;
	}
	pBaseTable = (PIMAGE_BASE_RELOCATION)((DWORD64)(pNewPe)+pNt64->OptionalHeader.DataDirectory[5].VirtualAddress);
	BaseTableSize = pNt64->OptionalHeader.DataDirectory[5].Size;

	while ((NowSize < BaseTableSize) && (pBaseTable->VirtualAddress != 0))
	{
		PWORD pBase = (PWORD)((DWORD64)pBaseTable + sizeof(IMAGE_BASE_RELOCATION));
		PDWORD64 pAageBase = (PDWORD64)((DWORD64)(pNewPe)+pBaseTable->VirtualAddress);
		int Count = (pBaseTable->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
		for (int i = 0; i < Count; i++)
		{
			if (pBase[i] >= 0x3000)//1�޸ĸ�16λ 2�޸ĵ�16λ
			{
				PDWORD64 UpdataAddr = (PDWORD64)((pBase[i] & 0xfff) + (DWORD64)pAageBase);
				*UpdataAddr = Offset + *UpdataAddr;
			}
		}
		NowSize += pBaseTable->SizeOfBlock;
		pBaseTable = (PIMAGE_BASE_RELOCATION)((DWORD64)pBaseTable + pBaseTable->SizeOfBlock);
	}

	char* mapadd = ((char*)pNewPe + pNtHdr->OptionalHeader.AddressOfEntryPoint);

	return mapadd;
}