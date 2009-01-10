#---------------------------------------
#
# Makefile for dreamass
#
# ToDo: use some deps here so every single c file does not depend on all .h's
#
#---------------------------------------
#
# *** CONFIG ***
#

# (1) Select which target to build

#all: dreamass dreamass.1.gz dreamass_dbg dreamass.exe doc
#all: dreamass dreamass.1.gz dreamass_dbg dreamass.exe
#all: dreamass.exe
#all: dreamass dreamass.1.gz
all: dreamass
#all: dreamass_dbg

#---------------------------------------

# get date for this version
export VERSION_DATE := $(shell date +%Y-%m-%d)

# get version number
include version
export VERSION_ALL = $(VERSION_MAJ).$(VERSION_MIN).$(VERSION_SUB)

#---------------------------------------

# c compiler
export CC = gcc

# Options for c compiler, *** NO DEBUG ***
export CFLAGS_REL = -Wall -Wextra -Wconversion -Wshadow -Wcast-qual -Wwrite-strings -Wcast-align -Wpointer-arith -Wmissing-prototypes -Wstrict-prototypes -DVERSION_ALL=\"$(VERSION_ALL)\" -O3 -g0

# Options for c compiler, *** DEBUG *** with dump terms
export CFLAGS_DBG = -Wall -DVERSION_ALL=\"$(VERSION_ALL)\" -DDEBUG -D__DUMP_TERMS__ -O0 -g3

# c cross compiler for dos
export DOSCC = i386-pc-msdosdjgpp-gcc

# c cross compiler for win
export WINCC = i586-mingw32msvc-gcc

# Options for dos c compiler
export DOSCFLAGS = -Wall -DVERSION_ALL=\"$(VERSION_ALL)\" -O2 -g0 -s

# Options for win c compiler
export WINCFLAGS = -Wall -DVERSION_ALL=\"$(VERSION_ALL)\" -O2 -g0 -s

export DISTDIR = distrib
export DEBTMPDIR = /tmp/dreamass_deb

#---------------------------------------
# End of Config
# nothing needs to be changed below
#---------------------------------------

.PHONY: all backup clean deb distrib doc test

.INTERMEDIATE: dreamass.exe.nodpmi


SRCFILES = filelist.c macro.c main.c message.c mne.c navi.c opcodes.c operand.c pass.c preproc.c psyopc.c segment.c srcfile.c srcstack.c term.c termfunc.c variable.c
HDRFILES = da_stdint.h filelist.h globals.h linebuf.h macro.h main.h message.h mne.h navi.h opcodes.h operand.h pass.h preproc.h preproc2.h psyopc.h segment.h sizes.h srcfile.h srcstack.h term.h termfunc.h variable.h
DEBFILES = $(addprefix debian/, changelog compat control copyright dirs docs rules)




OBJFILES = $(patsubst %.c,%.o,$(SRCFILES))

OBJFILES_LIN_DBG = $(addprefix lin_dbg/, $(OBJFILES))
OBJFILES_LIN_REL = $(addprefix lin_rel/, $(OBJFILES))
OBJFILES_DOS_REL = $(addprefix dos_rel/, $(OBJFILES))
OBJFILES_WIN_REL = $(addprefix win_rel/, $(OBJFILES))

DOCDIR = docs
TESTDIR = test

