# -------------------------------------
#
#   Makefile for Docs
#
# -------------------------------------

include ../version

all: dreamass.txt dreamass.info.gz dreamass.html
#all: dreamass.pdf
#all: dreamass.txt
#all: dreamass.info
#all: dreamass.html

#--------------------------------------

TEXI_FILES = dreamass.texi dreamslogo.texi history.texi

dreamass.dvi: $(TEXI_FILES)
	texi2dvi --clean dreamass.texi

dreamass.pdf: $(TEXI_FILES)
	texi2dvi --pdf --clean dreamass.texi

dreamass.txt: $(TEXI_FILES)
	makeinfo -D "VERSION $(VERSION_ALL)" -D "UPDATED $(VERSION_DATE)" --plaintext -o $@ $<

dreamass.info.gz: dreamass.info
	gzip -9 -c $< >$@

dreamass.info: $(TEXI_FILES)
	makeinfo -D "VERSION $(VERSION_ALL)" -D "UPDATED $(VERSION_DATE)" -o $@ $<

dreamass.html: $(TEXI_FILES)
	makeinfo -D "VERSION $(VERSION_ALL)" -D "UPDATED $(VERSION_DATE)" --html --no-split -o $@ $<

#dreamass_t2h.html: $(TEXI_FILES)
#	texi2html -menu -out_file=$@ $<

#--------------------------------------

clean:
	rm -f dreamass.dvi dreamass.pdf dreamass.txt dreamass.info dreamass.html

#--------------------------------------


