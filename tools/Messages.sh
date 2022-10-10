#!/bin/sh
cd ..
dirs="app plugins/iso plugins/krarc"
$EXTRACTRC `find . -name \*.rc -o -name \*.ui` >> rc.cpp || exit 11
$EXTRACTRC --tag-group=none --tag=title --tag=category --tag=tooltip --tag=description app/useraction_examples.xml >> rc.cpp
$XGETTEXT rc.cpp `find $dirs -name \*.cpp -o -name \*.cc -o -name \*.c -o -name \*.h` -o $podir/krusader.pot
rm -f rc.cpp
