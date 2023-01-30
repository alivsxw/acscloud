THIS_MAKEFILE=$(abspath $(firstword $(MAKEFILE_LIST)))
SDK_PATH = $(dir $(THIS_MAKEFILE))

ifeq ($(chip),hi3516dv300)
export CROSS:=/opt/hisi-linux/x86-arm/arm-himix200-linux/bin/arm-himix200-linux-
export CC:=$(CROSS)gcc
export AR:=$(CROSS)ar
export TP:=$(CROSS)strip
export RANLIB:=$(CROSS)ranlib
LIB_PATH  = -L$(SDK_PATH)lib/$(platform)/$(chip)
INC_PATH  = -I$(SDK_PATH)include/$(platform)/$(chip)
INC_PATH += -I$(SDK_PATH)src
INC_PATH += -I$(SDK_PATH)include
INC_PATH += -I$(SDK_PATH)../dcit-lib/third_party/zlog/$(platform)/$(chip)/include
INC_PATH += -I$(SDK_PATH)../dcit-lib/third_party/curl/$(platform)/$(chip)/include
INC_PATH += -I$(SDK_PATH)../dcit-lib/third_party/mqtt/$(platform)/$(chip)/include
INC_PATH += -I$(SDK_PATH)../dcit-lib/third_party/ssl/$(platform)/$(chip)/include
INSTALL_PATH = $(SDK_PATH)../dcit-lib/$(module_name)/$(platform)/$(chip)
INSTALL_STATIC_PATH = $(SDK_PATH)../dcit-lib-static/$(module_name)/$(platform)/$(chip)
LIBS = 		
FLAGS  = -DHI3516DV300
#FLAGS += -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4 -fno-aggressive-loop-optimizations -ffunction-sections -fdata-sections -fstack-protector-strong -Wno-error=implicit-function-declaration -Werror=logical-not-parentheses
endif

ifeq ($(chip),nt98528)
export AR=/opt/arm-ca9-linux-gnueabihf-6.5/usr/bin/arm-ca9-linux-gnueabihf-ar
export ARCH=arm
export AS=/opt/arm-ca9-linux-gnueabihf-6.5/usr/bin/arm-ca9-linux-gnueabihf-as
export CC=/opt/arm-ca9-linux-gnueabihf-6.5/usr/bin/arm-ca9-linux-gnueabihf-g++
#export CROSS_COMPILE=/opt/arm-ca9-linux-gnueabihf-6.5/usr/bin/arm-ca9-linux-gnueabihf-
export CROSS_TOOLCHAIN_DIR=/opt
export CROSS_TOOLCHAIN_PATH=/opt/arm-ca9-linux-gnueabihf-6.5
export CXX=/opt/arm-ca9-linux-gnueabihf-6.5/usr/bin/arm-ca9-linux-gnueabihf-g++
export LD=/opt/arm-ca9-linux-gnueabihf-6.5/usr/bin/arm-ca9-linux-gnueabihf-ld
export LDD=/opt/arm-ca9-linux-gnueabihf-6.5/usr/bin/arm-ca9-linux-gnueabihf-ldd
export LD_LIBRARY_PATH=/opt/arm-ca9-linux-gnueabihf-6.5/usr/local/lib
export STRIP=/opt/arm-ca9-linux-gnueabihf-6.5/usr/bin/arm-ca9-linux-gnueabihf-strip
export SYSROOT_PATH=/opt/arm-ca9-linux-gnueabihf-6.5/usr/arm-ca9-linux-gnueabihf/sysroot
export PATH=/opt/arm-ca9-linux-gnueabihf-6.5/usr/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games
LIB_PATH  = -L$(SDK_PATH)lib/$(platform)/$(chip)
INC_PATH  = -I$(SDK_PATH)include/$(platform)/$(chip)
INC_PATH += -I$(SDK_PATH)src
INC_PATH += -I$(SDK_PATH)include
INC_PATH += -I$(SDK_PATH)../dcit-lib/third_party/zlog/$(platform)/$(chip)/include
INC_PATH += -I$(SDK_PATH)../dcit-lib/third_party/curl/$(platform)/$(chip)/include
INC_PATH += -I$(SDK_PATH)../dcit-lib/third_party/mqtt/$(platform)/$(chip)/include
INC_PATH += -I$(SDK_PATH)../dcit-lib/third_party/ssl/$(platform)/$(chip)/include
INSTALL_PATH = $(SDK_PATH)../dcit-lib/$(module_name)/$(platform)/$(chip)
INSTALL_STATIC_PATH = $(SDK_PATH)../dcit-lib-static/$(module_name)/$(platform)/$(chip)
LIBS = 	
FLAGS  = -DNT98528
FLAGS += -ffunction-sections -fdata-sections  -march=armv7-a -mtune=cortex-a9 -mfpu=neon -mfloat-abi=hard -ftree-vectorize -fno-builtin -fno-common -Wformat=1 -D_BSP_NA51055_  -D__LINUX520 -D__LINUX -D_NETWORK_ON_CPU1_ -Wall -Wundef -Wsign-compare -Wno-missing-braces -Werror=logical-not-parentheses
endif

