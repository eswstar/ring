call ..\..\language\src\locatevc.bat
"C:\Qt\5.15.0\msvc2019\bin\qmake.exe" ring_qt515_core.pro -spec win32-msvc "CONFIG+=qtquickcompiler"
"C:\Qt\Tools\QtCreator\bin\jom\jom.exe"
copy release\ringqt_core.dll ..\..\bin