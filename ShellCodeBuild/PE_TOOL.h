#include <Windows.h>
class PE_TOOL
{
public:
	static PIMAGE_DOS_HEADER GetPeDOSHeader(char* pvoid);

	static PIMAGE_NT_HEADERS GetPeNtHeader(char* pvoid);

	static PIMAGE_FILE_HEADER GetPeFileHeader(char* pvoid);

	static PIMAGE_OPTIONAL_HEADER GetPeOptionHeader(char* pvoid);

	static BOOL InitDosInfo(char* pvoid);		//��ʼ��Dosͷ��Ϣ
	static BOOL InitNtInfo(char* pvoid);		//��ʼ��NTͷ��Ϣ
	static BOOL InitFileHeader(char* pvoid);	//��ʼ���ļ�ͷ��Ϣ
	static BOOL InitOptionHeader(char* pvoid);	//��ʼ����չͷ��Ϣ


	static LPVOID AddSection(char** pvoid, DWORD SectionSize, char* SectionName);						//������� ���ص�ַ
	static DWORD SetSectionData(char** pvoid, char* SectionName, char* DataBuffer, DWORD DataSize);		//��ĳ���θ�������
	static DWORD SetOpeAtTheSection(char** pvoid, char* SectionName);
	static DWORD GetPeMemSize(char* pvoid);
	static DWORD SnapToMemAddr(DWORD MemSize, DWORD Alignment_granularity);	//��Ҫ����ĵ�ַ������Ĵ�С
	static PIMAGE_SECTION_HEADER GetSeatchSectionByName(char* pvoid, const char* SectionName);    //PIMAGE_SECTION_HEADER ����

	static DWORD RvaToFoa(char* lpImage, DWORD dwRva);





};

