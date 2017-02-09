#/usr/bin/make
VERSION = 1
SRC = $(DESTDIR)/usr/src

all:

clean:

install:
	install -d $(SRC)
	for d in ashmem binder ; do \
		cp -a $$d $(SRC)/anbox-modules-$$d-$(VERSION) ; \
	done
