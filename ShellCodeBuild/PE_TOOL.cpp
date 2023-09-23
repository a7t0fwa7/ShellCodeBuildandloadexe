#include "PE_TOOL.h"
#include <string.h>
#include <iostream>
PIMAGE_DOS_HEADER PE_TOOL::GetPeDOSHeader(char* pvoid)
{
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)pvoid;
	return pDos;
}

PIMAGE_NT_HEADERS PE_TOOL::GetPeNtHeader(char* pvoid)
{
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(GetPeDOSHeader(pvoid)->e_lfanew + pvoid);
	return pNt;
}

PIMAGE_FILE_HEADER PE_TOOL::GetPeFileHeader(char* pvoid)
{
	PIMAGE_FILE_HEADER pFileh = &(GetPeNtHeader(pvoid)->FileHeader);
	return pFileh;
}

PIMAGE_OPTIONAL_HEADER PE_TOOL::GetPeOptionHeader(char* pvoid)
{
	PIMAGE_OPTIONAL_HEADER pFile = &(GetPeNtHeader(pvoid)->OptionalHeader);
	return pFile;
}

BOOL PE_TOOL::InitDosInfo(char* pvoid)
{
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)pvoid;
	pDos->e_magic = IMAGE_DOS_SIGNATURE;
	pDos->e_cblp = 100;
	pDos->e_cp = 0;
	pDos->e_cparhdr = 0;
	pDos->e_crlc = 0;
	pDos->e_cs = 0;
	pDos->e_csum = 0;
	pDos->e_ip = 0;
	pDos->e_lfanew = 0x60;		//����OptionHeaderλ��
	return TRUE;
}

BOOL PE_TOOL::InitNtInfo(char* pvoid)
{
	PIMAGE_NT_HEADERS hFile = (PIMAGE_NT_HEADERS)pvoid;
	hFile->Signature = IMAGE_NT_SIGNATURE;			//��־



	return 0;
}

BOOL PE_TOOL::InitFileHeader(char* pvoid)
{
	PIMAGE_FILE_HEADER hFile = (PIMAGE_FILE_HEADER)pvoid;
	hFile->TimeDateStamp = 0;
	hFile->Characteristics;					//
	hFile->Machine = 0x14c;					//����ƽ̨
	hFile->NumberOfSections = 8;			//��������
	hFile->SizeOfOptionalHeader = sizeof(IMAGE_OPTIONAL_HEADER);
	hFile->NumberOfSymbols = 0;				//��������
	hFile->PointerToSymbolTable = 0;		//���ŵ�ַ


	return 0;
}

BOOL PE_TOOL::InitOptionHeader(char* pvoid)
{
	PIMAGE_OPTIONAL_HEADER hFile = (PIMAGE_OPTIONAL_HEADER)pvoid;
	hFile->Magic = 0x10B;
	hFile->AddressOfEntryPoint;
	hFile->BaseOfCode;
	//hFile->BaseOfData;
	hFile->CheckSum;
	hFile->DataDirectory;
	hFile->DllCharacteristics;
	hFile->FileAlignment;
	hFile->ImageBase;
	hFile->LoaderFlags;
	hFile->NumberOfRvaAndSizes;
	hFile->SectionAlignment;
	hFile->SizeOfCode;
	hFile->SizeOfHeaders;
	hFile->SizeOfHeapCommit;
	hFile->SizeOfHeapReserve;
	hFile->SizeOfImage;
	hFile->SizeOfInitializedData;
	hFile->SizeOfUninitializedData;
	hFile->Subsystem;
	hFile->Win32VersionValue;

	hFile->MajorImageVersion;
	hFile->MajorLinkerVersion;
	hFile->MajorOperatingSystemVersion;
	hFile->MajorSubsystemVersion;

	hFile->MinorImageVersion;
	hFile->MinorLinkerVersion;
	hFile->MinorOperatingSystemVersion;
	hFile->MinorSubsystemVersion;

	return 0;
}

//�������
LPVOID PE_TOOL::AddSection(char** pvoid, DWORD SectionSize, char* SectionName)
{
	PIMAGE_FILE_HEADER pFileHeader = (GetPeFileHeader(*pvoid));
	PIMAGE_SECTION_HEADER FirstSection = IMAGE_FIRST_SECTION(GetPeNtHeader(*pvoid));
	DWORD OldNumberOfSections = pFileHeader->NumberOfSections;
	char* _SectionName = new char[8]{ 0 };
	strcpy_s(_SectionName, 0x8, SectionName);

	//����һ������
	pFileHeader->NumberOfSections += 1;
	//���������ε�������
	memcpy(&(FirstSection[OldNumberOfSections]), &(FirstSection[OldNumberOfSections - 1]), sizeof(IMAGE_SECTION_HEADER));
	//���һ��������Ϊ ������
	memset(&(FirstSection[pFileHeader->NumberOfSections]), 0, sizeof(IMAGE_SECTION_HEADER));
	//��������
	memcpy(FirstSection[OldNumberOfSections].Name, _SectionName, 8);

	FirstSection[OldNumberOfSections].PointerToRawData = FirstSection[OldNumberOfSections - 1].PointerToRawData + SnapToMemAddr(FirstSection[OldNumberOfSections - 1].SizeOfRawData, GetPeOptionHeader(*pvoid)->FileAlignment);	//ʵ�ʴ�С

	//����������
	FirstSection[OldNumberOfSections].VirtualAddress = FirstSection[OldNumberOfSections - 1].VirtualAddress + SnapToMemAddr(FirstSection[OldNumberOfSections - 1].Misc.VirtualSize, GetPeOptionHeader(*pvoid)->SectionAlignment);	//װ�ص��ڴ��RVA

	FirstSection[OldNumberOfSections].SizeOfRawData = FirstSection[OldNumberOfSections].Misc.VirtualSize;

	//����������Ϣ�������������� 0xE00000E0(����Ȩ��)
	FirstSection[OldNumberOfSections].Characteristics = 0xE00000E0;

	GetPeOptionHeader(*pvoid)->SizeOfImage = FirstSection[OldNumberOfSections].VirtualAddress + FirstSection[OldNumberOfSections].Misc.VirtualSize;

	//����µ�����
	DWORD NewFileSize = FirstSection[OldNumberOfSections].VirtualAddress + FirstSection[OldNumberOfSections].Misc.VirtualSize;

	//char ShellCode[] = { 0xE9,0x28,0x8F,0xF0,0xFF};
	*pvoid = (char*)realloc((LPVOID)*pvoid, NewFileSize);
	//SetSectionData(pvoid, SectionName, ShellCode, _countof(ShellCode));

	return NULL;
}

