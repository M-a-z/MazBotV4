#
#	Make file for MazBot generic
#
#	Revision:
#
#   -0.0.1 25.11.2009/Maz  First Draft
#
#

COMPONENT_SHORT := upperlayer
COMPONENT_NAME := MazBot_$(COMPONENT_SHORT)
COMPONENT_HOME := $(BUILDROOT)/$(COMPONENT_SHORT)/
SOURCE_FILES := irc_server_abstraction.c irc_channel_abstraction.c

OBJECT_FILES := $(addsuffix .o, $(basename $(SOURCE_FILES)))

SRC_W_PATH := $(addprefix $(COMPONENT_HOME)$(SRC_FOLDER), $(SOURCE_FILES))
OBJ_W_PATH := $(addprefix $(COMPONENT_HOME)$(OBJ_FOLDER), $(OBJECT_FILES))


export INCLUDE := -I $(COMPONENT_HOME)$(SRC_FOLDER)
export INCLUDE += -I $(BUILDROOT)parsers/$(SRC_FOLDER)
export INCLUDE += -I $(BUILDROOT)networking/$(SRC_FOLDER)
export INCLUDE += -I $(BUILDROOT)generic/$(SRC_FOLDER)
export INCLUDE += -I $(BUILDROOT)config/$(SRC_FOLDER)
export INCLUDE += -I $(BUILDROOT)callbackhandler/$(SRC_FOLDER)
export INCLUDE += -I $(BUILDROOT)user_storage/$(SRC_FOLDER)

all: $(COMPONENT_HOME)$(LIB_FOLDER)lib$(COMPONENT_NAME).a

$(COMPONENT_HOME)$(OBJ_FOLDER)%.o : $(COMPONENT_HOME)$(SRC_FOLDER)%.c
	@echo "Compiling: "$(COMPONENT_HOME)$(SRC_FOLDER)$<
	$(CC) $(CFLAGS) $@ $< $(INCLUDE)

clean$(COMPONENT_NAME):
	rm -rf $(COMPONENT_HOME)/make/core.*
	rm -rf $(COMPONENT_HOME)$(OBJ_FOLDER)*
	rm -rf $(COMPONENT_HOME)$(LIB_FOLDER)*

$(COMPONENT_HOME)$(LIB_FOLDER)lib$(COMPONENT_NAME).a: $(OBJ_W_PATH)
	$(AR) $(ARFLAGS) $(COMPONENT_HOME)$(LIB_FOLDER)lib$(COMPONENT_NAME).a $(OBJ_W_PATH) $(LIBS)
