@echo off
call "%VS90COMNTOOLS%\vsvars32.bat"
mkdir bchunk_out
cl bchunk.c getopt.c /link /out:bchunk_out\bchunk.exe
echo Built!
pause