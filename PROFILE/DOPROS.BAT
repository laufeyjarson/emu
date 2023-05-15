rem doprofs.bat
del emulator.mea
mfile copy ":Hard Drive:EMU:emulator.mea" emulator.mea
del emunt20.map
copy ..\nt\mac_prof\emunt20.map .
c:\msvc20\m68k\bin\mprof /rs /m emunt20.map emulator.mea > mprofs.txt
c:\msvc20\m68k\bin\mprof /ra /m emunt20.map emulator.mea > mprofa.txt
