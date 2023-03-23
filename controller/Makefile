include ../build-conf/vars.mk
include ../build-conf/rules.mk

TARGETS = ${UC_INCLUDE}/mModeDecoder.h ${UC_INCLUDE}/mOpDecoder.h ${UC_INCLUDE}/uROM-0.h ${UC_INCLUDE}/uROM-1.h ${UC_INCLUDE}/uROM-2.h ${UC_INCLUDE}/uROM-3.h ${UC_INCLUDE}/uROM-4.h 

all: all-roms

all-roms: ${UC_INCLUDE} $(TARGETS)

uc: 
	mkdir ${UC_INCLUDE}

