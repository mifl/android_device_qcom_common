AM_CPPFLAGS := \
        -I../

c_sources := \
        dec.c \
        gpt-utils.c \
        oem-updater.c

lib_LTLIBRARIES := librecovery_updater_msm.la
librecovery_updater_msm_la_SOURCES := $(c_sources)
librecovery_updater_msm_la_CFLAGS := $(AM_CFLAGS)
librecovery_updater_msm_la_CPPFLAGS := $(AM_CPPFLAGS)
librecovery_updater_msm_la_LIBADD := ../edify/libedify.la -lsparse
librecovery_updater_msm_la_LDFLAGS := -all-static
