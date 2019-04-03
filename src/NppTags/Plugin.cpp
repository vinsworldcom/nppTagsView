#include "Plugin.h"
#include "../TagsViewBase/resource.h"

// CTagsViewPlugin class
//----------------------------------------------------------------------------

const TCHAR* CTagsViewPlugin::PLUGIN_NAME = _T("TagsView");

void funcTagsView();
void funcSettings();

FuncItem CNppTagsDlg::FUNC_ARRAY[EFI_COUNT] = {
    { _T("TagsView"), funcTagsView, 0, false, NULL },
    { _T("Settings"), funcSettings, 0, false, NULL }
};

void CTagsViewPlugin::ewDoSaveFile()
{
    SendNppMsg(NPPM_SAVECURRENTFILE);
}

void CTagsViewPlugin::ewDoSetFocus()
{
    const HWND hSciEdit = ewGetEditHwnd();
    ::SetFocus(hSciEdit);
}

void CTagsViewPlugin::ewDoSetSelection(int selStart, int selEnd)
{
    const HWND hSciEdit = ewGetEditHwnd();
    const int line = (int) ::SendMessage( hSciEdit, SCI_LINEFROMPOSITION, selStart, 0 );
    ::SendMessage( hSciEdit, WM_SETREDRAW, (WPARAM) FALSE, 0 );
    ::SendMessage( hSciEdit, SCI_SETSEL, selStart, selEnd );
    ::SendMessage( hSciEdit, SCI_GOTOLINE, line, 0 );
    int firstLine = (int) ::SendMessage( hSciEdit, SCI_GETFIRSTVISIBLELINE, 0, 0 );
    if ( firstLine < line )
        firstLine = line - firstLine - 2;
    else
        firstLine = -2;
    ::SendMessage( hSciEdit, SCI_LINESCROLL, 0, firstLine );
    ::SendMessage( hSciEdit, WM_SETREDRAW, (WPARAM) TRUE, 0 );
    ::InvalidateRect( hSciEdit, NULL, TRUE );
    ::UpdateWindow( hSciEdit );    
}

HWND CTagsViewPlugin::ewGetEditHwnd() const
{
    int currentView = 0;
    SendNppMsg( NPPM_GETCURRENTSCINTILLA, 0, (LPARAM) &currentView );
    return ( (currentView == 0) ? 
      m_nppData._scintillaMainHandle : m_nppData._scintillaSecondHandle );
}

CTagsViewPlugin::t_string CTagsViewPlugin::ewGetFilePathName() const
{
    TCHAR szFilePathName[1024];

    szFilePathName[0] = 0;
    SendNppMsg( NPPM_GETFULLCURRENTPATH, 
      sizeof(szFilePathName)/sizeof(TCHAR) - 1, (LPARAM) szFilePathName );

    t_string fileName = szFilePathName;

    return fileName;
}

int CTagsViewPlugin::ewGetLineFromPos(int pos) const
{
    return (int) ::SendMessage( ewGetEditHwnd(), SCI_LINEFROMPOSITION, pos, 0 );
}

int CTagsViewPlugin::ewGetPosFromLine(int line) const
{
    return (int) ::SendMessage( ewGetEditHwnd(), SCI_POSITIONFROMLINE, line, 0 );
}

int CTagsViewPlugin::ewGetSelectionPos(int& selEnd) const
{
    const HWND hSciEdit = ewGetEditHwnd();
    int selStart = (int) ::SendMessage( hSciEdit, SCI_GETSELECTIONSTART, 0, 0 );
    selEnd = (int) ::SendMessage( hSciEdit, SCI_GETSELECTIONEND, 0, 0 );

    return selStart;
}

/*
CTagsViewPlugin::t_string CTagsViewPlugin::ewGetTextLine(int line) const
{
}
*/

bool CTagsViewPlugin::ewIsFileSaved() const
{
    return (::SendMessage(ewGetEditHwnd(), SCI_GETMODIFY, 0, 0) == 0);
}

void CTagsViewPlugin::ewOnTagsViewClose()
{
    ewClearNavigationHistory(true);

    if ( GetTagsDlg().GetHwnd() )
    {
        if ( GetTagsDlg().IsWindowVisible() )
        {
            SendNppMsg( NPPM_DMMHIDE, 0, (LPARAM) GetTagsDlg().GetHwnd() );
            SendNppMsg( NPPM_SETMENUITEMCHECK, 
              CNppTagsDlg::FUNC_ARRAY[CNppTagsDlg::EFI_TAGSVIEW]._cmdID, FALSE );
        }
    }

    /*Uninitialize();*/
}

