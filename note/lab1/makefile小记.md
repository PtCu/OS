
**流程:**
（单冒号表示依赖关系，双冒号好像也是。）

makefile 的make命令默认只执行第一条命令.通常我们会把生成最后的执行文件放在第一行，即只需要键入make就可以。而有些makefile文件是将生成最后的可执行文件的代码没有放在第一行，所以如果make的话，就只执行第一行的code.

具体工作顺序是：当在 shell 提示符下输入 make 命令以后。 make 读取当前目录下的 Makefile 文件，并将 Makefile 文件中的第一个目标作为其执行的“终极目标”，开始处理第一个规则（终极目标所在的规则）。在我们的例子中，第一个规则就是目标 "main" 所在的规则。规则描述了 "main" 的依赖关系，并定义了链接 ".o" 文件生成目标 "main" 的命令；make 在执行这个规则所定义的命令之前，首先处理目标 "main" 的所有的依赖文件（例子中的那些 ".o" 文件）的更新规则（以这些 ".o" 文件为目标的规则）。

    main:main.o test1.o test2.o
    gcc main.o test1.o test2.o -o main
    main.o:main.c test.h
    gcc -c main.c -o main.o
    test1.o:test1.c test.h
    gcc -c test1.c -o test1.o
    test2.o:test2.c test.h
    gcc -c test2.c -o test2.o

**中间文件**

对这些 ".o" 文件为目标的规则处理有下列三种情况：
- 目标 ".o" 文件不存在，使用其描述规则创建它；
- 目标 ".o" 文件存在，目标 ".o" 文件所依赖的 ".c" 源文件 ".h" 文-件中的任何一个比目标 ".o" 文件“更新”（在上一次 make 之后被修改）。则根据规则重新编译生成它；
- 目标 ".o" 文件存在，目标 ".o" 文件比它的任何一个依赖文件（".c" 源文件、".h" 文件）“更新”（它的依赖文件在上一次 make 之后没有被修改），则什么也不做。

编译时生成的 ".o" 文件。作用是检查某个源文件是不是进行过修改，最终目标文件是不是需要重建。我们执行 make 命令时，只有修改过的源文件或者是不存在的目标文件会进行重建，而那些没有改变的文件不用重新编译，这样在很大程度上节省时间，提高编程效率。小的工程项目可能体会不到，项目工程文件越大，效果才越明显。

**变量的定义**
 Makefile 文件中定义变量的基本语法如下：

    变量的名称=值列表

Makefile 中的变量的使用其实非常的简单，因为它并没有像其它语言那样定义变量的时候需要使用数据类型。变量的名称可以由大小写字母、阿拉伯数字和下划线构成。等号左右的空白符没有明确的要求，因为在执行 make 的时候多余的空白符会被自动的删除。至于值列表，既可以是零项，又可以是一项或者是多项。如：

    VALUE_LIST = one two three

调用变量的时候可以用\$(VALUE_LIST) 或者\${VALUE_LIST}来替换，这就是变量的引用。实例：

    OBJ=main.o test.o test1.o test2.o
    test:$(OBJ)
        gcc -o test $(OBJ)
这就是引用变量后的 Makefile 的编写，比我们之前的编写方式要简单的多。当要添加或者是删除某个依赖文件的时候，我们只需要改变变量 "OBJ" 的值就可以了。

**简单赋值：**
编程语言中常规理解的赋值方式，只对当前语句的变量有效

    x:=foo
**递归赋值 ( = )** 赋值语句可能影响多个变量，所有目标变量相关的其他变量都受影响。

    x=foo
    y=$(x)b
    x=new
    test：
        @echo "y=>$(y)"
        @echo "x=>$(x)"
结果为

    y=>newb
    x=>new
 **条件赋值 ( ?= )** 如果变量未定义，则使用符号中的值定义变量。如果该变量已经赋值，则该赋值语句无效。

    x:=foo
    y:=$(x)b
    x?=new
    test：
        @echo "y=>$(y)"
        @echo "x=>$(x)"
