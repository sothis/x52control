all:	linux win32

linux:
	$(MAKE) -C . -f Makefile.unix all

win32:
	$(MAKE) -C . -f Makefile.win32 all

.PHONY: 	clean

clean:
	echo -e "\033[0;31m[ cleaning ]\033[0m"
	-rm -rdf build
	-rm -rdf /tmp/tmp.reg

.SILENT:
