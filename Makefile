COMPILER = gcc
CFLAGS = -g
INCLUDES = -I./kii_sources
INCLUDES += -I/usr/local/include
INCLUDES += -I/usr/local/Cellar/curl/7.38.0/include/curl/
LDFLAGS = -L/usr/local/Cellar/curl/7.38.0/lib
LDFLAGS += -L/usr/local/lib
LIBS = -l curl
LIBS += -l jansson
TARGETNAME = thingdemo

SOURCES = $(wildcard *.c)
SOURCES += $(wildcard kii_sources/*.c)

all: build

build: $(OBJECTS)
	${COMPILER} ${CFLAGS} ${INCLUDES} ${LDFLAGS} ${LIBS} ${SOURCES} -o ${TARGETNAME}

clean:
	rm -f ${TARGETNAME}
	rm -rf ${TARGETNAME}.dSYM

cleandat:
	rm *.dat

.PHONY: all build clean cleandat
