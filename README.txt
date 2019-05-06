NAME

Notepad++ CTags View


DESCRIPTION

The original plugin was posted here:
https://sourceforge.net/projects/tagsview/

Uses ctags (http://ctags.sourceforge.net/) to index a file and provide
a docking panel viewer.


COMPILING

I compiled with MS Visual Studio Community 2017 and this seems to work
OK.

For 32-bit:
  [x86 Native Tools Command Prompt for VS 2017]
  C:\> set Configuration=Release
  C:\> set Platform=x86
  C:\> msbuild

For 64-bit:
  [x64 Native Tools Command Prompt for VS 2017]
  C:\> set Configuration=Release
  C:\> set Platform=x64
  C:\> msbuild

INSTALLATION

Copy the:

32-bit:
    ./bin/TagsView.dll

64-bit:
    ./bin64/TagsView.dll

to the Notepad++ plugins folder:
  - In N++ <7.6, directly in the plugins/ folder
  - In N++ >=7.6, in a directory called TagsView in the plugins/ folder
    (plugins/TagsView/)

Also need `ctags.exe` in a TagsView subdirectory adjacent to the DLL:

  ./TagsView.dll
  ./TagsView/ctags.exe
