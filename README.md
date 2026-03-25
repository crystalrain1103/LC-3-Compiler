<span style="font-size: 42px; font-weight: bold; text-align: center; display: block;">LC-3 Compiler</span>

[TOC]

## 这是什么？

这是一个**C语言**实现的**LC-3汇编器**，其接受一个`.asm`文本文件作为输入，输出`.bin`汇编后的机器码二进制文件和`.txt`符号表文件。

## 源代码介绍

### 文件结构

本汇编器一共有**6**个源文件，其中：

- **1**个头文件：
- - `lc3asm.h`：包含其他源文件所用到的**头文件**、**常量**、**错误处理宏**、**数据结构**、**函数原型声明**。
- **5**个源文件：
- - `main.c`：程序运行入口。
  - `parse.c`：处理原始文件。
  - `symbol.c`：处理符号表。
  - `instruction.c`：处理指令。
  - `assemble.c`：处理汇编过程。

### 代码实现

1. 数据结构：

   - **文件结构**：包含输入源文件，输出二进制文件，输出符号表文件

     ```c
     typedef struct files_t {
       char in_fname[FILENAME_LEN], out_bin_fname[FILENAME_LEN], out_sym_fname[FILENAME_LEN];
       FILE *in;
       FILE *outbin;
       FILE *outsym;
     } files;
     ```

   - **行结构**：读入的每一行，包含原始行和拆分后的tokens

     ```c
     typedef struct line_t {
         char raw_line[LINE_LEN];
         char tokens[TOKEN_NUM][TOKEN_LEN];
     } line;
     ```

   - **符号表结构**：采用单向链表实现

     ```c
     typedef struct symbol_t {
         char label[TOKEN_LEN];
         unsigned int addr;
         struct symbol_t *next;
     } symbol, *symtable;
     ```

2. 函数功能：

   - `parse.c`

     - ```c 
       void readline(line *l, files *f);
       ```

       读入源文件，向行结构中存储原始行和该行除去注释外的所有tokens（包括label、operation和operand）

     - ```c
       void reescape(char *raw);
       ```

       读入一行未分割的源码，将escape字符还原。（比如`'\n'`作为文本文件读入的时实际为两个字符：`'\\'`和`'n'`，该函数将其还原为`'\n'`）

     - ```c
       int getnumber(char *token);
       ```

       获取形如`x1234`、`#1234`一类带有十六进制和十进制数的十进制表示

     - ```c
       int isoperation(char *token);
       ```

       判断token是否为一个operation

   - `symbol.c`

     - ```c 
       void append_symtable(symtable *table, char *label, int addr);
       ```

       向符号表中追加label

     - ```c
       int search_symtable(symtable table, char *label);
       ```

       在符号表中搜索给定的label

     - ```c
       void destroy_symtable(symtable table);
       ```

       摧毁符号表

     - ```c
       void write_symfile(symtable table, files *f);
       ```

       将符号表写入`.txt`文件中

   - `instruction.c`

     - ```c 
       char *parsereg(char *token);
       ```

       将token对应的寄存器解析为二进制字符串

     - ```c
       char *int2bin(int n, char *bin, int size);
       ```

       将十进制整数n转换为对应的二进制字符串bin

     - ```c
       int process_inst(files *f, line l, int oploc, symtable table, int *AC);
       ```

       **（核心函数）**根据传入的tokens产生对应的二进制码

   - `assemble.c`

     - ```c 
       void assemble(files *f);
       ```

       汇编文件入口函数

     - ```c
       int firstpass(files *f, symtable *table);
       ```

       汇编过程*first pass*，`assemble`函数的辅助函数

     - ```c
       int secondpass(files *f, symtable table);
       ```

       汇编过程*second pass*，`assemble`函数的辅助函数

