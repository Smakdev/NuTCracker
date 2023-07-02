CFLAGS= -g
RM=rm -f
OUTFILES=*.exe *.tds *.obj
SRCFILES=main.cpp

all:
	g++ $(SRCFILES) $(CFLAGS) -o Test/NuTCracker
clean:
	@$(RM) $(OUTFILES)
