#!/bin/bash
echo "occur reason:address not mapped to object" > crash1.log
echo "occur position:" > crash2.log
addr2line -f -e $1 555 > crash3.log
addr2line -f -e $1 18b4 197c 1f11 25a4 1fc50 555 58b 5eaa18 5e4a03 96f4f 28fb23 287cc9 5b0354 5d6ba2 25a097 26061d 287cac 5b1435 5d6da2 25a097  > crash4.log
cat *.log > merge.log
rm -f crash*.log
