#
#	Make file for MazBot config
#
#	Revision:
#
#   -0.0.1 xx.07.2009/Maz  First Draft
#
#


COMPONENT_NAME := MazBot_config
COMPONENT_HOME := $(BUILDROOT)/config/
SOURCE_FILES   := config.c irc_config.c channel_config.c mbot_pseudo_xml.c server_config.c server_events.c 

#ARLIBS := $(BUILDROOT)/generic/$(LIB_FOLDER)libMazBot_generic.a
#INCLIBDIR := -L $(BUILDROOT)/generic/$(LIBS)

export INCLUDE := -I $(COMPONENT_HOME)$(SRC_FOLDER)
export INCLUDE += -I $(BUILDROOT)generic/$(SRC_FOLDER)
export INCLUDE += -I $(BUILDROOT)networking/$(SRC_FOLDER)
export INCLUDE += -I $(BUILDROOT)user_storage/$(SRC_FOLDER)
export INCLUDE += -I $(BUILDROOT)parsers/$(SRC_FOLDER)
export INCLUDE += -I $(BUILDROOT)callbackhandler/$(SRC_FOLDER)
OBJECT_FILES := $(addsuffix .o, $(basename $(SOURCE_FILES)))

SRC_W_PATH := $(addprefix $(COMPONENT_HOME)$(SRC_FOLDER), $(SOURCE_FILES))
OBJ_W_PATH := $(addprefix $(COMPONENT_HOME)$(OBJ_FOLDER), $(OBJECT_FILES))

all: $(COMPONENT_HOME)$(LIB_FOLDER)lib$(COMPONENT_NAME).a

$(COMPONENT_HOME)$(LIB_FOLDER)lib$(COMPONENT_NAME).a:  $(OBJ_W_PATH)
	$(AR) $(ARFLAGS) $(COMPONENT_HOME)$(LIB_FOLDER)lib$(COMPONENT_NAME).a $(OBJ_W_PATH) $(ARLIBS)

$(COMPONENT_HOME)$(OBJ_FOLDER)%.o : $(COMPONENT_HOME)$(SRC_FOLDER)%.c
	@echo "Compiling: "$(COMPONENT_HOME)$(SRC_FOLDER)$<
	$(CC) $(CFLAGS) $@ $< $(INCLUDE)

#	$(CC) $(CFLAGS) $(SOURCE_FILES) $(INCLUDE)
	#$(CC) $(CFLAGS) $(COMPONENT_HOME)$(OBJ_FOLDER)$@ $(COMPONENT_HOME)$(SRC_FOLDER)$< $(INCLUDE)

clean$(COMPONENT_NAME):
	rm -rf $(COMPONENT_HOME)/make/core.*
	rm -rf $(COMPONENT_HOME)$(OBJ_FOLDER)*
	rm -rf $(COMPONENT_HOME)$(LIB_FOLDER)*
