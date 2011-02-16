FC := gfortran
FFLAGS = -fimplicit-none 

test_executables = test-healpix test-hist test-sort

test: $(test_executables)

test_healpix_mods = healpix.f90 errlib.f90 arrlib.f90 sortlib.f90 histogram.f90
test_hist_mods = errlib.f90 arrlib.f90 sortlib.f90 histogram.f90
test_sort_mods = arrlib.f90 sortlib.f90

clean:
	rm *.mod
	rm $(test_executables)

test-healpix:  $(test_healpix_mods) test-healpix.f90 
	$(FC) $(FFLAGS) $(test_healpix_mods) test-healpix.f90 -o test-healpix

test-hist:  $(test_hist_mods) test-hist.f90 
	$(FC) $(FFLAGS) $(test_hist_mods) test-hist.f90 -o test-hist

test-sort:  $(test_sort_mods) test-sort.f90 
	$(FC) $(FFLAGS) $(test_sort_mods) test-sort.f90 -o test-sort
