#This makefile makes all the main book code with CppUTest test harness

#Set this to @ to keep the makefile quiet
SILENCE = @

#---- Outputs ----#
COMPONENT_NAME = tempodb

#--- Inputs ----#
CPPUTEST_HOME = CppUTest
CPP_PLATFORM = Gcc
PROJECT_HOME_DIR = .

SRC_DIRS = \
	src/tempodb\
	src/tempodb/platform

TEST_SRC_DIRS = \
	.\
	mocks\
	tests/TempoDb\
	tests\


INCLUDE_DIRS =\
  .\
  $(CPPUTEST_HOME)/include\
  mocks\
	include/tempodb

MOCKS_SRC_DIRS = \
	mocks\

CPPUTEST_WARNINGFLAGS = -Wall -Wswitch-default -Werror
#CPPUTEST_CFLAGS = -std=c89
CPPUTEST_CFLAGS += -Wall -Wstrict-prototypes -pedantic
LD_LIBRARIES = -lpthread

STUFF_TO_CLEAN += objs/*.o objs/platform/*.o objs/src/tempodb/* objs/tests/* objs/mocks/* objs/tests/tempodb/* lib/libtempodb*.a

CC = cc

all: posix

posix: lib/libtempodb_posix.a

lib/libtempodb_posix.a: objs/base64.o objs/tempodb.o objs/platform/posix.o
	ar rv lib/libtempodb_posix.a objs/base64.o objs/tempodb.o objs/platform/posix.o

objs/base64.o:
	$(CC) -c -I include/tempodb -o objs/base64.o src/tempodb/base64.c

objs/tempodb.o:
	$(CC) -c -I include/tempodb -o objs/tempodb.o src/tempodb/tempodb.c

objs/platform/posix.o:
	$(CC) -c -I include/tempodb -o objs/platform/posix.o src/tempodb/platform/posix.c

CppUTest/lib/libCppUTest.a:
	cd CppUTest && make lib/libCppUTest.a

include $(CPPUTEST_HOME)/build/MakefileWorker.mk
