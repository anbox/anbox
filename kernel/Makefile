#/usr/bin/make
VERSION = $(shell dpkg-parsechangelog -SVersion)
SRC = $(DESTDIR)/usr/src

all:

clean:

install:
	install -d $(SRC)
	for d in ashmem binder ; do \
		cp -a $$d $(SRC)/anbox-modules-$$d-$(VERSION) ; \
	done
