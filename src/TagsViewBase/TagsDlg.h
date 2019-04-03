#ifndef _TAGS_DLG_H_
#define _TAGS_DLG_H_
//---------------------------------------------------------------------------
#include "CTagsResultParser.h"
#include "win32++/include/dialog.h"
#include "win32++/include/listview.h"
#include "win32++/include/treeview.h"
#include "win32++/include/toolbar.h"
#include <string>
#include <map>
#include "EditorWrapper.h"
#include "OptionsManager.h"

using Win32xx::CDialog;
using Win32xx::CListView;
using Win32xx::CTreeView;
using Win32xx::CToolBar;
using Win32xx::tString;
using std::string;
using std::map;

class CEditorWrapper;
class CTagsDlg;

class CTagsDlgChild
{
    public:
        CTagsDlgChild() : m_pDlg(NULL) { }
        void SetTagsDlg(CTagsDlg* pDlg) { m_pDlg = pDlg; }

    protected:
        CTagsDlg* m_pDlg;
};

class CTagsFilterEdit : public CWnd, public CTagsDlgChild
{
    public:
        LRESULT DirectMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            return WndProc(uMsg, wParam, lParam);
        }

    protected:
        virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

class CTagsListView : public CListView, public CTagsDlgChild
{
    public:
        LRESULT DirectMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            return WndProc(uMsg, wParam, lParam);
        }

    protected:
        virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

class CTagsTreeView : public CTreeView, public CTagsDlgChild
{
    public:
        LRESULT DirectMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            return WndProc(uMsg, wParam, lParam);
        }

    protected:
        virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

class CTagsDlg : public CDialog
{
    public:
        enum eConsts {
            MAX_TAGTYPE = 32,
            MAX_TAGNAME = 128,
            MAX_TAGPATTERN = 1024
        };

        enum eTagsViewMode {
            TVM_NONE = 0,
            TVM_LIST,
            TVM_TREE
        };

        enum eTagsSortMode {
            TSM_NONE = 0,
            TSM_NAME,
            TSM_TYPE,
            TSM_LINE
        };

        enum eListViewColumns {
            LVC_NAME = 0,
            LVC_TYPE,
            LVC_LINE,

            LVC_TOTAL
        };

        enum eTagsViewFocus {
            TVF_FILTEREDIT = 0,
            TVF_TAGSVIEW
        };

        enum eMsg {
            /*WM_ADDTAGS         = (WM_USER + 7001),*/
            WM_UPDATETAGSVIEW  = (WM_USER + 7010),
            WM_CTAGSPATHFAILED = (WM_USER + 7050)
        };

        enum eOptions {
            OPT_NONE = 0,
            OPT_VIEW_MODE,
            OPT_VIEW_SORT,
            OPT_VIEW_WIDTH,
            OPT_VIEW_NAMEWIDTH,
            OPT_COLOR_BKGND,
            OPT_COLOR_TEXT,
            OPT_COLOR_TEXTSEL,
            OPT_COLOR_SELA,
            OPT_COLOR_SELN,

            OPT_COUNT
        };

        static const TCHAR* cszListViewColumnNames[LVC_TOTAL];

    public:        
        CTagsDlg();
        virtual ~CTagsDlg();

        const eTagsSortMode GetSortMode() const { return m_sortMode; }
        const eTagsViewMode GetViewMode() const { return m_viewMode; }
        bool  GoToTag(const TCHAR* cszTagName); // not implemented yet
        void  ParseFile(const TCHAR* const cszFileName);
        void  ReparseCurrentFile();
        void  SetSortMode(eTagsSortMode sortMode);
        void  SetViewMode(eTagsViewMode viewMode, eTagsSortMode sortMode);
        void  UpdateTagsView();
        void  UpdateCurrentItem(); // set current item according to caret pos
        void  UpdateNavigationButtons();

        void  CloseDlg()  { EndDialog(0); }

