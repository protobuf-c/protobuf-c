SUBDIRS = src

EXTRA_DIST = scripts pkgwriteinfo.in CMakeLists.txt

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libprotobuf-c.pc

# --- packages ---
DEBARCH = `dpkg --print-architecture`
deb:
	test -r protobuf-c-@VERSION@.tar.gz || $(MAKE) dist
	$(srcdir)/scripts/pkgwrite --format=debian \
	  --tarball=protobuf-c-@VERSION@.tar.gz \
	  --output=protobuf-c-packages \
	  --pkgwriteinfo-file=pkgwriteinfo \
	  --arch=$(DEBARCH)
