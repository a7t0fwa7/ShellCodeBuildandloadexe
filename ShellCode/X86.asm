
.386
.model flat, c
public GetPc32
;.data
 

.code                          ;��Ϊ������

							
GetPc32 PROC						;GetPc ��ȡ�ĵ�ַ����ʹ��Ӳ���� ����ֻ�� EAX ��ȥӲ�����С
	mov eax,[esp]
	and eax,0FFFFF000h
	ret
GetPc32 endp


GetImageBase32 PROC
	assume fs:nothing
	mov eax, fs:[30h]
	mov eax,[eax+08h]
	ret
GetImageBase32 endp

GetLdrModuleBase32 PROC					;LDR����
	assume fs:nothing
	push esi
	mov esi, dword ptr fs : [30h]		; esi = PEB�ĵ�ַ
	mov esi, [esi + 0Ch]				 ; esi = ָ��PEB_LDR_DATA�ṹ��ָ��
	pop esi
	ret
GetLdrModuleBase32 endp

;��ȡKernel32Base
GetKernel32Base32 PROC
	
	push esi
	assume fs:nothing
	mov esi, dword ptr fs : [30h]   ; esi = PEB�ĵ�ַ
	mov esi, [esi + 0Ch]            ; esi = ָ��PEB_LDR_DATA�ṹ��ָ��
	mov esi, [esi + 1Ch]            ; esi = ģ������ָ��InInit...List
	mov esi, [esi]                   ; esi = ���������еĵڶ�����Ŀ
	mov esi, [esi + 08h]            ; esi = ��ȡKernel32.dll��ַ��ע1��
	mov eax, esi
	pop esi
	ret

GetKernel32Base32 endp


;�ڴ濽��
Memcpy32 PROC

	push ebp
	mov ebp,esp
	sub esp,50h

	mov ecx, [ebp+10h]	;	������������С
	mov esi, [ebp+0ch]	;	�ڶ������� Դ��ַ
	mov edi, [ebp+8h]	;	��һ������ Ŀ���ַ
	rep movsb;

	add esp,50h
	mov esp,ebp
	pop ebp

	ret ;


Memcpy32 endp



;�����ڴ�λ0
SetMemZero32 PROC

	push ebp
	mov ebp,esp
	sub esp,50h

	mov edi,[ebp+8h]
	xor eax,eax
	mov ecx,[ebp+0ch]
	cld
	rep stosb

	add esp,50h
	mov esp,ebp
	pop ebp
	ret;

SetMemZero32 endp

;�Ƿ񱻵���
Debug_PEBBegingDebug PROC

	xor eax,eax							;���eax
	mov eax, fs:[ 0x30 ];               ;��ȡPEB
    mov al, byte ptr ds : [eax + 0x2 ]  ;��ȡPeb.BegingDebugged
    
Debug_PEBBegingDebug endp


end