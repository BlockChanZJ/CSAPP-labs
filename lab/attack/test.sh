# !/bin/bash
./hex2raw < ctarget.touch1| ./ctarget -q 
./hex2raw < ctarget.touch2| ./ctarget -q 
./hex2raw < ctarget.touch3| ./ctarget -q 
./hex2raw < rtarget.touch2| ./rtarget -q
./hex2raw < rtarget.touch3| ./rtarget -q
