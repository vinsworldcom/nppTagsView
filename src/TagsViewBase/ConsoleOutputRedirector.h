#ifndef _CONSOLE_OUTPUT_REDIRECTOR_H_
#define _CONSOLE_OUTPUT_REDIRECTOR_H_
//---------------------------------------------------------------------------
#include <windows.h>

#define CONSOLE_OUTPUT_REDIRECTOR_USES_VECTOR 0

#if CONSOLE_OUTPUT_REDIRECTOR_USES_VECTOR
  #include <vector>
  using std::vector;
#else
  #include <string>  
  using std::basic_string;
#endif

class CConsoleOutputRedirector 
{
    public:
        #if CONSOLE_OUTPUT_REDIRECTOR_USES_VECTOR
          typedef unsigned char ch_t;
        #else
          typedef char ch_t;
        #endif

        void ClearBuffer();
        unsigned int Execute(const TCHAR* cszCommandLine);
        const ch_t* GetBuffer() const;
        const unsigned int GetBufferSize() const;
        void ReserveBuffer(unsigned int uBufferSize);

        #if CONSOLE_OUTPUT_REDIRECTOR_USES_VECTOR
          const vector<ch_t>& GetDataVector() const { return _data; }
        #else
          const basic_string<ch_t>& GetDataString() const { return _data; }
        #endif
    
    protected:
        #if CONSOLE_OUTPUT_REDIRECTOR_USES_VECTOR
          vector<ch_t> _data;
        #else
          basic_string<ch_t> _data;
        #endif
};


//---------------------------------------------------------------------------
#endif
