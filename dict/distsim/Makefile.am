SUBDIRS = .

EXTRA_DIST = mi.dat* sim.dat*

dicdir = $(pkgdatadir)/dict/distsim

if DISTSIM_MI_EXIST
nodist_dic_DATA = mi.db sim.db
dist_dic_DATA = wordlist.txt
else
nodist_dic_DATA = 
endif

mi.db: mi.dat*
	if [ -f mi.dat.skip ]; then \
		if [ ! -f $@ ]; then \
			echo -e '\e[1;31m.skip exists. However $@ is not found\e[m' 1>&2; exit 5;\
		fi; \
		echo -e '\e[1;31mSkip build of $@ \e[m' 1>&2 :;\
	elif [ -f mi.dat.xz ]; then \
		xzcat mi.dat.xz | @MAKE_DB_COMMAND@ $@ -z; \
	elif [ -f mi.dat ]; then \
		@MAKE_DB_COMMAND@ $@ -z < mi.dat; \
	else \
		exit 2;\
	fi;

sim.db: sim.dat*
	if [ -f sim.dat.skip ]; then \
		if [ ! -f $@ ]; then \
			echo -e '\e[1;31m.skip exists. However $@ is not found\e[m' 1>&2; exit 5;\
		fi; \
		echo -e '\e[1;31mSkip build of $@ \e[m' 1>&2 :;\
	elif [ -f sim.dat.xz ]; then \
		xzcat sim.dat.xz | @MAKE_DB_COMMAND@ $@; \
	elif [ -f sim.dat ]; then \
		@MAKE_DB_COMMAND@ $@ < sim.dat; \
	else \
		exit 2;\
	fi;
CLEANFILES = mi.db sim.db
