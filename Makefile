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
export VERSION_ALL = $(VERSION_MAJ).$(VERSION_MIN)

#---------------------------------------

# c compiler
export CC = gcc
# to be a bit stricter
#export CC = g++

# Options for c compiler, *** NO DEBUG ***
export CFLAGS_REL = -Wall -DVERSION_ALL=\"$(VERSION_ALL)\" -O3 -g0

# Options for c compiler, *** DEBUG *** with dump terms
export CFLAGS_DBG = -Wall -DVERSION_ALL=\"$(VERSION_ALL)\" -DDEBUG -D__DUMP_TERMS__ -O0 -g3

# directory for backup
export ArcDir = /home/baccy/BackUp/

# c cross compiler for dos
export DOSCC = i386-pc-msdosdjgpp-gcc

# Options for dos c compiler
export DOSCFLAGS = -Wall -DVERSION_ALL=\"$(VERSION_ALL)\" -O2 -g0 -s

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


DOCDIR = docs
TESTDIR = test

SRCARCFILES = Makefile version $(SRCFILES) $(HDRFILES) $(DEBFILES) $(DOCDIR)/*.texi $(DOCDIR)/Makefile $(TESTDIR)/Makefile $(TESTDIR)/*.src $(TESTDIR)/*.bin COPYING
DOCARCFILES = $(addprefix $(DOCDIR)/, dreamass.html dreamass.info dreamass.txt)

#---------------------------------------

DATE := $(shell date +%Y%m%d)
DATETIME := $(shell date +%Y%m%d_%H%M)
SRCARCNAME = $(ArcDir)dreamass_src_$(DATE)

#---------------------------------------

doc:
	make -C $(DOCDIR) all

test:
	make -C $(TESTDIR) all

#---------------------------------------

#link everything for the release version
dreamass: lin_rel $(OBJFILES_LIN_REL) $(HDRFILES)
	$(CC) $(CFLAGS_REL) -o $@ $(OBJFILES_LIN_REL)
#	strip --remove-section=.comment --remove-section=.note $@

#link everything for the debug version
dreamass_dbg: lin_dbg $(OBJFILES_LIN_DBG) $(HDRFILES)
	$(CC) $(CFLAGS_DBG) -o $@ $(OBJFILES_LIN_DBG)

#generic rule for c files
lin_dbg/%.o: %.c $(HDRFILES)
	$(CC) $(CFLAGS_DBG) -c -o $@ $<

lin_rel/%.o: %.c $(HDRFILES)
	$(CC) $(CFLAGS_REL) -c -o $@ $<


dreamass.exe: dos_rel $(OBJFILES_DOS_REL) $(HFILES)
	$(DOSCC) $(DOSCFLAGS) -o $@ $(OBJFILES_DOS_REL)

dos_rel/%.o: %.c $(HDRFILES)
	$(DOSCC) $(DOSCFLAGS) -c -o $@ $<

#---------------------------------------

lin_dbg:
	mkdir lin_dbg

lin_rel:
	mkdir lin_rel

dos_rel:
	mkdir dos_rel

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
	cp $(DEBTMPDIR)/dreamass_$(VERSION_ALL)-*.dsc $(ArcDir)
	cp $(DEBTMPDIR)/dreamass_$(VERSION_ALL)-*.tar.gz $(ArcDir)
	cp $(DEBTMPDIR)/dreamass_$(VERSION_ALL)-*_i386.changes $(ArcDir)
	cp $(DEBTMPDIR)/dreamass_$(VERSION_ALL)-*_i386.deb $(ArcDir)

#---------------------------------------

clean:
	rm -f $(OBJFILES_LIN_DBG) dreamass_dbg
	rm -f $(OBJFILES_LIN_REL) dreamass
	rm -f $(OBJFILES_DOS_REL) dreamass.exe
	make -C $(DOCDIR) clean
	make -C $(TESTDIR) clean

#---------------------------------------

backup:
	rar a -apdreamass -k -m5 -r -s $(ArcDir)dreamass_complete_$(DATETIME).tar.gz

distrib: dreamass.exe doc
	rar a -apdreamass -k -m5 -s $(SRCARCNAME).rar $(SRCARCFILES)
	rar a -apdreamass -k -m5 -s $(ArcDir)dreamass_doc_$(DATE).rar $(DOCARCFILES)
	rar a -apdreamass -k -m5 -s $(ArcDir)dreamass_dos_$(DATE).rar dreamass.exe COPYING

#---------------------------------------

