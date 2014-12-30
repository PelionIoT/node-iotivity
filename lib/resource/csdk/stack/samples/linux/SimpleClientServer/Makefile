# //******************************************************************
# //
# // Copyright 2014 Intel Mobile Communications GmbH All Rights Reserved.
# //
# //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
# //
# // Licensed under the Apache License, Version 2.0 (the "License");
# // you may not use this file except in compliance with the License.
# // You may obtain a copy of the License at
# //
# //      http://www.apache.org/licenses/LICENSE-2.0
# //
# // Unless required by applicable law or agreed to in writing, software
# // distributed under the License is distributed on an "AS IS" BASIS,
# // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# // See the License for the specific language governing permissions and
# // limitations under the License.
# //
# //-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#
# override with `make BUILD=debug`
# default to release build
BUILD            := release
PLATFORM         := linux
CC               := g++
OUT_DIR          := $(BUILD)
OBJ_DIR          := $(OUT_DIR)/obj

ifeq ($(ROOT_DIR), )
ROOT_DIR	 = ../../../../../csdk/
endif

#OUT_DIR	= $(PWD)

OCLOGGER_DIR	= $(ROOT_DIR)/logger
OCRANDOM_DIR	= $(ROOT_DIR)/ocrandom
OCSOCKET_DIR	= $(ROOT_DIR)/ocsocket
LCOAP_DIR		= $(ROOT_DIR)/libcoap-4.1.1
OCCOAP_DIR		= $(ROOT_DIR)/occoap
OCTBSTACK_DIR	= $(ROOT_DIR)/stack

OCLOGGER_INC	= $(OCLOGGER_DIR)/include
OCRANDOM_INC	= $(OCRANDOM_DIR)/include
OCSOCKET_INC	= $(OCSOCKET_DIR)/include
LCOAP_INC		= $(LCOAP_DIR)
OCCOAP_INC		= $(OCCOAP_DIR)/include
OCTBSTACK_INC	= $(OCTBSTACK_DIR)/include


EXTLIBS_DIR      = $(ROOT_DIR)/../../extlibs
CJSON_DIR        = $(EXTLIBS_DIR)/cjson
CJSON_SRC        = $(CJSON_DIR)
CJSON_INC        = $(CJSON_DIR)
CJSON_SOURCES    := $(CJSON_SRC)/cJSON.c


INC_DIRS	:= -I$(OCLOGGER_INC)
INC_DIRS	+= -I$(OCRANDOM_INC)
INC_DIRS	+= -I$(OCSOCKET_INC)
INC_DIRS	+= -I$(LCOAP_INC)
INC_DIRS	+= -I$(OCCOAP_INC)
INC_DIRS	+= -I$(OCTBSTACK_INC)
INC_DIRS         += -I$(CJSON_INC)

CC_FLAGS.debug      := -O0 -g3 -Wall -ffunction-sections -fdata-sections -fno-exceptions \
                        -std=c++0x -pedantic $(INC_DIRS) -L$(ROOT_DIR)/build/$(PLATFORM)/$(BUILD) -DTB_LOG
CC_FLAGS.release    := -Os -Wall -fdata-sections -Wl,--gc-sections -Wl,-s -fno-exceptions \
                        -std=c++0x $(INC_DIRS) -L$(ROOT_DIR)/build/$(PLATFORM)/$(BUILD) -DTB_LOG

LDLIBS		+= -loctbstack -lpthread
CPPFLAGS	+= $(CC_FLAGS.$(BUILD)) $(LDLIBS)



SOURCES          := $(CJSON_SOURCES)
SOURCES          += common.cpp ledDemoServer.cpp

OBJECTS:=        $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SOURCES))

PROGRAMS         += ledDemoServer


all:             c_sdk prep_dirs $(OBJECTS) $(PROGRAMS)

prep_dirs:
	        -mkdir -p $(OUT_DIR)
	        -mkdir -p $(OBJ_DIR)

c_sdk:
	        $(MAKE) -C $(ROOT_DIR)/build/linux "BUILD=$(BUILD)" "PLATFORM=$(PLATFORM)"

$(OBJ_DIR)/%.o: %.cpp
	        $(CC) -c $(CPPFLAGS) $< -o $@

ledDemoServer:  $(OBJ_DIR)/ledDemoServer.o $(OBJ_DIR)/common.o
	        $(CC) $^ $(CPPFLAGS) -o $(BUILD)/$@

occlient:  $(OBJ_DIR)/occlient.o $(OBJ_DIR)/common.o
	        $(CC) $^ $(CPPFLAGS) -o $(BUILD)/$@



	
.PHONY:         clean

clean:
	        -rm -rf release
	        -rm -rf debug
	        $(MAKE) -C $(ROOT_DIR) clean
	        $(MAKE) -C $(ROOT_DIR) deepclean
