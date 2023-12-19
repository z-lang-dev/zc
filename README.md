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

关于Z语言的设计和介绍，可以参看炼成记里的简介：

- [Z语言炼成记README](https://gitee.com/z-lang/devlog)
- [Z语言是什么](https://gitee.com/z-lang/devlog/blob/master/Z%E8%AF%AD%E8%A8%80%E6%98%AF%E4%BB%80%E4%B9%88.md)

## 语法概览

下面是Z语言大致的语法：

```js
// 导入标准库，用use关键字。
use io.[print, open, W]

// 常量定义用const关键字。
// 浮点数的字面值需要加上f后缀。
const PI = 3.1415926f


// 函数定义和Go比较像。函数的关键字是`fn`。Z语言里函数默认是纯函数，即不能产生副作用。
fn add(a int, b int) int {
  // 代码块的最后一个语句即是返回值
  a + b
}

// 加上@mut标注，则函数可以产生副作用
@mut
fn writeFile(name str, s str) bool {
  let f = open(name, W) 
  on(exit) {
    f.close()
    return false
  }
  f.write(s)
  return true
}

// 加上@var标注，则函数可以不指定参数和返回值类型，这样的函数和JS与Python的函数类似。
@var
fn alert(message) {
  message = "Alert!! $message"
  print(message)
}

// main函数是特殊的函数，它是程序的入口。不需要指定@mut或@var标注。
fn main {
  // 函数调用与C一致。
  print("Hello, world!") // 语句结尾不需要';'。

  // Z语言支持嵌入字符串。这里PI值被直接嵌入到字符串里了。
  print("Here is pi: ${PI}")

  // let标量，类似于C的变量，但它的值是不可变的。
  let a int = 10 // 可以在变量名称后直接指定类型，如果不指定，编译器会自动推导。
  a = 12 // 错误！a是不可变的量。

  // mut变量。这个相当于C语言里的普通变量。
  mut b = 5 // 支持基本的类型推导。
  b = b * 2 // 正确！b是可变量。
  b = "Z语言" // 错误！不能改变变量的类型。

  // var幻量。这个相当于JavaScript里的var变量，不但值可以变，类型也可以变。
  var c = 5
  c = "Z语言" // 正确！c是幻量，可以从整数类型变成字符串。

  print("a+b is $add(a, b)")
}
```

注：上面的代码是语法的设计，还没有完全实现。