DWORD PE_TOOL::SnapToMemAddr(DWORD MemSize, DWORD Alignment_granularity)
{

	//�����������������ô�Ͳ���Ҫ���뼴�ɣ������������+1��*1����
	return MemSize % Alignment_granularity == 0 ? MemSize : Alignment_granularity * (MemSize / Alignment_granularity + 1);
}

DWORD RvaToFoa(char* lpImage, DWORD dwRva)
{
	//1 ��ȡ���α����ʼλ��
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)lpImage;
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(pDos->e_lfanew + lpImage);
	PIMAGE_SECTION_HEADER pHeader = IMAGE_FIRST_SECTION(pNt);
	if (dwRva < pNt->OptionalHeader.SizeOfHeapReserve)
	{
		return dwRva;

	}
	//ѭ���ж�RVA �䵽���Ǹ�������
	for (int i = 0; i < pNt->FileHeader.NumberOfSections; i++)
	{
		DWORD dwSectionRva = pHeader[i].VirtualAddress;
		DWORD dwSectionEndRva = dwSectionRva + pHeader[i].SizeOfRawData;
		DWORD dwSectionFOA = pHeader[i].PointerToRawData;
		if (dwRva >= dwSectionRva && dwRva <= dwSectionEndRva)
		{
			pHeader[i].VirtualAddress;
			DWORD dwFOA = dwRva - dwSectionRva + dwSectionFOA;
			return dwFOA;

		}

	}
	return -1;

}

DWORD PE_TOOL::SetOpeAtTheSection(char** pvoid, char* SectionName)
{
	DWORD SectionNum = 0;

	SectionNum = GetPeFileHeader(*pvoid)->NumberOfSections;


	PIMAGE_SECTION_HEADER pFirstSection = IMAGE_FIRST_SECTION(GetPeNtHeader(*pvoid));
	for (int i = 0; i < SectionNum; ++i)
	{
		if (!strcmp((char*)pFirstSection[i].Name, SectionName))
		{
			GetPeOptionHeader(*pvoid)->AddressOfEntryPoint = pFirstSection[i].VirtualAddress;
			return TRUE;
		}


	}
	return FALSE;

};

DWORD PE_TOOL::GetPeMemSize(char* pvoid)
{
	DWORD SectionNum = 0;
	SectionNum = GetPeFileHeader(pvoid)->NumberOfSections;
	PIMAGE_SECTION_HEADER LastSection = &(IMAGE_FIRST_SECTION(GetPeNtHeader(pvoid))[SectionNum - 1]);

	return (LastSection->VirtualAddress + LastSection->Misc.VirtualSize);
}


DWORD PE_TOOL::SetSectionData(char** pvoid, char* SectionName, char* DataBuffer, DWORD DataSize)
{
	DWORD SectionNum = 0;
	SectionNum = GetPeFileHeader(*pvoid)->NumberOfSections;

	PIMAGE_SECTION_HEADER pFirstSection = IMAGE_FIRST_SECTION(GetPeNtHeader(*pvoid));
	for (int i = 0; i < SectionNum; ++i)
	{
		if (!strcmp((char*)pFirstSection[i].Name, SectionName))
		{
			if (pFirstSection[i].SizeOfRawData < DataSize)
			{
				return FALSE;
			}
			else {
				memcpy((*pvoid + pFirstSection[i].PointerToRawData), DataBuffer, DataSize);
				return TRUE;
			}
		}
	}
	return FALSE;
}



PIMAGE_SECTION_HEADER PE_TOOL::GetSeatchSectionByName(char* pvoid, const char* SectionName)
{
	DWORD SectionNum = 0;
	SectionNum = GetPeFileHeader(pvoid)->NumberOfSections;

	PIMAGE_SECTION_HEADER pFirstSection = IMAGE_FIRST_SECTION(GetPeNtHeader(pvoid));
	for (int i = 0; i < SectionNum; ++i)
	{
		if (!strcmp((char*)pFirstSection[i].Name, SectionName))
		{
			return (PIMAGE_SECTION_HEADER)(&pFirstSection[i]);
		}
	}
	return NULL;


}