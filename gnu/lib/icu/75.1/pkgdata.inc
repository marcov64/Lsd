GENCCODE_ASSEMBLY_TYPE=-a gcc-mingw64
SO=dll
SOBJ=dll
A=a
LIBPREFIX=lib
LIB_EXT_ORDER=75.dll
COMPILE=gcc -D__USE_MINGW_ANSI_STDIO=1   -DHAVE_DLOPEN=0 -DU_HAVE_MMAP=0 -DU_HAVE_STRTOD_L=0  -DWINVER=0x0601 -D_WIN32_WINNT=0x0601 -DU_ALL_IMPLEMENTATION -DU_ATTRIBUTE_DEPRECATED= -march=nocona -msahf -mtune=generic -O2 -pipe -Wp,-D_FORTIFY_SOURCE=2 -fstack-protector-strong -std=c11 -Wall -pedantic -Wshadow -Wpointer-arith -Wmissing-prototypes -Wwrite-strings -mthreads  -c
LIBFLAGS=-I/mingw64/include -DPIC 
GENLIB=gcc -march=nocona -msahf -mtune=generic -O2 -pipe -Wp,-D_FORTIFY_SOURCE=2 -fstack-protector-strong -std=c11 -Wall -pedantic -Wshadow -Wpointer-arith -Wmissing-prototypes -Wwrite-strings -mthreads   -shared  -Wl,--enable-auto-import -Wl,--out-implib=./all.dll.a
LDICUDTFLAGS=
LD_SONAME=
RPATH_FLAGS=
BIR_LDFLAGS=
AR=ar
ARFLAGS=r
RANLIB=ranlib
INSTALL_CMD=/usr/bin/install -c
