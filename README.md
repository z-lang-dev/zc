# zc

## 介绍

Z语言的编译器框架

zc（Z Compiler）包括如下几个部分：


- zast：Z语言的通用AST语法树，可以支持多个前端与多个后端
- zf: Z语言的前端，将Z语言源码解析成zast
- zi: Z解释器（Z Interpreter），动态解释执行Z语言源码。
- zt: Z转译器（Z Transpiler），将Z语言转译成其他语言（C/Python/JS）
- zc: Z编译器（Z Compiler），将Z语言编译为汇编语言（X86-64汇编）
- z: 命令入口程序，集成上面所有的模块

## 配套书籍

Z语言是一门面向**学习**和**探索**的编译器框架。

Z语言编译器框架的[《Z语言炼成记》](https://gitee.com/z-lang/devlog)一书同步，我会把设计和开发中总结的信息都记录在这本书里。