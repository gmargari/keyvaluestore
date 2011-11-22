#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Syntax: $0 <stats folder>"
    exit 1
fi

#============================
# my_print()
#============================
scriptname=`basename $0`
my_print() {
    echo -e "\e[1;31m [ $scriptname ] $* \e[m"
}

statsfolder=$1
curdir=`pwd`
texfile="keyvaluestore" # without the .tex extension

cp ${texfile}.tex ${statsfolder}/ &&
cd ${statsfolder} &&

my_print "latex ${texfile}.tex"
latex ${texfile}.tex && rm ${texfile}.tex &&

my_print "dvips ${texfile}.dvi"
dvips ${texfile}.dvi && rm ${texfile}.dvi &&

my_print "ps2pdf ${texfile}.ps"
ps2pdf ${texfile}.ps && rm ${texfile}.ps &&

rm -f ${texfile}.aux ${texfile}.log && 
mv ${texfile}.pdf ${curdir} &&
my_print "${texfile}.pdf was created successfully! \n" &&
exit 0

echo "Error creating pdf!" >&2
exit 1
