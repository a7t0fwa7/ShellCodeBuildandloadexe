;������ ml64 /c %(filename).asm
;��� %(filename).obj;%(Outputs)

;AddTowSum_64.asm
ExitProcess PROTO

public GetPc64

.code




GetPc64 proc

   ;mov rax,[rbp+40h]
   mov rax,[rsp]
   mov rbx,0FFFFFFFFFFFFF000h
   and rax,rbx
   ret

GetPc64 endp

ProcFunA PROC



ProcFunA endp



;�Ƿ񱻵���
Debug_PEBBegingDebug PROC

	xor rax,rax							;���eax
	mov rax, gs:[60h];               ;��ȡPEB
    mov rax, [rax + 2h ]  ;��ȡPeb.BegingDebugged
    
Debug_PEBBegingDebug endp


;����TEB
GetTeb64 PROC
    mov rax,gs:[30h]
    ret
GetTeb64 endp

;����PEB
GetPeb64 PROC
    mov rax,gs:[60h]
    ret
GetPeb64 endp

;����PEBLdr
GetPebLdr64 proc
    call GetPeb64
    add eax,18h
    mov eax,[eax]
    ret
GetPebLdr64 endp



; ���� ���ػ�ַ
GetImageBase64 proc

    mov rax, GS:[30h] 
    mov rax, [rax + 60h] 
    mov rax, [rax + 10h] 
    ret
GetImageBase64 endp


;����Kernel32.dll
GetModuleBase64 proc

    push RSI
    mov RSI,gs:[60h]                ;esi = PEB��ַ
    mov RSI,[RSI+18h]              ;ָ��PEB_LDR_DATA �ṹ��
    mov RSI,[RSI+30h]              ;ģ������ָ�룬InInit...List
    mov RAX,RSI
    pop RSI
    ret
GetModuleBase64 endp



;�ڴ濽��  des src size
Mymemcpy64 proc
    push rbp
    mov rbp,rsp
    sub rsp,50h

    push rcx
    push rdx
    mov rcx,r8
    pop rsi
    pop rdi
    
    rep movsb

    add rsp,50h
    mov rsp,rbp
    pop rbp

    ret
Mymemcpy64 endp

;�����ڴ�Ϊ0
MySetMemZero64 proc

    mov rdi,rcx
    xor rax,rax
    mov rcx,r8
    cld
    rep stosb

MySetMemZero64 endp


end