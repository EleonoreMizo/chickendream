
@cd /d "%~dp0"

@mkdir "releases"
@if exist "releases\chickendream-rnew.zip" del "releases\chickendream-rnew.zip"
@rmdir /s /q "reltmp"

@mkdir "reltmp"
@mkdir "reltmp\src"
@mkdir "reltmp\build\unix\m4"
@mkdir "reltmp\build\win\common"
@mkdir "reltmp\build\win\chickendream"
@mkdir "reltmp\build\win\test"
@mkdir "reltmp\win32"
@mkdir "reltmp\win64"
@xcopy /I "doc"                                                  "reltmp\doc"
@xcopy /I "src\avs"                                              "reltmp\src\avs"
@xcopy /I "src\avsutl"                                           "reltmp\src\avsutl"
@xcopy /I "src\chkdr"                                            "reltmp\src\chkdr"
@xcopy /I "src\chkdravs"                                         "reltmp\src\chkdravs"
@xcopy /I "src\chkdrvs"                                          "reltmp\src\chkdrvs"
@xcopy /I "src\fgrn"                                             "reltmp\src\fgrn"
@xcopy /I "src\fstb"                                             "reltmp\src\fstb"
@xcopy /I "src\test"                                             "reltmp\src\test"
@xcopy /I "src\vsutl"                                            "reltmp\src\vsutl"
@copy     "src\*.cpp"                                            "reltmp\src"
@copy     "src\*.h"                                              "reltmp\src"
@copy     "src\*.hpp"                                            "reltmp\src"
@copy     "build\unix\autogen.sh"                                "reltmp\build\unix"
@copy     "build\unix\configure.ac"                              "reltmp\build\unix"
@copy     "build\unix\Makefile.am"                               "reltmp\build\unix"
@copy     "build\unix\m4\ax_*.m4"                                "reltmp\build\unix\m4"
@copy     "build\win\*.sln"                                      "reltmp\build\win"
@copy     "build\win\*.vcxproj"                                  "reltmp\build\win"
@copy     "build\win\*.vcxproj.filters"                          "reltmp\build\win"
@copy     "build\win\*.props"                                    "reltmp\build\win"
@copy     "build\win\common\*.vcxproj"                           "reltmp\build\win\common"
@copy     "build\win\common\*.vcxproj.filters"                   "reltmp\build\win\common"
@copy     "build\win\chickendream\*.vcxproj"                     "reltmp\build\win\chickendream"
@copy     "build\win\chickendream\*.vcxproj.filters"             "reltmp\build\win\chickendream"
@copy     "build\win\chickendream\ReleaseWin32\chickendream.dll" "reltmp\win32"
@copy     "build\win\chickendream\Releasex64\chickendream.dll"   "reltmp\win64"
@copy     "build\win\test\*.vcxproj"                             "reltmp\build\win\test"
@copy     "build\win\test\*.vcxproj.filters"                     "reltmp\build\win\test"
@copy     "*.md"                                                 "reltmp"
@copy     "COPYING"                                              "reltmp"

@cd reltmp

del /S *.lo *.o *.dirstamp *.deps

@echo chickendream - Realistic grain generator for Vapoursynth and Avisynth+ | "C:\Program Files (x86)\Infozip\zip.exe" -r -o -9 -z "..\releases\chickendream-rnew.zip" "*.*"
@cd ..

@rmdir /s /q "reltmp"

@pause
