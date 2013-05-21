#
#	Make file for MazBot tests/release v0.1
#
#	Revision:
#
#   - 0.0.2 10.08.2009/Maz  Added trimmertest. 
#   						(And actually whole bunch of text since previous revision comment :( )
#   - 0.0.1 xx.07.2009/Maz  First Draft
#
#


COMPONENT_NAME := MazBot_test
COMPONENT_HOME := $(BUILDROOT)/test/
#SOURCE_FILES := main.c
#SOURCE_FILES := helpers_test.c 
#SOURCE_FILES := parsertest.c
#SOURCE_FILES := dicebot.c
#SOURCE_FILES := cexplodetest.c
#SOURCE_FILES := splittertest.c
#SOURCE_FILES := testfileconfparser.c
#SOURCE_FILES := trimtest.c
#SOURCE_FILES := botV02.c
SOURCE_FILES := irc_cfg_test.c

INCLIBS := -lMazBot_startup -lMazBot_upperlayer -lMazBot_user_storage -lMazBot_networking -lMazBot_parsers -lMazBot_config -lMazBot_usercallbacks -lMazBot_callbackhandler -lMazBot_generic -pthread
INCLIBDIR := -L$(BUILDROOT)/generic/$(LIB_FOLDER) -L$(BUILDROOT)/networking/$(LIB_FOLDER) -L$(BUILDROOT)/parsers/$(LIB_FOLDER) -L$(BUILDROOT)/config/$(LIB_FOLDER) -L$(BUILDROOT)/startup/$(LIB_FOLDER) -L$(BUILDROOT)/callbackhandler/$(LIB_FOLDER) -L$(BUILDROOT)/usercallbacks/$(LIB_FOLDER) -L$(BUILDROOT)/user_storage/$(LIB_FOLDER) -L$(BUILDROOT)/upperlayer/$(LIB_FOLDER)

export INCLUDE := -I $(COMPONENT_HOME)$(SRC_FOLDER)
export INCLUDE += -I $(BUILDROOT)generic/$(SRC_FOLDER)
export INCLUDE += -I $(BUILDROOT)networking/$(SRC_FOLDER)
export INCLUDE += -I $(BUILDROOT)
export INCLUDE += -I $(BUILDROOT)config/$(SRC_FOLDER)
export INCLUDE += -I $(BUILDROOT)parsers/$(SRC_FOLDER)
export INCLUDE += -I $(BUILDROOT)callbackhandler/$(SRC_FOLDER)
export INCLUDE += -I $(BUILDROOT)startup/$(SRC_FOLDER)
export INCLUDE += -I $(BUILDROOT)user_storage/$(SRC_FOLDER)
export INCLUDE += -I $(BUILDROOT)upperlayer/$(SRC_FOLDER)

OBJECT_FILES := $(addsuffix .o, $(basename $(SOURCE_FILES)))

SRC_W_PATH := $(addprefix $(COMPONENT_HOME)$(SRC_FOLDER), $(SOURCE_FILES))
OBJ_W_PATH := $(addprefix $(COMPONENT_HOME)$(OBJ_FOLDER), $(OBJECT_FILES))

all: $(COMPONENT_HOME)/exe/TestExe 

$(COMPONENT_HOME)/exe/TestExe:
	$(CC) -Wall -ggdb -dH -o $(COMPONENT_HOME)/exe/TestExe $(SRC_W_PATH) $(INCLUDE) $(INCLIBDIR) $(INCLIBS)

#$(COMPONENT_HOME)$(LIB_FOLDER)lib$(COMPONENT_NAME).a:  $(OBJ_W_PATH)
#	$(AR) $(ARFLAGS) $(COMPONENT_HOME)$(LIB_FOLDER)lib$(COMPONENT_NAME).a $(OBJ_W_PATH) $(ARLIBS)

#$(COMPONENT_HOME)$(OBJ_FOLDER)%.o : $(COMPONENT_HOME)$(SRC_FOLDER)%.c
#	@echo "Compiling: "$(COMPONENT_HOME)$(SRC_FOLDER)$<
#	$(CC) $(CFLAGS) $@ $< $(INCLUDE)

#	$(CC) $(CFLAGS) $(SOURCE_FILES) $(INCLUDE)
	#$(CC) $(CFLAGS) $(COMPONENT_HOME)$(OBJ_FOLDER)$@ $(COMPONENT_HOME)$(SRC_FOLDER)$< $(INCLUDE)

clean$(COMPONENT_NAME):
	rm -rf $(COMPONENT_HOME)/exe/TestExe
	rm -rf $(COMPONENT_HOME)/exe/core.*
	rm -rf $(COMPONENT_HOME)/make/core.*
#	rm -rf $(COMPONENT_HOME)$(LIB_FOLDER)*