/*HWND CTagsViewPlugin::getCurrentEdit() const
{
    int currentView = 0;
    SendNppMsg( NPPM_GETCURRENTSCINTILLA, 0, (LPARAM) &currentView );
    return ( (currentView == 0) ? 
        m_nppData._scintillaMainHandle : m_nppData._scintillaSecondHandle );
}*/

LRESULT CTagsViewPlugin::SendNppMsg(UINT uMsg, WPARAM wParam , LPARAM lParam ) const
{
    return ::SendMessage(m_nppData._nppHandle, uMsg, wParam, lParam);
}

LRESULT CTagsViewPlugin::SendNppMsg(UINT uMsg, WPARAM wParam , LPARAM lParam )
{
    return ::SendMessage(m_nppData._nppHandle, uMsg, wParam, lParam);
}

void CTagsViewPlugin::OnNppReady()
{
    TCHAR szConfigPath[2*MAX_PATH];
    
    szConfigPath[0] = 0;
    SendNppMsg( NPPM_GETPLUGINSCONFIGDIR, sizeof(szConfigPath) - 1, (LPARAM) szConfigPath );

    if ( szConfigPath[0] )
    {
        lstrcat( szConfigPath, _T("\\TagsView.ini") );
        m_optRdWr.SetIniFilePathName(szConfigPath);
    }

    m_tagsDlg.SetOptionsReaderWriter(&m_optRdWr);
    m_tagsDlg.ReadOptions();
}

void CTagsViewPlugin::OnNppTBModification()
{
    SendNppMsg( NPPM_ADDTOOLBARICON, 
      CNppTagsDlg::FUNC_ARRAY[CNppTagsDlg::EFI_TAGSVIEW]._cmdID,
      (LPARAM) &m_TB_Icon );
}

void CTagsViewPlugin::Initialize(HINSTANCE hInstance)
{
    TCHAR szCurDir[2*MAX_PATH];
    unsigned int uLen;
    
    SetResourceHandle(hInstance);
    m_hTabIcon = (HICON) ::LoadImage( hInstance, 
      MAKEINTRESOURCE(IDI_TAGSVIEW), IMAGE_ICON, 0, 0, 
      LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT );
    m_TB_Icon.hToolbarBmp = (HBITMAP) ::LoadImage( hInstance,
     MAKEINTRESOURCE(IDB_TAGSVIEW), IMAGE_BITMAP, 0, 0, 
     LR_DEFAULTSIZE | LR_LOADMAP3DCOLORS );
    
    uLen = ::GetModuleFileName( (HMODULE) hInstance, szCurDir, sizeof(szCurDir) - 1 );
    while ( uLen > 0 )
    {
        --uLen;
        if ( (szCurDir[uLen] == _T('\\')) || (szCurDir[uLen] == _T('/')) )
        {
            break;
        }
    }
    szCurDir[uLen] = 0;

    if ( uLen > 0 )
    {
        tString ctagsPath = szCurDir;
        ctagsPath += _T("\\TagsView\\ctags.exe");
        m_tagsDlg.SetCTagsPath(ctagsPath);
    }
}

void CTagsViewPlugin::Uninitialize()
{
    m_tagsDlg.SaveOptions();
    if ( m_hTabIcon )
    {
        ::DestroyIcon(m_hTabIcon);
        m_hTabIcon = NULL;
    }
    if ( m_TB_Icon.hToolbarBmp )
    {
        ::DeleteObject(m_TB_Icon.hToolbarBmp);
        m_TB_Icon.hToolbarBmp = NULL;
    }
}

//----------------------------------------------------------------------------


// global vars
CTagsViewPlugin thePlugin;

