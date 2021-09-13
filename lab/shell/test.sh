#! /bin/bash

for file in $(ls trace*)
do
    ./sdriver.pl -t $file -s ./tshref > tshref_$file
    ./sdriver.pl -t $file -s ./tsh > tsh_$file
done

python3 diff.py
