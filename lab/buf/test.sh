# !/bin/bash
./hex2raw < candle | ./bufbomb -u BlockChanZJ 
echo ""
./hex2raw < sparkler | ./bufbomb -u BlockChanZJ 
echo ""
./hex2raw < firecracker | ./bufbomb -u BlockChanZJ 
echo ""
./hex2raw < dynamite | ./bufbomb -u BlockChanZJ 
echo ""
./hex2raw -n < nitroglycerin | ./bufbomb -u BlockChanZJ -n
echo ""
#./hex2raw -n < nitroglycerin-bad | ./bufbomb -u BlockChanZJ  -n