输出：

    y=>foob
    x=>foo

** 追加赋值 ( += )** 原变量用空格隔开的方式追加一个新值。

    x:=foo
    y:=$(x)b
    x+=$(y)
    test：
        @echo "y=>$(y)"
        @echo "x=>$(x)"
输出：

    y=>foob
    x=>foo foob


**自动化变量**
关于自动化变量可以理解为由 Makefile 自动产生的变量。在模式规则中，规则的目标和依赖的文件名代表了一类的文件。规则的命令是对所有这一类文件的描述。我们在 Makefile 中描述规则时，依赖文件和目标文件是变动的，显然在命令中不能出现具体的文件名称，否则模式规则将失去意义。

那么模式规则命令中该如何表示文件呢？就需要使用“自动化变量”，自动化变量的取值根据执行的规则来决定，取决于执行规则的目标文件和依赖文件。下面是对所有的自动化变量进行的说明：

**自动化变量**
简言之，\$@--目标文件，\$^--所有的依赖文件，\$<--第一个依赖文件。
|自动化变量|	说明|
|---|---|
|\$@	|表示规则的目标文件名。如果目标是一个文档文件（Linux 中，一般成 .a 文件为文档文件，也成为静态的库文件）,那么它代表这个文档的文件名。在多目标模式规则中，它代表的是触发规则被执行的文件名。|
|\$%|	当目标文件是一个静态库文件时，代表静态库的一个成员名。|
|\$<|	规则的第一个依赖的文件名。如果是一个目标文件使用隐含的规则来重建，则它代表由隐含规则加入的第一个依赖文件。|
|\$?|	所有比目标文件更新的依赖文件列表，空格分隔。如果目标文件时静态库文件，代表的是库文件（.o 文件）。|
|\$^|	代表的是所有依赖文件列表，使用空格分隔。如果目标是静态库文件，它所代表的只能是所有的库成员（.o 文件）名。一个文件可重复的出现在目标的依赖中，变量“$^”只记录它的第一次引用的情况。就是说变量“$^”会去掉重复的依赖文件。|
|\$+|	类似“$^”，但是它保留了依赖文件中重复出现的文件。主要用在程序链接时库的交叉引用场合。|
|\$*|	在模式规则和静态模式规则中，代表“茎”。“茎”是目标模式中“%”所代表的部分（当文件名中存在目录时，“茎”也包含目录部分）。|
下面我们就自动化变量的使用举几个例子。

实例1：

    test:test.o test1.o test2.o
            gcc -o $@ $^
    test.o:test.c test.h
            gcc -o $@ $<
    test1.o:test1.c test1.h
            gcc -o $@ $<
    test2.o:test2.c test2.h
            gcc -o $@ $<
这个规则模式中用到了 "\$@" 、"\$<" 和 "\$^" 这三个自动化变量，对比之前写的 Makefile 中的命令，我们可以发现 "\$@" 代表的是目标文件test，“\$^”代表的是依赖的文件，“\$<”代表的是依赖文件中的第一个。我们在执行 make 的时候，make 会自动识别命令中的自动化变量，并自动实现自动化变量中的值的替换，这个类似于编译C语言文件的时候的预处理的作用。

实例2：

    lib:test.o test1.o test2.o
        ar r $?
假如我们要做一个库文件，库文件的制作依赖于这三个文件。当修改了其中的某个依赖文件，在命令行执行 make 命令，库文件 "lib" 就会自动更新。"$?" 表示修改的文件。

GNU make 中在这些变量中加入字符 "D" 或者 "F" 就形成了一系列变种的自动化变量，这些自动化变量可以对文件的名称进行操作。

下面是一些详细的描述：

