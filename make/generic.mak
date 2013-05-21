#
#	Make file for MazBot generic
#
#	Revision History:
#
#   -0.0.1  xx.07.2009/Maz  First Draft
#
#
export CC := gcc
export CFLAGS := -Wall -dH -ggdb -c -D_GNU_SOURCE -o  
export AR := ar
export ARFLAGS := -cr
export OBJ_FOLDER := obj/
export LIB_FOLDER := lib/
export SRC_FOLDER := src/

#export INCLUDE := -I ${BUILDROOT}/global_defs/
