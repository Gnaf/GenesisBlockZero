CC=cl -c
CFLAGS=-c -DSTRICT -G3 -Ow -W3 -Zp -Tp
CFLAGSMT=
LINKER=cl
GUILIBS=/NXCOMPAT /MACHINE:X64 /ERRORREPORT:NONE 
#/ENTRY:wWinMainCRTStartup

op.exe : genesisblock.obj
	$(LINKER) $(GUILIBS) genesisblock.obj   


genesisblock.obj :genesisblock.cpp 
	$(CC) $(CFLAGS) genesisblock.cpp