|变量名|	功能|
|---|---|
|\$(@D)|	表示文件的目录部分（不包括斜杠）。如果 "\$@" 表示的是 "dir/foo.o" 那么 "\$(@D)" 表示的值就是 "dir"。如果 "\$@" 不存在斜杠（文件在当前目录下），其值就是 "."。|
|\$(@F)|	表示的是文件除目录外的部分（实际的文件名）。如果 "\$@" 表示的是 "dir/foo.o"，那么 "\$@F" 表示的值为 "dir"。|
|\$(*D) \$(*F)|	分别代表 "茎" 中的目录部分和文件名部分|
|\$(%D) \$(%F)|	当以 "archive(member)" 形式静态库为目标时，分别表示库文件成员 "member" 名中的目录部分和文件名部分。踏进对这种新型时的目标有效。|
|\$(<D) \$(<F)|	表示第一个依赖文件的目录部分和文件名部分。|
|\$(^D) \$(^F)|	分别表示所有依赖文件的目录部分和文件部分。|
|\$(+D) \$(+F)|	分别表示所有的依赖文件的目录部分和文件部分。|
|\$(?D) \$(?F)|	分别表示更新的依赖文件的目录部分和文件名部分。|

**常用函数**
**\$(foreach \<var>,\<list>,\<text>)**
函数的功能是：把参数\<list>中的单词逐一取出放到参数\<var>所指定的变量中，然后再执行\<text>所包含的表达式。每一次\<text>会返回一个字符串，循环过程中，\<text>的返所返回的每个字符串会以空格分割，最后当整个循环结束的时候，\<text>所返回的每个字符串所组成的整个字符串（以空格分隔）将会是 foreach 函数的返回值。所以\<var>最好是一个变量名，\<list>可以是一个表达式，而\<text>中一般会只用\<var>这个参数来一次枚举\<list>中的单词。

    name:=a b c d
    files:=$(foreach n,$(names),$(n).o)
    all:
        @echo $(files)   

执行 make 命令，我们得到的值是“a.o b.o c.o d.o”。    

**$(if \<condition>,\<then-part>)或(if\<condition>,\<then-part>,\<else-part>)**
都懂

**$(call \<expression>,\<parm1>,\<parm2>,\<parm3>,...)**
call是用来调用函数的
call 函数是唯一一个可以用来创建新的参数化的函数。我们可以用来写一个非常复杂的表达式，这个表达式中，我们可以定义很多的参数，然后你可以用 call 函数来向这个表达式传递参数。

当 make 执行这个函数的时候，expression参数中的变量\$(1)、\$(2)、\$(3)等，会被参数parm1，parm2，parm3依次取代。而expression的返回值就是 call 函数的返回值。

    reverse = $(1) $(2)
    foo = $(call reverse,a,b)
    all：
        @echo $(foo)

foo 的值就是“a b”

**一些其他用法**
1. **调用shell**
   $(shell ...)
2. **dd** 
   Linux dd命令用于读取、转换并输出数据。dd可从标准输入或文件中读取数据，根据指定的格式来转换数据，再输出到文件、设备或标准输出。
   - if=文件名：输入文件名，默认为标准输入。即指定源文件。
   - of=文件名：输出文件名，默认为标准输出。即指定目的文件。
   - seek=blocks：从输出文件开头跳过blocks个块后再开始复制。
   - count=blocks：仅拷贝blocks个块，块大小等于ibs指定的字节数。
   - conv=<关键字>,关键字有好几种，下面这几种：
      - notrunc：不截短输出文件
      - unblock：使每一行的长度都为cbs，不足部分用空格填充
  
    在Linux 下制作启动盘，可使用如下命令：

        dd if=boot.img of=/dev/fd0 bs=1440k 

3. **ld**
    ld命令是GNU的连接器，将目标文件连接为可执行程序。
    语法为 **ld(选项)(参数)**
    - 选项
        - -o：指定输出文件名；
        - -e：指定程序的入口符号。
    - 参数
        - 目标文件：指定需要连接的目标文件。