ifeq ($(chip),nt98566)
export CROSS:=/opt/arm-ca9-linux-uclibcgnueabihf-8.4.01/usr/bin/arm-linux-
export CC:=$(CROSS)gcc
export AR:=$(CROSS)ar
export TP:=$(CROSS)strip
export RANLIB:=$(CROSS)ranlib
LIB_PATH  = -L$(SDK_PATH)lib/$(platform)/$(chip)
INC_PATH  = -I$(SDK_PATH)include/$(platform)/$(chip)
INC_PATH += -I$(SDK_PATH)src
INC_PATH += -I$(SDK_PATH)include
INC_PATH += -I$(SDK_PATH)../dcit-lib/third_party/zlog/$(platform)/$(chip)/include
INC_PATH += -I$(SDK_PATH)../dcit-lib/third_party/curl/$(platform)/$(chip)/include
INC_PATH += -I$(SDK_PATH)../dcit-lib/third_party/mqtt/$(platform)/$(chip)/include
INC_PATH += -I$(SDK_PATH)../dcit-lib/third_party/ssl/$(platform)/$(chip)/include
INSTALL_PATH = $(SDK_PATH)../dcit-lib/$(module_name)/$(platform)/$(chip)
INSTALL_STATIC_PATH = $(SDK_PATH)../dcit-lib-static/$(module_name)/$(platform)/$(chip)
LIBS = 	
FLAGS  = -DNT98566
FLAGS += -ffunction-sections -fdata-sections  -march=armv7-a -mtune=cortex-a9 -mfpu=neon -mfloat-abi=hard -ftree-vectorize -fno-builtin -fno-common -Wformat=1 -D_BSP_NA51055_  -D__LINUX520 -D__LINUX -D_NETWORK_ON_CPU1_ -Wall -Wundef -Wsign-compare -Wno-missing-braces -Werror=logical-not-parentheses
endif

