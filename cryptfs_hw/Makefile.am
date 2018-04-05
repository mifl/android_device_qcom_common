ACLOCAL_AMFLAGS = -I m4
cryptfs_hw_c_sources = cryptfs_hw.c

# Build libcryptfs_hw.so
lib_LTLIBRARIES = libcryptfs_hw.la

libcryptfs_hw_la_SOURCES = $(cryptfs_hw_c_sources)
libcryptfs_hw_la_CFLAGS = -I$(WORKSPACE)/hardware/libhardware/include/hardware
if USE_EMMC_ICE
libcryptfs_hw_la_CFLAGS += -DUSE_ICE_FOR_STORAGE_ENCRYPTION
endif
libcryptfs_hw_la_LDFLAGS = -lcutils -lutils -ldl -lhardware

pkgconfigdir := $(libdir)/pkgconfig
pkgconfig_DATA := libcryptfs_hw.pc
EXTRA_DIST := $(pkgconfig_DATA)
