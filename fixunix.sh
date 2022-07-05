#!/bin/sh
#
#  Sets up gstream for building under Unix, converting text
#  files from CR/LF to LF format.

echo "Converting files to Unix format..."

find . -type f "(" -not -name font.dat -not -iname "*.zip" -not -perm +111 ")" \
   -exec sh -c "echo {};
		mv {} _tmpfile;
		tr -d \\\r < _tmpfile > {};
		touch -r _tmpfile {}; 
		rm _tmpfile" \;

cp Makefile.unx Makefile
echo "Done!"