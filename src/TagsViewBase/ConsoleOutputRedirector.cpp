#include "ConsoleOutputRedirector.h"

#define DEFAULT_PIPE_SIZE 0
#define PIPE_READBUFSIZE  4096
#define MAX_CMDLINE       4096

static bool isWindowsNT()
{
    OSVERSIONINFO osv;
    osv.dwOSVersionInfoSize = sizeof(osv);
    ::GetVersionEx(&osv);
    return (osv.dwPlatformId >= VER_PLATFORM_WIN32_NT);
}

static void readFromPipe(HANDLE hPipe, 
#if CONSOLE_OUTPUT_REDIRECTOR_USES_VECTOR
  vector<CConsoleOutputRedirector::ch_t>& data
#else
  basic_string<CConsoleOutputRedirector::ch_t>& data
#endif
)
{
    DWORD dwBytesRead;
    CConsoleOutputRedirector::ch_t buf[PIPE_READBUFSIZE];
    const DWORD uMaxBytesToRead = (PIPE_READBUFSIZE - 1)*sizeof(CConsoleOutputRedirector::ch_t);
    
    do {
        ::Sleep(10); // it prevents from 100% CPU usage while reading!
        
        dwBytesRead = 0;
        if ( ::PeekNamedPipe(hPipe, NULL, 0, NULL, &dwBytesRead, NULL) 
          && (dwBytesRead > 0) )
        {
            dwBytesRead = 0;
            if ( ::ReadFile(hPipe, buf, uMaxBytesToRead, &dwBytesRead, NULL)
              && (dwBytesRead > 0) )
            {
                WORD w = 0;
                
                data.reserve( data.size() + dwBytesRead );
                while ( w < dwBytesRead )
                {
                    data.push_back( buf[w] );
                    ++w;
                }
            }
        }
        
    } while ( dwBytesRead > 0 );
}

void CConsoleOutputRedirector::ClearBuffer()
{
    _data.clear();
}

static void copyCmdLine(TCHAR* pDst, const TCHAR* pSrc)
{
    unsigned short u = 0;
    while ( (++u < MAX_CMDLINE) && (*pDst++ = *pSrc++) ) ;
    *pDst = 0;
}

unsigned int CConsoleOutputRedirector::Execute(const TCHAR* cszCommandLine)
{
    _data.clear();

    if ( cszCommandLine && cszCommandLine[0] )
    {
        SECURITY_DESCRIPTOR sd;
        SECURITY_ATTRIBUTES sa;

        HANDLE hStdOutReadPipe = NULL;
        HANDLE hStdOutWritePipe = NULL;

        if ( isWindowsNT() )
        {
            ::InitializeSecurityDescriptor( &sd, SECURITY_DESCRIPTOR_REVISION );
            ::SetSecurityDescriptorDacl( &sd, TRUE, NULL, FALSE );
            sa.lpSecurityDescriptor = &sd;
        }
        else
        {
            sa.lpSecurityDescriptor = NULL;
        }
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;

        if ( ::CreatePipe(&hStdOutReadPipe, &hStdOutWritePipe, &sa, DEFAULT_PIPE_SIZE) )
        {
            PROCESS_INFORMATION pi;
            STARTUPINFO         si;
            TCHAR               szCmdLine[MAX_CMDLINE];

            ::SetHandleInformation(hStdOutReadPipe, HANDLE_FLAG_INHERIT, 0);

            // initialize STARTUPINFO struct
            ::ZeroMemory( &si, sizeof(STARTUPINFO) );
            si.cb = sizeof(STARTUPINFO);
            si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
            si.wShowWindow = SW_HIDE;
            si.hStdInput = NULL;
            si.hStdOutput = hStdOutWritePipe;
            si.hStdError = hStdOutWritePipe;

            ::ZeroMemory( &pi, sizeof(PROCESS_INFORMATION) );

            copyCmdLine(szCmdLine, cszCommandLine);

            if ( ::CreateProcess(
                     NULL, 
                     szCmdLine,
                     NULL,                     // security
                     NULL,                     // security
                     TRUE,                     // inherits handles
                     CREATE_NEW_PROCESS_GROUP, // creation flags
                     NULL,                     // environment
                     NULL,                     // current directory
                     &si,                      // startup info
                     &pi                       // process info
               ) )
            {
                ::CloseHandle(pi.hThread);

                ::WaitForSingleObject(pi.hProcess, 100);

                do {
                    readFromPipe(hStdOutReadPipe, _data);
                } while ( ::WaitForSingleObject(pi.hProcess, 50) == WAIT_TIMEOUT );
                
                readFromPipe(hStdOutReadPipe, _data);

                ::CloseHandle(pi.hProcess);
            }

            ::CloseHandle(hStdOutReadPipe);
            ::CloseHandle(hStdOutWritePipe);
        }
    }

    #if CONSOLE_OUTPUT_REDIRECTOR_USES_VECTOR
      if ( !_data.empty() )
      {
          _data.push_back(0); // trailing '\0'
      }
    #endif

    return static_cast<unsigned int>( _data.size() );
}

const CConsoleOutputRedirector::ch_t* CConsoleOutputRedirector::GetBuffer() const
{
    static const ch_t empty_buf[2] = { 0, 0 };

    #if CONSOLE_OUTPUT_REDIRECTOR_USES_VECTOR
      return ( _data.empty() ? empty_buf : &_data[0] );
    #else
      return ( _data.empty() ? empty_buf : _data.c_str() );
    #endif
}

const unsigned int CConsoleOutputRedirector::GetBufferSize() const
{
    #if CONSOLE_OUTPUT_REDIRECTOR_USES_VECTOR
      unsigned int uSize = static_cast<unsigned int>( _data.size() );
      if ( uSize > 0 )  --uSize; // exclude trailing '\0'
      return uSize;
    #else
      return static_cast<unsigned int>( _data.size() );
    #endif
}

void CConsoleOutputRedirector::ReserveBuffer(unsigned int uBufferSize)
{
    _data.reserve(uBufferSize);
}
