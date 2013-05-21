#
#	Main make file for MazBot
#
#	Revision:
#
#   -0.0.1 xx.07.2009/Maz  First Draft
#
#


tmp := $(shell pwd)
export BUILDROOT := $(tmp)/../
export BUILDROOT := $(tmp)/../

COMPONENTS := generic networking config parsers callbackhandler usercallbacks user_storage startup upperlayer test

include $(BUILDROOT)/make/generic.mak

all:
	for comp in $(COMPONENTS); do \
			cd $(BUILDROOT)$(strip $$comp)/make && $(MAKE);\
	done

clean: 
	for comp in $(COMPONENTS); do \
			cd $(BUILDROOT)$(strip $$comp)/make && $(MAKE) cleanMazBot_$(strip $$comp);\
	done