ifeq ($(chip),nt98562)
export AR=/opt/arm-ca9-linux-uclibcgnueabihf-8.4.01/usr/bin/arm-linux-ar
export ARCH=arm
export AS=/opt/arm-ca9-linux-uclibcgnueabihf-8.4.01/usr/bin/arm-linux-as
export CC=/opt/arm-ca9-linux-uclibcgnueabihf-8.4.01/usr/bin/arm-linux-g++
#export CROSS_COMPILE="/opt/arm-ca9-linux-uclibcgnueabihf-8.4.01/usr/bin/arm-linux-"
export CROSS_TOOLCHAIN_DIR=/opt
export CROSS_TOOLCHAIN_PATH=/opt/arm-ca9-linux-uclibcgnueabihf-8.4.01
export CXX=/opt/arm-ca9-linux-uclibcgnueabihf-8.4.01/usr/bin/arm-linux-g++
export LD=/opt/arm-ca9-linux-uclibcgnueabihf-8.4.01/usr/bin/arm-linux-ld
export LDD=/opt/arm-ca9-linux-uclibcgnueabihf-8.4.01/usr/bin/arm-linux-ldd
export LD_LIBRARY_PATH=/opt/arm-ca9-linux-uclibcgnueabihf-8.4.01/usr/local/lib
export STRIP=/opt/arm-ca9-linux-uclibcgnueabihf-8.4.01/usr/bin/arm-linux-strip
export SYSROOT_PATH=/opt/arm-ca9-linux-uclibcgnueabihf-8.4.01/usr/arm-ca9-linux-uclibcgnueabihf/sysroot
export PATH=/opt/arm-ca9-linux-uclibcgnueabihf-8.4.01/usr/bin:/opt/arm/arm-ca9-linux-gnueabihf-6.5/usr/bin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games
export HOST=arm-linux
LIB_PATH  = -L$(SDK_PATH)lib/$(platform)/$(chip)
INC_PATH  = -I$(SDK_PATH)include/$(platform)/$(chip)
INC_PATH += -I$(SDK_PATH)src
INC_PATH += -I$(SDK_PATH)include
INC_PATH += -I$(SDK_PATH)../dcit-lib/third_party/zlog/$(platform)/$(chip)/include
INC_PATH += -I$(SDK_PATH)../dcit-lib/third_party/curl/$(platform)/$(chip)/include
INC_PATH += -I$(SDK_PATH)../dcit-lib/third_party/mqtt/$(platform)/$(chip)/include
INC_PATH += -I$(SDK_PATH)../dcit-lib/third_party/ssl/$(platform)/$(chip)/include
INSTALL_PATH = $(SDK_PATH)../dcit-lib/$(module_name)/$(platform)/$(chip)
INSTALL_STATIC_PATH = $(SDK_PATH)../dcit-lib-static/$(module_name)/$(platform)/$(chip)
LIBS = 	
FLAGS  = -DNT98562
FLAGS += -ffunction-sections -fdata-sections  -march=armv7-a -mtune=cortex-a9 -mfpu=neon -mfloat-abi=hard -ftree-vectorize -fno-builtin -fno-common -Wformat=1 -D_BSP_NA51055_  -D__LINUX520 -D__LINUX -D_NETWORK_ON_CPU1_ -Wall -Wundef -Wsign-compare -Wno-missing-braces -Werror=logical-not-parentheses
endif


ifeq ($(chip),ar9201)
export CROSS:=arm-linux-
export CC:=$(CROSS)gcc
export AR:=$(CROSS)ar
export TP:=$(CROSS)strip
export RANLIB:=$(CROSS)ranlib
LIB_PATH  = -L$(SDK_PATH)lib/$(platform)/$(chip)
INC_PATH  = -I$(SDK_PATH)include/$(platform)/$(chip)
INC_PATH += -I$(SDK_PATH)src
INC_PATH += -I$(SDK_PATH)include
INC_PATH += -I$(SDK_PATH)../dcit-lib/third_party/zlog/$(platform)/$(chip)/include
INC_PATH += -I$(SDK_PATH)../dcit-lib/third_party/curl/$(platform)/$(chip)/include
INC_PATH += -I$(SDK_PATH)../dcit-lib/third_party/mqtt/$(platform)/$(chip)/include
INC_PATH += -I$(SDK_PATH)../dcit-lib/third_party/ssl/$(platform)/$(chip)/include
INSTALL_PATH = $(SDK_PATH)../dcit-lib/$(module_name)/$(platform)/$(chip)
INSTALL_STATIC_PATH = $(SDK_PATH)../dcit-lib-static/$(module_name)/$(platform)/$(chip)
LIBS = 		
FLAGS  = -DAR9201
endif

DIR_SRC = ./src
DIR_OBJ = ./obj

CFLAGS := -g -Werror $(FLAGS) $(INC_PATH) $(LIB_PATH)

# Files to be compiled
SRC = $(wildcard ${DIR_SRC}/*.c)
OBJ = $(patsubst %.c,${DIR_OBJ}/%.o,$(notdir ${SRC})) 

# Don't remove *.o files automatically
.SECONDARY: $(OBJS)

DIR_BIN = .
TAR_GET_O = ${DIR_BIN}/lib$(module_name).so
TAR_GET_A = ${DIR_BIN}/lib$(module_name).a

all: $(TAR_GET_O) $(TAR_GET_A) $(TARGET)

${TAR_GET_A}:${OBJ}
	${AR} cru ${TAR_GET_A}	$(OBJ)
	${RANLIB} ${TAR_GET_A}

${TAR_GET_O}:${OBJ}
	${CC} -shared -o ${TAR_GET_O} $(OBJ)
	
${DIR_OBJ}/%.o:${DIR_SRC}/%.c
	$(CC) $(CFLAGS) $(LIBS) -c -fPIC $< -o $@
	
.PHONY: all clean install
clean:
#ifeq ($(TAR_GET_O), $(wildcard $(TAR_GET_O)))
	rm -rf ${DIR_OBJ}/*.o
	rm -rf $(TAR_GET_O)
	rm -rf $(INSTALL_PATH)/include/*
	rm -rf $(INSTALL_PATH)/lib/$(TAR_GET_O)
#endif
	rm -rf $(INSTALL_STATIC_PATH)/include/*
	rm -rf $(INSTALL_STATIC_PATH)/lib/$(TAR_GET_O)

DCITLIB_DIR_EXIST        := $(shell if [ -d $(SDK_PATH)../dcit-lib ]; then echo "exist"; else echo "noexist"; fi;)
INSTALL_DIR_EXIST        := $(shell if [ -d $(INSTALL_PATH)/include ] && [ -d $(INSTALL_PATH)/lib ]; then echo "exist"; else echo "noexist"; fi;)

DCITLIB_STATIC_DIR_EXIST := $(shell if [ -d $(SDK_PATH)../dcit-lib-static ]; then echo "exist"; else echo "noexist"; fi;)
INSTALL_STATIC_DIR_EXIST := $(shell if [ -d $(INSTALL_STATIC_PATH)/include ] && [ -d $(INSTALL_STATIC_PATH)/lib ]; then echo "exist"; else echo "noexist"; fi;)

install:
ifeq ($(DCITLIB_DIR_EXIST),noexist)
	@echo "*****************************************************************************************"
	@echo "There is no dcit-lib file"
	@echo "Please download from SVN https://desktop-2ji46sv/svn/dcit-lib/ put $(SDK_PATH)../dcit-lib"
	@echo "*****************************************************************************************"
	exit 1
endif

ifeq ($(DCITLIB_STATIC_DIR_EXIST),noexist)
	@echo "*****************************************************************************************"
	@echo "There is no dcit-lib file"
	@echo "Please download from SVN https://desktop-2ji46sv/svn/dcit-lib-static/ put $(SDK_PATH)../dcit-lib-static"
	@echo "*****************************************************************************************"
	exit 1
endif

ifeq ($(INSTALL_DIR_EXIST),noexist)
	mkdir $(INSTALL_PATH)/include -p
	mkdir $(INSTALL_PATH)/lib -p
endif

ifeq ($(INSTALL_STATIC_DIR_EXIST),noexist)
	mkdir $(INSTALL_STATIC_PATH)/include -p
	mkdir $(INSTALL_STATIC_PATH)/lib -p
endif
	sudo cp -rf $(TAR_GET_O) $(INSTALL_PATH)/lib
	sudo cp -rf include/acsclouddefine.h $(INSTALL_PATH)/include
	sudo cp -rf include/acscloudshare.h $(INSTALL_PATH)/include
	
	sudo cp -rf $(TAR_GET_A) $(INSTALL_STATIC_PATH)/lib
	sudo cp -rf include/acsclouddefine.h $(INSTALL_STATIC_PATH)/include
	sudo cp -rf include/acscloudshare.h $(INSTALL_STATIC_PATH)/include
