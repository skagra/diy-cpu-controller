include ../build-conf/vars.mk
include ../build-conf/rules.mk

PORT=COM6
BOARD=arduino:avr:nano:cpu=atmega328old

TARGETS = ${UC_INCLUDE}/mModeDecoder.h ${UC_INCLUDE}/mOpDecoder.h ${UC_INCLUDE}/uROM-0.h ${UC_INCLUDE}/uROM-1.h ${UC_INCLUDE}/uROM-2.h ${UC_INCLUDE}/uROM-3.h ${UC_INCLUDE}/uROM-4.h 

all: all-roms compile 

all-roms: ${UC_INCLUDE} $(TARGETS)

uc: 
	mkdir ${UC_INCLUDE}

compile:
	arduino-cli compile --fqbn ${BOARD} --output-dir ${BUILD_DIR}

upload: compile
	arduino-cli upload --input-dir ${BUILD_DIR} -p ${PORT} --fqbn ${BOARD}

clean:
	rmdir /Q /S ${BUILD_DIR}
	rmdir /Q /S ${UC_INCLUDE}

