@ECHO OFF
CD /d %~dp0

md debug\xbspeed release\xbspeed
xcopy /e /y %~dp0\xbspeed %~dp0\debug\xbspeed
xcopy /e /y %~dp0\xbspeed %~dp0\release\xbspeed

CD lewang
python _makerc.py
CD ..
::rcc -binary lewang\default.qrc -o .\essential.res