######################################
#
#	Script for compiling tests for  generic funcs
#
#  Lisence info: You're allowed to use / modify this - you're only required to
#  write me a short note and your opinion about this to Mazziesaccount@gmail.com.
#  if you redistribute this you're required to mention the original author:
#  "Maz - http://maz-programmersdiary.blogspot.com/" in your distribution
#  channel.
#
#
#  PasteLeft 2009 Maz.
#
######################################
#!/bin/bash
gcc -o testi testgen.c -I./ -L ../lib/ -lMazBot_generic
