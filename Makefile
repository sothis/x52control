printinfo:
	echo "specify platform, i.e. make <linux | mac>"

linux:
	$(MAKE) -C . -f Makefile.unix all

mac:
	$(MAKE) -C . -f Makefile.mac104 all

.PHONY: 	clean

clean:
	$(MAKE) -C . -f Makefile.unix clean
	$(MAKE) -C . -f Makefile.mac104 clean

.SILENT:
