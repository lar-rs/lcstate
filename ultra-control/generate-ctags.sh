#!/bin/bash

# ./ctags_with_dep.sh file1.c file2.c ... to generate a tags file for these files.

tags_source=`find src -type f -name '*.c' -print`

gcc -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/ultimate-1.0 -I/usr/include/mktbus-1.0 -I/usr/include/mktlib-2.0 -M $tags_source | sed -e 's/[\\ ]/\n/g' | \
       sed -e '/^$/d' -e '/\.o:[ \t]*$/d' | \
        ctags -L - --c-kinds=+lpx-dgcv --langmap=c:.c.x
cp tags .tags

