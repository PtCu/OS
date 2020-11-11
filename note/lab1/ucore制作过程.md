**gcc(编译),并带有大量参数,参数含义如下:**

- -Idir: 添加头文件搜索路径

- -march=cpu-type: 为特定的CPU编译二进制代码

- -fno-builtin: 不承认没有builtin前缀的函数作为内建函数

- -fno-PIC: 不生成位置无关代码(position-independent code)(应该是这个意思,但是这个好像并不重要)

- -Wall: 打开所有等级的编译警告

- -ggdb: 生成可供gdb调试的信息

- -m32: 生成32位机器上的代码

- -gstabs: 以stabs格式生成调试信息,但是不包括gdb调试信息(这个也不太重要)

- -nostdinc: 不在系统缺省的头文件目录中找头文件,即只在参数(例如-I)指定的文件夹中寻找头文件

- -fno-stack-protector: 禁用堆栈保护

**ld(链接),带有少量参数,参数含义如下:**

- -m: 模拟对应的链接器(此处为i386)

- -T: 显式的向链接器提供命令文件(也叫链接器脚本)

- -N: 将文本和数据部分设置为可读和可写.而且,不对数据段进行页面对齐(page-align)

- -e: 设置一个显式的符号作为程序执行的入口

- -Ttext: 设置输出文件的文本段的起始地址

**dd(用指定大小的块拷贝一个文件,并在拷贝的同时进行指定的转换),带有少量参数,参数含义如下:**

- if=file: 指定源文件,缺省为stdin(有一处使用了/dev/zero,这是一个虚拟设备,可以无限的提供空字符,常用来生成一个特定大小的文件)

- of=file: 指定目的文件,缺省为stdout

- ibs=bytes: 指定块的大小,缺省为512bytes(此参数并未给出,故取缺省值)

- conv=: 用指定的参数转换文件(notrunc为不截短输出文件)

- seek=N: 跳过输出文件的前N个块


了解了所有的命令及参数后,一切都豁然开朗,容易看出,ucore.img的生成分为三大步.

1. 生成bin/kernel: 较为简单,先将16个.c文件编译称为.o文件,再将其链接生成bin/kernel

2. 生成bin/bootblock:

    1. 先由bootasm.S,bootmain.c,sign.c分别生成bootasm.o,bootmain.o,sign.o

    2. 再将bootasm.o和bootmain.o链接生成bootblock.o (接下来两步从终端输出的信息是看不出来的,只能去看makefile文件,找到bootblock(line 161))

    3. 拷贝二进制代码bootblock.o到bootblock.out

    4. 使用sign处理bootblock.out生成bootblock

3. 生成ucore.img

    1. 初始化ucore.img为大小为10000个512bytes的全零文件

    2. 将bootblock拷贝到ucore.img中

    3. 将kernel拷贝到ucore.img中(append到bootblock后)



