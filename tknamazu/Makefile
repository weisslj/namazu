##---------------------------------------------------------------------------##
##	@(#) Makefile for Tknamazu 
##---------------------------------------------------------------------------##

TKNMZPATH	= /usr/local/namazu/tknamazu
WISHPATH	= /usr/bin/wish
BINDIR_SYS	= /usr/local/bin

### Example for Win32
##TKNMZPATH	= c:/usr/local/namazu/tknamazu
##WISHPATH	= c:\\tcl\\bin\\wish42
##BINDIR_SYS	= c:/usr/local/bin

TRIGGERFILE	= tknamazu		## for unix
##TRIGGERFILE	= tknamazu.bat		## for win


PROGRAMFILE	= tknamazu.tcl
HELPFILE	= tknamazu.hlp
REFERENCE	= tknamazu.gif $(HELPFILE)
CONFIGURATION	= .tknmzrc

CHMOD		= /bin/chmod
MKDIR		= /bin/mkdir
CP		= /bin/cp
RM		= /bin/rm
MV		= /bin/mv
NKF		= /usr/bin/nkf

all: $(TRIGGERFILE)

tknamazu: 
	@echo "== Making $@"
	@echo "#!/bin/sh" > $@
	@echo "TKNMZPATH=$(TKNMZPATH)" >> $@
	@echo "export TKNMZPATH" >> $@
	@echo "exec $(WISHPATH) ${TKNMZPATH}/tknamazu.tcl" >> $@
	$(CHMOD) +x $@
	$(RM) -f $(CONFIGURATION)
	$(CP) .unixtknmzrc $(CONFIGURATION)

tknamazu.bat: 
	@echo "== Making $@"
	@echo "@echo off" > $@
	@echo "SET TKNMZPATH=$(TKNMZPATH)" >> $@
	@echo "$(WISHPATH) %TKNMZPATH%/tknamazu.tcl" >> $@
	$(RM) -f $(CONFIGURATION)
	$(NKF) -s -O -c .wintknmzrc
	$(MV) -f nkf.out $(CONFIGURATION)
	$(NKF) -s -O -c $(PROGRAMFILE)
	$(MV) -f nkf.out $(PROGRAMFILE)
	$(NKF) -s -O -c $(HELPFILE)
	$(MV) -f nkf.out $(HELPFILE)

install: all
	if [ ! -d $(TKNMZPATH) ]; then \
	    $(MKDIR) -p $(TKNMZPATH); \
	fi
	if [ ! -d $(BINDIR_SYS) ]; then \
	    $(MKDIR) -p $(BINDIR_SYS); \
	fi
	$(CP) $(TRIGGERFILE) $(BINDIR_SYS)
	$(CP) $(PROGRAMFILE) $(REFERENCE) $(CONFIGURATION) $(TKNMZPATH)
	@echo ""
	@echo "!!! Please copy .tknmzrc to your home directory. And customize it. !!!"
	@echo "%$(CP) $(TKNMZPATH)/$(CONFIGURATION) ~/$(CONFIGURATION)"

test:
	@echo "No tests"

clean:
	$(RM) -f $(TRIGGERFILE) $(CONFIGURATION)
	$(NKF) -e -O -d $(PROGRAMFILE)
	$(MV) -f nkf.out $(PROGRAMFILE)
	$(NKF) -e -O -d $(HELPFILE)
	$(MV) -f nkf.out $(HELPFILE)

tar:
	$(MAKE) clean
	(ARCHIVE=`pwd|sed 's@[^/]*/@@g'`; \
	    ( \
	        cd ..; \
	        tar cf - $${ARCHIVE}/* $${ARCHIVE}/.??* | \
	        gzip -c \
	    ) > ../$${ARCHIVE}.tar.gz \
	)

