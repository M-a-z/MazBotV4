#
#	Make file for MazBot networking
#
#	Revision:
#
#   -0.0.1 xx.07.2009/Maz  First Draft
#
#


COMPONENT_NAME := MazBot_networking
COMPONENT_HOME := $(BUILDROOT)/networking/
SOURCE_FILES := networking.c TCP.c

export INCLUDE := -I $(COMPONENT_HOME)$(SRC_FOLDER)
export INCLUDE += -I $(BUILDROOT)generic/$(SRC_FOLDER)

OBJECT_FILES := $(addsuffix .o, $(basename $(SOURCE_FILES)))

SRC_W_PATH := $(addprefix $(COMPONENT_HOME)$(SRC_FOLDER), $(SOURCE_FILES))
OBJ_W_PATH := $(addprefix $(COMPONENT_HOME)$(OBJ_FOLDER), $(OBJECT_FILES))

all: $(COMPONENT_HOME)$(LIB_FOLDER)lib$(COMPONENT_NAME).a

$(COMPONENT_HOME)$(LIB_FOLDER)lib$(COMPONENT_NAME).a:  $(OBJ_W_PATH)
	$(AR) $(ARFLAGS) $(COMPONENT_HOME)$(LIB_FOLDER)lib$(COMPONENT_NAME).a $(OBJ_W_PATH) $(ARLIBS)

$(COMPONENT_HOME)$(OBJ_FOLDER)%.o : $(COMPONENT_HOME)$(SRC_FOLDER)%.c
	@echo "Compiling: "$(COMPONENT_HOME)$(SRC_FOLDER)$<
	$(CC) $(CFLAGS) $@ $< $(INCLUDE)

clean$(COMPONENT_NAME):
	rm -rf $(COMPONENT_HOME)/make/core.*
	rm -rf $(COMPONENT_HOME)$(OBJ_FOLDER)*
	rm -rf $(COMPONENT_HOME)$(LIB_FOLDER)*
