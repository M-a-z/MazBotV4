#
#	Make file for MazBot generic
#
#	Revision:
#
#   -0.0.1 xx.07.2009/Maz  First Draft
#
#


COMPONENT_NAME := MazBot_generic
COMPONENT_HOME := $(BUILDROOT)/generic/
SOURCE_FILES := generic.c helpers.c splitter.c MbotPackedArray.c MbotBitset.c MbotRingBuff.c irchelpers.c

OBJECT_FILES := $(addsuffix .o, $(basename $(SOURCE_FILES)))

SRC_W_PATH := $(addprefix $(COMPONENT_HOME)$(SRC_FOLDER), $(SOURCE_FILES))
OBJ_W_PATH := $(addprefix $(COMPONENT_HOME)$(OBJ_FOLDER), $(OBJECT_FILES))

export INCLUDE := -I $(COMPONENT_HOME)$(SRC_FOLDER)

all: $(COMPONENT_HOME)$(LIB_FOLDER)lib$(COMPONENT_NAME).a

$(COMPONENT_HOME)$(OBJ_FOLDER)%.o : $(COMPONENT_HOME)$(SRC_FOLDER)%.c
	@echo "Compiling: "$(COMPONENT_HOME)$(SRC_FOLDER)$<
	$(CC) $(CFLAGS) $@ $<

test:
	gcc -Wall -ggdb -std=c99 -o test ../src/irchelpers.c ../src/ircsendtest.c ../src/generic.c

clean$(COMPONENT_NAME):
	rm -rf $(COMPONENT_HOME)/make/core.*
	rm -rf $(COMPONENT_HOME)$(OBJ_FOLDER)*
	rm -rf $(COMPONENT_HOME)$(LIB_FOLDER)*

$(COMPONENT_HOME)$(LIB_FOLDER)lib$(COMPONENT_NAME).a: $(OBJ_W_PATH)
	$(AR) $(ARFLAGS) $(COMPONENT_HOME)$(LIB_FOLDER)lib$(COMPONENT_NAME).a $(OBJ_W_PATH) $(LIBS)
