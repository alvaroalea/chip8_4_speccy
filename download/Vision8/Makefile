default:
	@echo "To compile Vision-8, use one of the following:"
	@echo " make x     - Make the Unix/X version"
	@echo " make msdos - Make the MS-DOS version"
	@echo " make msx   - Make the MSX version"
	@echo " make adam  - Make the Coleco ADAM version"
	@echo ""
	@echo "The MS-DOS makefile is configured for Borland C++"
	@echo "The MSX and Coleco ADAM versions are configured for Hi-Tech C 3.09"

x:
	make -f Makefile.X

msdos:
	make -f Makefile.DOS

msx:
	make -f Makefile.MSX

adam:
	make -f Makefile.ADM

clean:
	rm -f *.o *~
