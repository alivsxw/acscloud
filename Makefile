# Makefile for ARM and PC
# Copyrith(C) 2013-2023 FX Inc.

all		: ALL
clean	: CLEAN
install : INSTALL

ALL:
	#make hi3516dv300;
	make nt98528;
	make nt98566;
	make nt98562;
	#make ar9201;
hi3516dv300:
	make -f Makefile.am module_name=acscloud platform=hisi chip=hi3516DV300 clean;
	make -f Makefile.am module_name=acscloud platform=hisi chip=hi3516dv300;
	make -f Makefile.am module_name=acscloud platform=hisi chip=hi3516dv300 install

nt98528:
	make -f Makefile.am module_name=acscloud platform=lianyong chip=nt98528 clean;
	make -f Makefile.am module_name=acscloud platform=lianyong chip=nt98528;
	make -f Makefile.am module_name=acscloud platform=lianyong chip=nt98528 install
	
nt98566:
	make -f Makefile.am module_name=acscloud platform=lianyong chip=nt98566 clean;
	make -f Makefile.am module_name=acscloud platform=lianyong chip=nt98566;
	make -f Makefile.am module_name=acscloud platform=lianyong chip=nt98566 install	

nt98562:
	make -f Makefile.am module_name=acscloud platform=lianyong chip=nt98562 clean;
	make -f Makefile.am module_name=acscloud platform=lianyong chip=nt98562;
	make -f Makefile.am module_name=acscloud platform=lianyong chip=nt98562 install

ar9201:
	make -f Makefile.am module_name=acscloud platform=kuxinwei chip=ar9201 clean;
	make -f Makefile.am module_name=acscloud platform=kuxinwei chip=ar9201;
	make -f Makefile.am module_name=acscloud platform=kuxinwei chip=ar9201 install
