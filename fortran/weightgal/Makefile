FORTRAN = g77 
FFLAGS  = -O3 


OBJFIN =  weightgal.o


weightgal: ${OBJFIN}
	$(FORTRAN) $(FFLAGS) -o $@ ${OBJFIN}
weightgal.o: weightgal.f 
	$(FORTRAN) $(FFLAGS) -c weightgal.f

clean:
	rm -f *.o
