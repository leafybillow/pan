# Analyzer for "Pan" (Parity Analyzer)
# for Hall A Parity Experiments.
# R. Michaels, A. Vacheret, R. Holmes
#
# You must have the 'codaclass' library and point DCDIR to the
# directory where codaclass headers are found.  See README.
#
# ONLINE if compiling ET system.  Off-site users may comment this out.
# Environment $LD_LIBRARY_PATH must include $CODA/$OSNAME/lib
# export ONLINE = 1
#
# Uncomment this if profiling (note: it slows down the code)
# export PROFILE = 1
#
# To compile optimized
# export OPTIMIZE = 1
#
# Uncomment this if you want to use MySQL database
export MYSQLDB = 1
#
# Would uncommment this if you want rootcint Dictionary.
# Only works with the CVS version of root as of Dec 21, 2001.
# export MAKEDICTIONARY = 1

export OSNAME := $(shell uname)

ifeq ($(OSNAME),SunOS)

   ROOTCFLAGS    = $(shell root-config --cflags)
   ROOTLIBS      = $(shell root-config --libs)
   ROOTGLIBS     = $(shell root-config --glibs)
   CXX           = g++
   CXXFLAGS      = -KPIC -DSUNVERS -I$(ROOTSYS)/include -I$(MAINDIR)
   CXXFLAGS     += $(ROOTCFLAGS)
   LD            = g++
   LDFLAGS       = -g -D
   SOFLAGS       = -G
   LIBCODACLASS=libcoda.a
   GLIB =  -lm -lc -lgen -lw -lnsl -ldl
   SLIB = -L/opt/SUNWspro/SC4.2/lib -lF77 -lM77 -lsunmath
   ET_AC_FLAGS = -D_REENTRANT -D_POSIX_THREAD_SEMANTICS
   ET_CFLAGS = -mt -fast -xO5 -KPIC $(ET_AC_FLAGS) -DSUNVERS
# Typically CODA is an environment variable,e.g.
#   CODA = /adaqfs/coda/2.2
   LIBET = $(CODA)/SunOS/lib/libet.so
   ONLIBS = $(LIBET) -lposix4 -lnsl -lsocket -lresolv
   LIBS = $(GLIB)

endif

# Linux with egcs

ifeq ($(OSNAME),Linux)

   ROOTLIBS      = $(shell root-config --libs)
   ROOTGLIBS     = $(shell root-config --glibs)
   INCLUDES      = -I$(ROOTSYS)/include
   CXX           = g++
   CXXFLAGS      = -Wall -fno-exceptions -fPIC $(INCLUDES) 
   LD            = g++
   LDFLAGS       = 
   SOFLAGS       = -shared
   GLIBS         = $(ROOTGLIBS) -L/usr/X11R6/lib -lXpm -lX11
   LIBCODACLASS=libcoda.a
   ET_AC_FLAGS = -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS
   ET_CFLAGS = -02 -fPIC -I. $(ET_AC_FLAGS) -DLINUXVERS
# Typically CODA is an environment variable,e.g.
#   CODA = /adaqfs/coda/2.2
   LIBET = $(CODA)/Linux/lib/libet.so
   ONLIBS = $(LIBET) -lieee -lpthread -ldl -lresolv
   LIBS = $(GLIBS) $(ROOTLIBS) $(ROOTGLIBS)

endif

MAKEDEPEND    = g++

ALL_LIBS = $(LIBS) $(LIBCODACLASS)

DCDIR=../codaclass
INCLUDES += -I$(DCDIR)

ifdef ONLINE
    ALL_LIBS += $(ONLIBS)
endif

ifdef PROFILE
   CXXFLAGS += -pg
endif

ifdef OPTIMIZE
   CXXFLAGS += -O
else
   CXXFLAGS += -g
endif

#
# Mysql can be dowloaded from http://www.mysql.com 
# It is also found at CUE in /apps/mysql/
# Use Mysql version: mysql-3.23.xx 
# 
# libMySQL.so is root library found under $(ROOTSYS)/lib
#
# libz.so is a compression library used by libMySQL.so
# and can be found at CUE in /apps/lib/
#

ifdef MYSQLDB
MYSQL_INCLUDE = /apps/mysql/PRO/include/mysql
INCLUDES      +=-I$(MYSQL_INCLUDE) 
MYSQL_LIBS    = $(ROOTSYS)/lib/libMySQL.so /apps/lib/libz.so
ALL_LIBS += $(MYSQL_LIBS)
endif

# Grow the list of sources here.

SRC = TaAnalysisManager.cc  \
      TaBeamAna.cc TaFdbkAna.cc TaDevice.cc \
      TaCutInterval.cc TaCutList.cc TaEvent.cc TaLabelledQuantity.cc \
      TaPairFromPair.cc TaRun.cc TaStatistics.cc TaString.cc \
      VaAnalysis.cc VaDataBase.cc VaPair.cc TaADCCalib.cc

ifdef MYSQLDB
 SRC +=TaMysql.cc
 CXXFLAGS += -DMYSQLDB
else
 SRC +=TaAsciiDB.cc
endif

DEPS = $(SRC:.cc=.d)
OBJS = $(SRC:.cc=.o)
DEP  = $(SRC:.cc=.d)
HEAD = $(SRC:.cc=.hh)
HEAD += PanTypes.hh

ifdef ONLINE
  SRC += THaEtClient.cc
  CXXFLAGS += -DONLINE
endif




PROGS = pan

# Make the dictionary
ifdef MAKEDICTIONARY
  CXXFLAGS += -DDICT
  OBJS += TaDict.o
endif

install: all
	mv pan ../

all: $(PROGS)

$(PROGS): main.o $(OBJS) $(SRC) $(HEAD) 
	rm -f $@
	$(CXX) $(CXXFLAGS) -o $@ main.o $(OBJS) $(ALL_LIBS)

main.o: main.cc
	g++ -c $(INCLUDES) $<	


# Dictionary

TaDict.cc: $(OBJS) TaLinkDef.hh
	@echo "Generating Decoder Dictionary..."
	$(ROOTSYS)/bin/rootcint -f TaDict.cc -c -p -DDICT $(HEAD) TaLinkDef.hh

# create a tar file of ./$(VERS)/* (all code)

tarfile: clean version
	tar cvf $(VERS).tar ./$(VERS)

clean:
	rm -f *.o core *~ *.d TaDict* *.tar $(PROGS)

realclean:  clean
	rm -f *.d


.SUFFIXES:
.SUFFIXES: .c .cc .cpp .C .o .d

%.o:	%.cc
	$(CXX) $(CXXFLAGS) -c $<

%.d:	%.cc
	@echo Creating dependencies for $<
	@$(SHELL) -ec '$(MAKEDEPEND) -MM $(INCLUDES) -c $< \
                | sed '\''s%^.*\.o%$*\.o%g'\'' \
                | sed '\''s%\($*\)\.o[ :]*%\1.o $@ : %g'\'' > $@; \
                [ -s $@ ] || rm -f $@'

-include $(DEPS)
