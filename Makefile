#uncoment the next 2 lines to get LKL's file APIs. This currently works on linux only
.PHONY=clean
LKL_DEFINES+=-DLKL_FILE_APIS -Iinclude
LKL=$(CROSS)lkl/lkl.a

APR_LIN_CFLAGS=$(shell apr-config --includes --cppflags)
APR_WIN_CFLAGS=-Iapr_win/include/

APR_LIN_LIB=-lapr-1
APR_WIN_LIB=apr_win/Debug/libapr-1.lib

#LUA_LIN_CFLAGS=`lua-config --include`
#Ubuntu misses lua-config for lua5.1, thus the hardcoded path
LUA_LIN_CFLAGS=-I/usr/include/lua5.1
LUA_WIN_CFLAGS=TODO: add INCLUDE for Windows LUA

#LUA_LIN_LIB=`lua-config --libs`
#Ubuntu misses lua-config for lua5.1, thus the hardcoded path
LUA_LIN_LIB=/usr/lib/liblua5.1.a -lm
LUA_WIN_LIB=TODO: add LIB for Windows LUA

CFLAGS_OS=$(LUA_LIN_CFLAGS) $(APR_LIN_CFLAGS)
#CFLAGS_OS=$(LUA_WIN_CFLAGS) $(APR_WIN_CFLAGS)

HERE=$(PWD)
LINUX=$(HERE)/../linux-2.6

SRCS=$(shell ls *.c)
OBJS=$(patsubst %.c,%.o,$(SRCS))
DEPS=$(patsubst %.c,.deps/%.d,$(SRCS))

MKDIR=mkdir -p

all: daemon.out 

include/asm:
	-$(MKDIR) `dirname $@`
	ln -s $(LINUX)/include/asm-lkl include/asm

include/asm-i386:
	-$(MKDIR) `dirname $@`
	ln -s $(LINUX)/include/asm-i386 include/asm-i386

include/asm-generic:
	-$(MKDIR) `dirname $@`
	ln -s $(LINUX)/include/asm-generic include/asm-generic

include/linux:
	-$(MKDIR) `dirname $@`
	ln -s $(LINUX)/include/linux include/linux

INC=include/asm include/asm-generic include/asm-i386 include/linux

$(CROSS)lkl/.config: .config 
	mkdir -p $(CROSS)lkl && \
	cp $< $@

$(CROSS)lkl/lkl.a: $(CROSS)lkl/.config 
	cd $(LINUX) && \
	$(MAKE) O=$(HERE)/$(CROSS)lkl ARCH=lkl \
	CROSS_COMPILE=$(CROSS) \
	LKLENV_CFLAGS="$(CFLAGS_OS)" \
	lkl.a

CFLAGS=-Wall -g $(CFLAGS_OS) $(LKL_DEFINES)

%.o: %.c $(INC)
	$(CC) -c $(CFLAGS) $< 

AOUT=$(OBJS) $(LKL) 
AEXE=$(OBJS) $(LKL)

clean:
	-rm -rf daemon.out daemon.exe include *.o drivers/*.o drivers/built-in* drivers/.*.cmd .deps/ *~

clean-all: clean
	-rm -rf lkl lkl-nt

TAGS: 
	etags *.c drivers/*.c

daemon.out: $(AOUT) $(INC) 
	$(CC) $(AOUT) $(APR_LIN_LIB) $(LUA_LIN_LIB) -o $@

daemon.exe: CROSS=i586-mingw32msvc-

daemon.exe: $(AEXE) $(INC) 
	$(CROSS)gcc $(AEXE) $(APR_WIN_LIB) $(LUA_WIN_LIB) -o $@

.deps/%.d: %.c
	mkdir -p .deps/$(dir $<)
	$(CC) $(CFLAGS) -MM -MT $(patsubst %.c,%.o,$<) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

include $(DEPS)


