#!/bin/bash

FONT_RED="\e[1;31m"
FONT_GREEN="\e[1;32m"
FONT_YELLOW="\e[1;33m"
FONT_BLUE="\e[1;34m"
FONT_END="\e[0m"


#wake-up
bg96_log=$(./motalk -d /dev/ttyUSB2 -b 9600 -c at)
#echo ${bg96_log}

#Request TA Model Identification
#1st time command is for wake-up BG96
#2nd time command is real command
for i in {1..2}
do
   bg96_log=$(./motalk -d /dev/ttyUSB2 -b 9600 -c at+gmm)
   sleep 1
done

bg96_log2=$(echo ${bg96_log} | awk -F "-" '{print $2}')

if [ ${bg96_log2} == "BG96" ]; then
	echo -e "\n"$FONT_GREEN"PASS!"$FONT_END"\n"
else
	echo -e "\n"$FONT_RED"FAULT!"$FONT_END"\n"
fi

echo -e "LTE CAT-M1 check test:" ${bg96_log2}