3. 错误处理：

   - **缺少参数错误**

     ```c
     #define checkLackArgumentsError(opname, opoffset, addr) 
     	if (!l.tokens[oploc+opoffset][0]) {
             printLine(addr);
             fprintf(stderr, "Invaild format for "opname" : lack arguments\n");
             return 0;
         }
     ```

   - **参数过多错误**

     ```c
     #define checkTooManyArgumentsError(opname, opoffset, addr) 
     	if (l.tokens[oploc+opoffset][0]) {
             printLine(addr);
             fprintf(stderr, "Invaild format for "opname" : too many arguments\n");
             return 0;
     	}
     ```

   - **非法参数错误**

     ```c
     #define raiseInvalidArgumentError(opname, argument, opoffset, addr) {
     	printLine(addr);
     	fprintf(stderr, "Invalid operands for "opname" : expected "argument", but got [%s]\n", l.tokens[oploc+opoffset]);
     	return 0;
     }
     ```

   - **参数越界错误**

     ```c
     #define raiseArgumentOutOfRangeError(opname, argument, opoffset, addr) {
     	printLine(addr);
     	fprintf(stderr, "Invalid operands for "opname" : "argument" [%s] out of range\n", l.tokens[oploc+opoffset]);
     	return 0;
     }
     ```

   - **标签未识别错误**

     ```c
     #define raiseLabelNotFoundError(opname, opoffset, addr) {
     	printLine(addr);
     	fprintf(stderr, "Label not found for "opname" : [%s]\n", l.tokens[oploc+opoffset]);
     	return 0;
     }
     ```

   - 此外还有一些特殊错误，如**未找到`.ORIG`错误**、**不支持的指令错误**等等，因为没有上述错误普遍，因此未在宏定义中编写。

## 使用方法

- 使用指令

  ```shell 
  gcc -g -Wall -Werror -std=c99 -o "lc3asm" "main.c" "parse.c" "symbol.c" "assemble.c" "instruction.c"
  ```

  编译后得到`lc3asm.exe`，之后执行

  ```shell 
  lc3asm.exe {name}.asm
  ```

  即可。

- 此外，压缩包中还配套有批量汇编脚本`batch_assemble.bat`，批量检测脚本`batch_bin_verify.bat`和`batch_sym_verify.bat`，可以使用它们进行批量汇编和批量检测。

  - 编译脚本`batch_assemble.bat`会检测`.\input`文件夹（当前目录`.\`为脚本运行目录），向其中的所有文件filename执行

    ```shell
    .\lc3asm.exe filename
    ```

    执行结束后创建`.\output`文件夹，将所有`.bin`文件放入`.\output\bin`文件夹中，所有符号表文件放入`.\output\symbol`文件夹中。

  - 检测脚本`batch_bin_verify.bat`和`batch_sym_verify.bat`会检测基准文件夹`.\bin`（`.\symbol`）与比对文件夹`.\output\bin`（`.\output\symbol`）（当前目录`.\`为脚本运行目录），对比其中的所有文件。如果基准文件夹下出现比对文件夹下没有的文件名，该文件记为**Missing**，否则将对文件名相同的文件进行比对（使用fc函数）

    ```shell
    fc /b "%%f" "%FOLDER_B%\%%f" >nul
    ```

    匹配成功记为**Match**，匹配失败记为**Mismatch**。

    **（所以想要正确使用批量匹配，需要确定文件夹严格与上面一致，要保证匹配文件的文件名（包括后缀名）完全一致！）**

  - 文件树示例如下：

    ```
    ./
    ├── input/
    │   ├── input1.asm
    │   └── ...
    ├── bin/
    │   ├── input1.bin
    │   └── ...
    ├── symbol/
    │   ├── input1.asm_symtable.txt
    │   └── ...
    ├── (执行batch_assemble.bat后创建)output/
    │   ├── bin/
    │   │   ├── input1.bin
    |	|	└── ...
    │   └── symbol/
    |		├── input1.asm_symtable.txt
    │       └── ...
    ├── batch_assemble.bat
    ├── batch_bin_verify.bat
    ├── batch_sym_verify.bat
    └── lc3asm.exe
    ```

## 功能展示

对于给出的往年测试中，该汇编器均能给出正确的二进制码和错误提示。下面是部分输出展示：

<img src=".\README.assets\1.png" alt="1" style="zoom:50%;" /><img src=".\README.assets\2.png" alt="1" style="zoom:50%;" />
<img src=".\README.assets\3.png" alt="1" style="zoom:50%;" /><img src=".\README.assets\4.png" alt="1" style="zoom:50%;" />
<img src=".\README.assets\5.png" alt="1" style="zoom:50%;" /><img src=".\README.assets\6.png" alt="1" style="zoom:50%;" />