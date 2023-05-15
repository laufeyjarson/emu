This is the sources and source tree for an Apple // Emulator.

The directories are such so that an NT version and Windows version can be
compiled at the same time without obliterating the .OBJ or .EXE or compiler
temp files.

The SRC and INC directories contain soures and headers; INC needs to be in
the include search path for the compiler and RC.  Put all the files in SRC
in the project, along with the .RC file in INC.

Put each makefile in the OS-specific directory and let the compiler work
there.

Microsoft uses a similar technique and it works well.  Because we used MSVC
to develop the makefile, nmake -f emu.mak will make, or you can open from
within MSVC.  

NT development needs to have a WIN32 macro defined as well.  NT development
is currently using MSVC for NT version 1.1 and Windows development is
currently using MSVC 1.5.  Each of these are the standard user install.

NT development has now moved to MSVC 2.0 for NT.  Along with several cool new
things, NT 2.0 gave us a Mac cross compiler, and we took the time to make
that port.  It's very slow and there are some issues, but, more or less, it's
there.