// static func
void funcTagsView()
{
    static tTbData dockData;
    static TCHAR   szPluginName[64];

    bool bParseFile = true;
    
    if ( thePlugin.GetTagsDlg().GetHwnd() )
    {
        UINT uMsg = NPPM_DMMSHOW;
        BOOL bCheck = TRUE;
        if ( thePlugin.GetTagsDlg().IsWindowVisible() )
        {
            thePlugin.ewClearNavigationHistory(true);

            uMsg = NPPM_DMMHIDE;
            bCheck = FALSE;
            bParseFile = false;
        }
        
        thePlugin.SendNppMsg( uMsg, 0, (LPARAM) dockData.hClient );
        thePlugin.SendNppMsg( NPPM_SETMENUITEMCHECK, 
          CNppTagsDlg::FUNC_ARRAY[CNppTagsDlg::EFI_TAGSVIEW]._cmdID,
          bCheck );
    }
    else
    {
        lstrcpy(szPluginName, CTagsViewPlugin::PLUGIN_NAME);
        thePlugin.GetTagsDlg().DoModeless();

        dockData.hClient = thePlugin.GetTagsDlg().GetHwnd();
        dockData.pszName = szPluginName;
        dockData.dlgID = -1;
        dockData.uMask = DWS_DF_CONT_LEFT | DWS_ICONTAB;
        dockData.hIconTab = thePlugin.GetTabIcon();
        dockData.pszAddInfo = NULL;
        dockData.rcFloat.left = 0;
        dockData.rcFloat.top = 0;
        dockData.rcFloat.right = 0;
        dockData.rcFloat.bottom = 0;
        dockData.iPrevCont = -1;
        dockData.pszModuleName = CTagsViewPlugin::PLUGIN_NAME;

        thePlugin.SendNppMsg( NPPM_DMMREGASDCKDLG, 0, (LPARAM) &dockData );
        thePlugin.SendNppMsg( NPPM_MODELESSDIALOG, MODELESSDIALOGADD, (LPARAM) dockData.hClient );
        thePlugin.SendNppMsg( NPPM_DMMSHOW, 0, (LPARAM) dockData.hClient );
        thePlugin.SendNppMsg( NPPM_SETMENUITEMCHECK, 
          CNppTagsDlg::FUNC_ARRAY[CNppTagsDlg::EFI_TAGSVIEW]._cmdID,
          TRUE );
    }

    if ( bParseFile )
    {
        thePlugin.GetTagsDlg().ReparseCurrentFile();
    }
}

// static func
void funcSettings()
{
    ::MessageBox(
        thePlugin.ewGetMainHwnd(),
        _T("Oops, settings are not implemented yet..."),
        _T("TagsView plugin"),
        MB_OK
    );
}


extern "C" __declspec(dllexport) void setInfo(NppData nppd)
{
    thePlugin.SetNppData(nppd);
    thePlugin.ewSetMainHwnd(nppd._nppHandle);
}

extern "C" __declspec(dllexport) const TCHAR * getName()
{
    return CTagsViewPlugin::PLUGIN_NAME;
}

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int * pnbFuncItems)
{
    *pnbFuncItems = CNppTagsDlg::EFI_COUNT;
    return CNppTagsDlg::FUNC_ARRAY;
}

extern "C" __declspec(dllexport) void beNotified(SCNotification * pscn)
{
    //static CTagsViewPlugin::t_string fileToBeClosed;
    
    if ( pscn->nmhdr.hwndFrom == thePlugin.GetNppData()._nppHandle )
    {
        switch ( pscn->nmhdr.code )
        {
            case NPPN_BUFFERACTIVATED:
                thePlugin.ewOnFileActivated();
                break;

            case NPPN_FILESAVED:
                thePlugin.ewOnFileSaved();
                break;

            case NPPN_FILEOPENED:
                thePlugin.ewOnFileOpened();
                break;

            case NPPN_FILEBEFORECLOSE:
                //fileToBeClosed = thePlugin.ewGetFilePathName();
                break;

            case NPPN_FILECLOSED:
                thePlugin.ewOnFileClosed();
                //fileToBeClosed.clear();
                break;

            case NPPN_TBMODIFICATION:
                thePlugin.OnNppTBModification();
                break;

            case NPPN_READY:
                thePlugin.OnNppReady();
                break;
        }
    }
    else
    {
        switch ( pscn->nmhdr.code )
        {
            case SCN_PAINTED:
                thePlugin.ewOnSelectionChanged();
                break;
        }
    }
}

extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
    return 1;
}

#ifdef UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode()
{
    return TRUE;
}
#endif //UNICODE

extern "C" BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    switch ( dwReason )
    {
        case DLL_PROCESS_ATTACH:
            thePlugin.Initialize(hInstance);
            break;

        case DLL_PROCESS_DETACH:
            thePlugin.Uninitialize();
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        default:
            break;
    }
    
    return TRUE;
}
