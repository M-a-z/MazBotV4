#
#	Make file for MazBot releases v0.2 ->
#
#	Revision:
#
#   - 0.0.1 20.08.2009/Maz  First Draft
#
#

BOT_VERSION := 0.2
COMPONENT_NAME := MazBot_da_bot_client
COMPONENT_HOME := $(BUILDROOT)/da_bot_client/
SOURCE_FILES := botV02.c

INCLIBS := -lMazBot_networking -lMazBot_startup -lMazBot_parsers -lMazBot_config -lMazBot_callbackhandler -lMazBot_usercallbacks -lMazBot_generic -pthread
INCLIBDIR := -L$(BUILDROOT)/generic/$(LIB_FOLDER) -L$(BUILDROOT)/networking/$(LIB_FOLDER) -L$(BUILDROOT)/parsers/$(LIB_FOLDER) -L$(BUILDROOT)/config/$(LIB_FOLDER) -L$(BUILDROOT)/startup/$(LIB_FOLDER) -L$(BUILDROOT)/callbackhandler/$(LIB_FOLDER) -L$(BUILDROOT)/usercallbacks/$(LIB_FOLDER)

export INCLUDE := -I $(COMPONENT_HOME)$(SRC_FOLDER)
export INCLUDE += -I $(BUILDROOT)generic/$(SRC_FOLDER)
export INCLUDE += -I $(BUILDROOT)networking/$(SRC_FOLDER)
export INCLUDE += -I $(BUILDROOT)
export INCLUDE += -I $(BUILDROOT)parsers/$(SRC_FOLDER)
export INCLUDE += -I $(BUILDROOT)callbackhandler/$(SRC_FOLDER)
export INCLUDE += -I $(BUILDROOT)startup/$(SRC_FOLDER)

OBJECT_FILES := $(addsuffix .o, $(basename $(SOURCE_FILES)))

SRC_W_PATH := $(addprefix $(COMPONENT_HOME)$(SRC_FOLDER), $(SOURCE_FILES))
OBJ_W_PATH := $(addprefix $(COMPONENT_HOME)$(OBJ_FOLDER), $(OBJECT_FILES))

all: $(COMPONENT_HOME)/exe/TestExe 

$(COMPONENT_HOME)/exe/TestExe:
	$(CC) -Wall -ggdb -o $(COMPONENT_HOME)/exe/MazBot_$(BOT_VERSION) $(SRC_W_PATH) $(INCLUDE) $(INCLIBDIR) $(INCLIBS)

#$(COMPONENT_HOME)$(LIB_FOLDER)lib$(COMPONENT_NAME).a:  $(OBJ_W_PATH)
#	$(AR) $(ARFLAGS) $(COMPONENT_HOME)$(LIB_FOLDER)lib$(COMPONENT_NAME).a $(OBJ_W_PATH) $(ARLIBS)

#$(COMPONENT_HOME)$(OBJ_FOLDER)%.o : $(COMPONENT_HOME)$(SRC_FOLDER)%.c
#	@echo "Compiling: "$(COMPONENT_HOME)$(SRC_FOLDER)$<
#	$(CC) $(CFLAGS) $@ $< $(INCLUDE)

#	$(CC) $(CFLAGS) $(SOURCE_FILES) $(INCLUDE)
	#$(CC) $(CFLAGS) $(COMPONENT_HOME)$(OBJ_FOLDER)$@ $(COMPONENT_HOME)$(SRC_FOLDER)$< $(INCLUDE)

clean$(COMPONENT_NAME):
	rm -rf $(COMPONENT_HOME)/exe/MazBot_$(BOT_VERSION)
	rm -rf $(COMPONENT_HOME)/make/core.*
#	rm -rf $(COMPONENT_HOME)$(LIB_FOLDER)*
