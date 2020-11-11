AT&T汇编指令手册
https://www.cnblogs.com/jokerjason/p/9578646.html

|指令名|含义|
|--|--|
|jnz| 非负数则跳转。 跳转条件 ~SF
|testb S2,S1| S1 & S2 测试字节，与关系|
|cmpb S2,S1| S1 – S2 比较字节，差关系|
|movb S,D| D <-- S  传字节|
|inb|从I/O端口读取一个字节|
|outb|向I/O端口写入一个字节|
|LGDT|加载全局描述符|


ESP: 栈指针寄存器(extended stack pointer)，其内存放着一个指针，该指针永远指向系统栈最上面一个栈帧的栈顶

EBP：基址指针寄存器(extended base pointer)，其内存放着一个指针，该指针永远指向系统栈最上面一个栈帧的底部