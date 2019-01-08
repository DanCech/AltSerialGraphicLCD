# -*- Makefile -*-
#
# Get the platform name
#
UNAME:sh =uname | tr '[A-Z]' '[a-z]'
ARCH:sh =uname -p
#
# Find out the compiler.
#
WHICHCC:sh =which cc
