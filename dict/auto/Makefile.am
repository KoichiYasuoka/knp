SUBDIRS = .

EXTRA_DIST = auto.dat wikipedia.dat*

dicdir = $(pkgdatadir)/dict/auto

nodist_dic_DATA = auto.db

if NV_MI_EXIST
nodist_dic_DATA += nv_mi.db
endif

all: auto.db

auto.dat:
	true

auto.db: auto.dat wikipedia.dat*
	if [ -f wikipedia.dat.skip ]; then \
		if [ ! -f $@ ]; then \
			echo -e '\e[1;31m.skip exists. However $@ is not found\e[m' 1>&2; exit 5;\
		fi; \
		echo -e '\e[1;31mSkip build of $@ \e[m' 1>&2 :;\
	elif [ -f wikipedia.dat.xz ]; then \
		bash -c 'LANG=C sort auto.dat <(xzcat wikipedia.dat.xz) | @MAKE_DB_COMMAND@ $@ -append \|;' ; \
	elif [ -f wikipedia.dat ]; then \
		LANG=C sort auto.dat wikipedia.dat | @MAKE_DB_COMMAND@ $@ -append \|; \
	else \
		exit 2; \
	fi

nv_mi.db: nv_mi.dat
	if [ -f $< ]; then \
		@MAKE_DB_COMMAND@ $@ < $<; \
	fi

CLEANFILES = auto.db nv_mi.db
