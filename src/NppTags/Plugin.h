#ifndef _PLUGIN_H_
#define _PLUGIN_H_
//---------------------------------------------------------------------------
#include "../TagsViewBase/EditorWrapper.h"
#include "../TagsViewBase/TagsDlg.h"
#include <windows.h>
#include "PluginInterface.h"
#include "Notepad_plus_msgs.h"
#include "Docking.h"

using Win32xx::CWinApp;

class CNppTagsDlg : public CTagsDlg
{
    public:
        enum eFuncItem {
            EFI_TAGSVIEW = 0,
            EFI_SETTINGS,

            EFI_COUNT
        };

        static FuncItem FUNC_ARRAY[EFI_COUNT];

    public:
        CNppTagsDlg() : m_hNppWnd(NULL)
        {
        }

        void SetNppWnd(HWND hNppWnd)
        {
            m_hNppWnd = hNppWnd;
        }

    protected:
        virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            if ( uMsg == WM_NOTIFY )
            {
                NMHDR* pnmh = (NMHDR*) lParam;
                if ( pnmh->hwndFrom == m_hNppWnd )
                {
                    if ( LOWORD(pnmh->code) == DMN_CLOSE )
                    {
                        ::SendMessage( m_hNppWnd, NPPM_SETMENUITEMCHECK, 
                            FUNC_ARRAY[EFI_TAGSVIEW]._cmdID, FALSE );
                        return 0;
                    }
                }
            }

            return CTagsDlg::DialogProc(uMsg, wParam, lParam);
        }

    protected:
        HWND m_hNppWnd;
};

class CTagsViewPlugin : public CWinApp, public CEditorWrapper
{
    public:
        CTagsViewPlugin() : CEditorWrapper(&m_tagsDlg)
        {
            m_tagsDlg.SetEditorWrapper(this);
            m_hTabIcon = NULL;
            m_TB_Icon.hToolbarBmp = NULL;
            m_TB_Icon.hToolbarIcon = NULL;
        }

        virtual void ewDoSaveFile();
        // save current file

        virtual void ewDoSetFocus();
        // set focus to the editor's window
        
        virtual void ewDoSetSelection(int selStart, int selEnd);
        // set selection in current file

        virtual HWND ewGetEditHwnd() const;
        // current edit window

        virtual t_string ewGetFilePathName() const;
        // current file pathname (e.g. "C:\My Project\File Name.cpp")

        virtual int ewGetLineFromPos(int pos) const; // 0-based

        virtual int ewGetPosFromLine(int line) const; // 0-based

        virtual int ewGetSelectionPos(int& selEnd) const;

        //virtual t_string ewGetTextLine(int line) const = 0;

        virtual bool ewIsFileSaved() const;
        // true when current file is saved (unmodified)

        virtual void ewOnTagsViewClose();
        // close TagsView dialog
        
        const CTagsDlg& GetTagsDlg() const { return m_tagsDlg; }
        CTagsDlg& GetTagsDlg() { return m_tagsDlg; }

        const NppData& GetNppData() const { return m_nppData; }

        void SetNppData(const NppData& nppd)
        {
            m_nppData = nppd;
            m_tagsDlg.SetNppWnd(nppd._nppHandle);
        }
        
        void OnNppReady();
        void OnNppTBModification();
        
        void Initialize(HINSTANCE hInstance);
        void Uninitialize();

    public:
        static const TCHAR* PLUGIN_NAME;

        //HWND    getCurrentEdit() const;
        LRESULT SendNppMsg(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0) const;
        LRESULT SendNppMsg(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0);

        HICON GetTabIcon() const { return m_hTabIcon; }


    protected:
        CIniOptionsReaderWriter m_optRdWr;
        CNppTagsDlg  m_tagsDlg;
        NppData      m_nppData;
        HICON        m_hTabIcon;
        toolbarIcons m_TB_Icon;
};

//---------------------------------------------------------------------------
#endif
