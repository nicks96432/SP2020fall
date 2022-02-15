#! /bin/sh

time -p ./cracker "SPisGooood" "1234" 4 8 sample_output.txt
echo
time -p ./cracker "SPisBad" "5678" 4 8 sample_output.txt
echo
time -p ./cracker "SPPAssss" "1a2b" 4 8 sample_output.txt
echo
