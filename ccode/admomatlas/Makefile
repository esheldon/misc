default: 	all

clean: 
	@ cd phadmom; ${MAKE} $@
	@ cd atlas; ${MAKE} $@
	@ cd app; ${MAKE} $@
	@ cd idl; ${MAKE} $@

all:
	@ cd phadmom; ${MAKE} $@
	@ cd atlas; ${MAKE} $@
	@ cd app; ${MAKE} $@
	@ cd idl; ${MAKE} $@

irix :
	@ cd atlas; cp Makefile.irix Makefile
	@ cd idl; cp Makefile.irix Makefile
	@ cd phadmom; cp Makefile.irix Makefile
	@ cd app; cp Makefile.irix Makefile

osf:
	@ cd atlas; cp Makefile.osf Makefile
	@ cd idl; cp Makefile.osf Makefile
	@ cd phadmom; cp Makefile.osf Makefile
	@ cd app; cp Makefile.osf Makefile

linux:
	@ cd atlas; cp Makefile.linux Makefile
	@ cd idl; cp Makefile.linux Makefile
	@ cd phadmom; cp Makefile.linux Makefile
	@ cd app; cp Makefile.linux Makefile