        // MUST be called manually to read the options
        void  ReadOptions()  { initOptions(); m_opt.ReadOptions(m_optRdWr); }

        // MUST be called manually to save the options
        void  SaveOptions()  { m_opt.SaveOptions(m_optRdWr); }

        // MUST be called manually to set required options reader-writer
        void  SetOptionsReaderWriter(COptionsReaderWriter* optRdWr) { m_optRdWr = optRdWr; }

        // MUST be called manually to set required editor wrapper
        void  SetEditorWrapper(CEditorWrapper* pEdWr) { m_pEdWr = pEdWr; }
        
        // MUST be called manually to set required path to ctags.exe
        void  SetCTagsPath(const tString& ctagsPath = _T("TagsView\\ctags.exe")) { m_ctagsPath = ctagsPath; }

        void  SetFocusTo(eTagsViewFocus tvf = TVF_TAGSVIEW)
        {
            switch ( tvf )
            {
                case TVF_FILTEREDIT:
                    m_edFilter.SetFocus();
                    break;

                case TVF_TAGSVIEW:
                    if ( m_lvTags.IsWindowVisible() )
                        m_lvTags.SetFocus();
                    else if ( m_tvTags.IsWindowVisible() )
                        m_tvTags.SetFocus();
                    else
                        this->SetFocus();
                    break;
            }
        }

        static eTagsViewMode toViewMode(int viewMode)
        {
            switch ( viewMode )
            {
                case TVM_TREE:
                    return TVM_TREE;
            }
            return TVM_LIST;
        }

        static eTagsSortMode toSortMode(int sortMode)
        {
            switch ( sortMode )
            {
                case TSM_NAME:
                    return TSM_NAME;
                case TSM_TYPE:
                    return TSM_TYPE;
            }
            return TSM_LINE;
        }

        COptionsManager& GetOptions() { return m_opt; }

    protected:
        virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual void EndDialog(INT_PTR nResult);
        virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
        virtual void OnCancel();
        virtual BOOL OnInitDialog();
        virtual void OnOK();

        virtual LRESULT OnNotify(WPARAM wParam, LPARAM lParam);

        void OnAddTags(const string& s, bool isUTF8);
        void OnSize(bool bInitial = false);
        
        void OnPrevPosClicked();
        void OnNextPosClicked();
        void OnParseClicked();

        int addListViewItem(int nItem, const CTagsResultParser::tTagData& tagData);
        HTREEITEM addTreeViewItem(HTREEITEM hParent, const CTagsResultParser::tTagData& tagData);
        bool isTagMatchFilter(const string& tagName);
        void sortTagsByLine();
        void sortTagsByName();
        void sortTagsByType();

        void checkCTagsPath();

        CTagsResultParser::tags_map::iterator getTagByLine(const int line);
        CTagsResultParser::tags_map::iterator getTagByName(const string& tagName);

        virtual void initOptions();

        static DWORD WINAPI CTagsDlg::CTagsThreadProc(LPVOID lpParam);

    protected:
        friend class CTagsFilterEdit;
        friend class CTagsListView;
        friend class CTagsTreeView;

        eTagsViewMode   m_viewMode;
        eTagsSortMode   m_sortMode;
        DWORD           m_dwLastTagsThreadID;
        HANDLE          m_hTagsThreadEvent;
        CToolBar        m_tbButtons;
        CTagsFilterEdit m_edFilter;
        CTagsListView   m_lvTags;
        CTagsTreeView   m_tvTags;
        string          m_tagFilter;
        tString         m_ctagsPath;
        CTagsResultParser::tags_map m_tags;
        bool            m_isUTF8tags;
        COptionsManager m_opt;
        COptionsReaderWriter* m_optRdWr;
        CEditorWrapper* m_pEdWr;
        int  m_prevSelStart;
        volatile LONG m_nTagsThreadCount;
};

//---------------------------------------------------------------------------
#endif
