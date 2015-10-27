#############################################################################
# Makefile for building: gameproxy
#############################################################################

MAKEFILE      = Makefile

####### Compiler, tools and options

CC            = gcc
CXX           = g++
DEFINES       = 

ifeq ($(BIT),64)
BITS = -m64
else
BITS = -m32
endif

CFLAGS        = $(BITS) -pipe -O3 -Wall -W -fPIE $(DEFINES)
CXXFLAGS      = $(BITS) -pipe -O3 -Wall -W -fPIE $(DEFINES)
SRC_DIR       = ..
INCPATH       = -I$(SRC_DIR) -I$(SRC_DIR)/libs/parson -I.
LINK          = g++
LFLAGS        = $(BITS) -Wl,-O3
LIBS          = $(SUBLIBS) $(SRC_DIR)/libs/parson/parson.c -lev
AR            = ar cqs
RANLIB        = 
TAR           = tar -cf
COMPRESS      = gzip -9f
COPY          = cp -f
SED           = sed
COPY_FILE     = cp -f
COPY_DIR      = cp -f -R
STRIP         = strip
INSTALL_FILE  = install -m 644 -p
INSTALL_DIR   = $(COPY_DIR)
INSTALL_PROGRAM = install -m 755 -p
DEL_FILE      = rm -f
SYMLINK       = ln -f -s
DEL_DIR       = rmdir
MOVE          = mv -f
CHK_DIR_EXISTS= test -d
MKDIR         = mkdir -p

####### Output directory

OBJECTS_DIR   = ./

####### Files

SOURCES       = $(SRC_DIR)/main.cpp \
		$(SRC_DIR)/CProxySession.cpp \
		$(SRC_DIR)/CTcpSession.cpp \
		$(SRC_DIR)/CListenSocket.cpp \
		$(SRC_DIR)/CPool.cpp \
		$(SRC_DIR)/CSetting.cpp \
		$(SRC_DIR)/CBackend.cpp 
OBJECTS       = main.o \
		CProxySession.o \
		CTcpSession.o \
		CListenSocket.o \
		CPool.o \
		CSetting.o \
		CBackend.o

DESTDIR       = #avoid trailing-slash linebreak
TARGET        = gameproxy


first: all
####### Implicit rules

.SUFFIXES: .o .c .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"

####### Build rules

all: $(TARGET)

$(TARGET):  $(OBJECTS)  
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJCOMP) $(LIBS)

clean:compiler_clean 
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core


####### Sub-libraries

distclean: clean
	-$(DEL_FILE) $(TARGET) 
	-$(DEL_FILE) Makefile


check: first

compiler_yacc_decl_make_all:
compiler_yacc_decl_clean:
compiler_yacc_impl_make_all:
compiler_yacc_impl_clean:
compiler_lex_make_all:
compiler_lex_clean:
compiler_clean: 

####### Compile

main.o: $(SRC_DIR)/main.cpp $(SRC_DIR)/CProxySession.h \
		$(SRC_DIR)/CTcpSession.h \
		$(SRC_DIR)/gstypes.h \
		$(SRC_DIR)/CListenSocket.h \
		$(SRC_DIR)/CSetting.h \
		$(SRC_DIR)/libs/parson/parson.h \
		$(SRC_DIR)/CBackend.h \
		$(SRC_DIR)/CPool.h \
		$(SRC_DIR)/gslog.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o main.o $(SRC_DIR)/main.cpp

CProxySession.o: $(SRC_DIR)/CProxySession.cpp $(SRC_DIR)/CProxySession.h \
		$(SRC_DIR)/CTcpSession.h \
		$(SRC_DIR)/gstypes.h \
		$(SRC_DIR)/CListenSocket.h \
		$(SRC_DIR)/CPool.h \
		$(SRC_DIR)/gslog.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o CProxySession.o $(SRC_DIR)/CProxySession.cpp

CTcpSession.o: $(SRC_DIR)/CTcpSession.cpp $(SRC_DIR)/CTcpSession.h \
		$(SRC_DIR)/gstypes.h \
		$(SRC_DIR)/CProxySession.h \
		$(SRC_DIR)/CListenSocket.h \
		$(SRC_DIR)/gssocket.h \
		$(SRC_DIR)/gslog.h \
		$(SRC_DIR)/CPool.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o CTcpSession.o $(SRC_DIR)/CTcpSession.cpp

CListenSocket.o: $(SRC_DIR)/CListenSocket.cpp $(SRC_DIR)/CListenSocket.h \
		$(SRC_DIR)/gstypes.h \
		$(SRC_DIR)/CProxySession.h \
		$(SRC_DIR)/CTcpSession.h \
		$(SRC_DIR)/gssocket.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o CListenSocket.o $(SRC_DIR)/CListenSocket.cpp

CPool.o: $(SRC_DIR)/CPool.cpp $(SRC_DIR)/CPool.h \
		$(SRC_DIR)/gstypes.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o CPool.o $(SRC_DIR)/CPool.cpp

CSetting.o: $(SRC_DIR)/CSetting.cpp $(SRC_DIR)/CSetting.h \
		$(SRC_DIR)/libs/parson/parson.h \
		$(SRC_DIR)/gstypes.h \
		$(SRC_DIR)/gslog.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o CSetting.o $(SRC_DIR)/CSetting.cpp

CBackend.o: $(SRC_DIR)/CBackend.cpp $(SRC_DIR)/CBackend.h \
		$(SRC_DIR)/gstypes.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o CBackend.o $(SRC_DIR)/CBackend.cpp

FORCE:
