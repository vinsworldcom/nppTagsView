#ifndef _tags_editor_wrapper_h_
#define _tags_editor_wrapper_h_
//---------------------------------------------------------------------------
#include <windows.h>
#include <string>
#include <map>
#include <vector>
#include "TagsDlg.h"

class CEditorWrapperInterface
{
    public:
        typedef std::basic_string<TCHAR> t_string;

    public:
        virtual void ewDoSaveFile() = 0;
        // save current file
        
        virtual void ewDoSetFocus() = 0;
        // set focus to the editor's window
        
        virtual void ewDoSetSelection(int selStart, int selEnd) = 0;
        // set selection in current file

        virtual HWND ewGetEditHwnd() const = 0;
        // current edit window

        virtual t_string ewGetFilePathName() const = 0;
        // current file pathname (e.g. "C:\My Project\File Name.cpp")

        virtual int ewGetLineFromPos(int pos) const = 0; // 0-based

        virtual int ewGetPosFromLine(int line) const = 0; // 0-based
        
        virtual int ewGetSelectionPos(int& selEnd) const = 0;
        
        //virtual t_string ewGetTextLine(int line) const = 0;

        virtual bool ewIsFileSaved() const = 0;
        // true when current file is saved (unmodified)

        virtual void ewOnTagsViewClose() = 0;
        // close TagsView dialog
};

class CTagsDlg;

class CEditorWrapper : public CEditorWrapperInterface
{
    public:
        CEditorWrapper(CTagsDlg* pDlg);
        virtual ~CEditorWrapper();

        // seems DoFunctions together with navMap must be located
        // inside TagsDlg, not here !!!!!!!

        bool ewCanNavigateBackward();
        // can navigate backward in current file

        bool ewCanNavigateForward();
        // can navigate forward in current file
        
        void ewClearNavigationHistory(bool bAllFiles);
        // clear navigation history for all files or just current file
        // call it manually
        
        virtual void ewDoNavigateBackward();
        // navigate backward in current file
        // may be overridden if needed

        virtual void ewDoNavigateForward();
        // navigate forward in current file
        // may be overridden if needed
        
        void ewDoParseFile();
        // (re)parse current file
        
        t_string ewGetNavigateBackwardHint();
        // returns "<FunctionName>, line <NN>:\n  <TextLine>"

        t_string ewGetNavigateForwardHint();
        // returns "<FunctionName>, line <NN>:\n  <TextLine>"

        bool ewHasNavigationHistory();
        // true when current file has navigation history
        
        void ewOnFileActivated();
        void ewOnFileClosed();
        void ewOnFileOpened();
        void ewOnFileReloaded();
        void ewOnFileSaved();
        void ewOnSelectionChanged();

        HWND ewGetMainHwnd() const { return m_hMainWnd; }
        void ewSetMainHwnd(HWND hWnd) { m_hMainWnd = hWnd; }
        
        void ewSetNavigationPoint(const t_string& hint, bool incPos = true);

    protected:
        typedef struct sNavigationPoint {
            int selPos;
            t_string hint;
        } tNavigationPoint;
        
        class CNavigationHistory 
        {
            public:
                CNavigationHistory()
                {
                    Clear();
                }

                void Add(int selPos, const t_string& hint, bool incPos)
                {
                    tNavigationPoint np = { selPos, hint };

                    if ( incPos )
                    {
                        m_history.erase( m_history.begin() + m_pos, m_history.end() );
                        ++m_pos;
                    }

                    if ( incPos || (m_pos == m_history.size()) )
                        m_history.push_back(np);
                }
                
                bool CanBackward() const
                {
                    return (m_pos > 0);
                }

                bool CanForward() const
                {
                    return (m_pos + 1 < m_history.size());
                }
                
                void Clear()
                {
                    m_pos = 0;
                    m_history.clear();
                }
                
                int Backward()
                {
                    if ( m_pos > 0 ) 
                    {
                        const tNavigationPoint& np = m_history[--m_pos];
                        return np.selPos;
                    }
                    
                    return -1;
                }

                int Forward()
                {
                    if ( m_pos + 1 < m_history.size() )
                    {
                        const tNavigationPoint& np = m_history[++m_pos];
                        return np.selPos;
                    }
                    
                    return -1;
                }

                t_string GetBackwardHint() const
                {
                    t_string hint;
                    
                    if ( m_pos > 0 ) 
                    {
                        const tNavigationPoint& np = m_history[m_pos - 1];
                        hint = np.hint;
                    }

                    return hint;
                }

                t_string GetForwardHint() const
                {
                    t_string hint;

                    if ( m_pos + 1 < m_history.size() ) 
                    {
                        const tNavigationPoint& np = m_history[m_pos + 1];
                        hint = np.hint;
                    }

                    return hint;
                }

                const tNavigationPoint& GetLastItem() const
                {
                    return m_history.back();
                }
                
                tNavigationPoint& GetLastItem()
                {
                    return m_history.back();
                }

                bool IsEmpty() const
                {
                    return m_history.empty();
                }

            protected:
                std::vector<t_string>::size_type m_pos; 
                std::vector<tNavigationPoint> m_history;
        };

        typedef std::map<t_string, CNavigationHistory> t_navmap;

        t_navmap::const_iterator getNavItr(const t_string& filePathName) const;
        t_navmap::iterator getNavItr(const t_string& filePathName);

        const t_string& getCurrentFilePathName();

    protected:
        CTagsDlg* m_pDlg;
        HWND      m_hMainWnd;
        t_navmap  m_navMap;
        t_string  m_currentFilePathName;
};


//---------------------------------------------------------------------------
#endif