SRCARCFILES = Makefile version $(SRCFILES) $(HDRFILES) $(DEBFILES) $(DOCDIR)/*.texi $(DOCDIR)/Makefile $(TESTDIR)/Makefile $(TESTDIR)/*.src $(TESTDIR)/*.bin COPYING
DOCARCFILES = $(addprefix $(DOCDIR)/, dreamass.html dreamass.info dreamass.txt)

#---------------------------------------

DATE := $(shell date +%Y%m%d)
DATETIME := $(shell date +%Y%m%d_%H%M)

#---------------------------------------

doc:
	make -C $(DOCDIR) all

test:
	make -C $(TESTDIR) all

#---------------------------------------

#link everything for the release version
dreamass: lin_rel $(OBJFILES_LIN_REL) $(HDRFILES)
	$(CC) $(CFLAGS_REL) -o $@ $(OBJFILES_LIN_REL)

#link everything for the debug version
dreamass_dbg: lin_dbg $(OBJFILES_LIN_DBG) $(HDRFILES)
	$(CC) $(CFLAGS_DBG) -o $@ $(OBJFILES_LIN_DBG)

#generic rule for c files
lin_dbg/%.o: %.c $(HDRFILES)
	$(CC) $(CFLAGS_DBG) -c -o $@ $<

lin_rel/%.o: %.c $(HDRFILES)
	$(CC) $(CFLAGS_REL) -c -o $@ $<


dos_rel/dreamass.exe: dos_rel $(OBJFILES_DOS_REL) $(HFILES)
	$(DOSCC) $(DOSCFLAGS) -o $@ $(OBJFILES_DOS_REL)

dos_rel/%.o: %.c $(HDRFILES)
	$(DOSCC) $(DOSCFLAGS) -c -o $@ $<

win_rel/dreamass.exe: win_rel $(OBJFILES_WIN_REL) $(HFILES)
	$(WINCC) $(WINCFLAGS) -o $@ $(OBJFILES_WIN_REL)

win_rel/%.o: %.c $(HDRFILES)
	$(WINCC) $(WINCFLAGS) -c -o $@ $<

#---------------------------------------

lin_dbg:
	mkdir lin_dbg

lin_rel:
	mkdir lin_rel

dos_rel:
	mkdir dos_rel

win_rel:
	mkdir win_rel

#---------------------------------------

dreamass.1.gz: dreamass
	help2man dreamass -o dreamass.1
	rm -f $@
	gzip -9 dreamass.1

#---------------------------------------

# don't build deb from this dir, all files would end up in the source archive
deb:
	rm -rf $(DEBTMPDIR)
	mkdir $(DEBTMPDIR)
	mkdir $(DEBTMPDIR)/dreamass-$(VERSION_ALL)
	cp -p --parents --target-directory=$(DEBTMPDIR)/dreamass-$(VERSION_ALL) $(SRCARCFILES)
	cd $(DEBTMPDIR)/dreamass-$(VERSION_ALL) && dpkg-buildpackage -rfakeroot
	cp $(DEBTMPDIR)/dreamass_$(VERSION_ALL)-*.dsc .
	cp $(DEBTMPDIR)/dreamass_$(VERSION_ALL)-*.tar.gz .
	cp $(DEBTMPDIR)/dreamass_$(VERSION_ALL)-*_i386.changes .
	cp $(DEBTMPDIR)/dreamass_$(VERSION_ALL)-*_i386.deb .

#---------------------------------------

clean:
	rm -f $(OBJFILES_LIN_DBG) dreamass_dbg
	rm -f $(OBJFILES_LIN_REL) dreamass
	rm -f $(OBJFILES_DOS_REL) dos_rel/dreamass.exe
	rm -f $(OBJFILES_WIN_REL) win_rel/dreamass.exe
	make -C $(DOCDIR) clean
	make -C $(TESTDIR) clean

#---------------------------------------

distrib: dos_rel/dreamass.exe win_rel/dreamass.exe doc
	echo clean the old distrib directory
	rm -rf $(DISTDIR)
	
	echo copy the sources
	mkdir -p $(DISTDIR)/src/dreamass
	mkdir -p $(DISTDIR)/src/dreamass/docs
	mkdir -p $(DISTDIR)/src/dreamass/test
	cp -t $(DISTDIR)/src/dreamass --parents $(SRCARCFILES)
	tar -c -f $(DISTDIR)/dreamass_src_$(DATE).tar.gz -v -z -C $(DISTDIR)/src/ dreamass
	
	echo copy the docs
	mkdir -p $(DISTDIR)/docs/dreamass/docs
	cp $(DOCARCFILES) $(DISTDIR)/docs/dreamass/docs/
	tar -c -f $(DISTDIR)/dreamass_doc_$(DATE).tar.gz -v -z -C $(DISTDIR)/docs/ dreamass/docs
	cd $(DISTDIR)/docs/ && zip -9r ../../$(DISTDIR)/dreamass_doc_$(DATE).zip dreamass/docs
	
	# create the dos distrib
	mkdir -p $(DISTDIR)/dos/dreamass
	cp dos_rel/dreamass.exe COPYING $(DISTDIR)/dos/dreamass/
	cd $(DISTDIR)/dos/ && zip -9r ../../$(DISTDIR)/dreamass_dos_$(DATE).zip dreamass
	
	# create the win distrib
	mkdir -p $(DISTDIR)/win/dreamass
	cp win_rel/dreamass.exe COPYING $(DISTDIR)/win/dreamass/
	cd $(DISTDIR)/win/ && zip -9r ../../$(DISTDIR)/dreamass_win_$(DATE).zip dreamass

#---------------------------------------

