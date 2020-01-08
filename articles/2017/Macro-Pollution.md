# 令人头疼的宏定义污染

> 2017/11/18
>
> Prefer `const`s, `enum`s, and `inline`s to `#define`s.
> —— Item 2, Effective C++ 3rd Edition

[heading-numbering]

很多人已经写过了 C 语言的宏，使用不当可能带来的各种后果。

这里，我记录之前遇到的，在掉到 **宏 挖的坑里** 的故事。。。

## TOC [no-toc] [no-number]

[TOC]

## 遇到问题

> 例子都在 Windows 下，使用 MSVC 2017 编译

### 一段很短的代码 —— 掉到了别人挖的坑里

判断程序运行时，传入参数的个数；如果大于 2 个，取前 2 个。

#### [`main.cpp`](Macro-Pollution/main.cpp)

``` cpp
#include <algorithm>
#include "my_module.h"  // 😕
...

int main (int argc, char *argv[]) {
    auto validArgumentCount = std::min (argc, 2);
    ...
}
```

#### 报错 😔

```
error C2589: '(': illegal token on right side of '::'
error C2062: type 'unknown-type' unexpected
error C2059: syntax error: ')'
```

#### 分析 💡

Oh! 经过分析，发现 [`my_module.h`](Macro-Pollution/my_module.h) 的代码里包含了头文件 `Windows.h`（然后使用了 `HWND` 和 `POINT` 两个类型定义）：

``` cpp
#include <Windows.h>  // 😭 oops!
...

HWND GetHwndFromPoint (const POINT &pt);  // depend on Windows.h
```

而头文件 `Windows.h` 里，却定义了一个 `min` 宏：

``` cpp
#define min(a,b) (((a) < (b)) ? (a) : (b))
```

这导致我们的代码被展开为：

``` cpp
auto validArgumentCount = std::(((argc) < (2)) ? (argc) : (2));
```

当然，这是个语法错误。。。

### 另一段代码 —— 掉到了自己挖的坑里

实现 `PathHelper` 的一个[功能](https://docs.microsoft.com/en-us/windows/win32/api/shlwapi/nf-shlwapi-pathappenda)：拼接两个路径，可以识别 `..\` 等路径转换符。

例如，可以利用这个函数返回当前工作路径：

``` cpp
int main (int argc, char *argv[]) {
    auto workingDirectory = PathHelper::PathAppend (argv[0], "..\\");
    ...
}
```

#### [`path_helper.h`](Macro-Pollution/path_helper.h)

``` cpp
#include <string>

class PathHelper {
public:
    static std::string PathAppend (const std::string &base,
        const std::string &more);
    ...
};
```

#### [`path_helper.cpp`](Macro-Pollution/path_helper.cpp)

``` cpp
#include "path_helper.h"

#include <Shlwapi.h>  // 😭 oops!

std::string PathHelper::PathAppend (const std::string &base,
    const std::string &more)
{
    char path[MAX_PATH];
    strncpy (path, base.c_str (), MAX_PATH);

    PathAppendA (path, more.c_str ());
    return path;
}
```

#### 报错 😔

```
error C2039: 'PathAppendW': is not a member of 'PathHelper'
```

#### 分析 💡

类似的，`Shlwapi.h` 里定义了 `PathAppend` 宏（这里打开了 `UNICODE`）：

``` cpp
#define PathAppend PathAppendW
```

把 `PathHelper::PathAppend` 替换成了 `PathHelper::PathAppendW`，于是编译没通过。。。

## 解决问题

如果能放下历史包袱，使用 C++ 提倡的规范，当然是最好的。不过，既然坑已经在那里了，我们只能去面对这个问题了。

> **语言上**，可以通过 空宏、加括号 等方式避免污染。而这里主要从 **设计上** 讨论这个问题。

### 改名 —— 绕开污染源

如果被污染的名称不是对外暴露的接口，或者改动接口名称的成本不高，那么改名是最快的方法。例如，`PathHelper::PathAppend` 改名为 `PathHelper::AppendPath`。

但是如果接口不是自己定义的（例如 `std::min`），我们只能另想办法了。。。

### 使用 `#undef` —— 阻止污染扩散

在引入污染的位置后边，使用一个 `#undef` 可以避免污染向下扩散。例如，上述代码的开头加入 `#undef min`：

``` cpp
#include "my_module.h"  // 😕

#ifdef min
#undef min  // 🙂
#endif
```

另外，需要注意 `#undef` 的时机。例如，下面的代码在引用 `path_helper.h` 的时候，引入的 `PathHelper::PathAppend` 没有被解除污染，会导致函数的声明不一致：

``` cpp
#include <Shlwapi.h>

#include "path_helper.h"  // 😕

#ifdef PathAppend
#undef PathAppend  // 😔 too late!
#endif
```

### 间接 C 相关实现 —— 缩小污染范围

> **间接** _(indirect)_ 在这里是一个动词

尽可能在定义模块的地方（`*.h`）：

- 避免引用可能导致宏污染的 `C` 相关头文件，例如 `Windows.h`
- 尽可能把对 `C` 相关头文件的依赖放到实现上

从而保持接口的干净（最好把自己定义的符号加入到 **命名空间** _(name space)_ 里）。

例如，上述代码里的 `my_module.h` 可以定义不依赖于 `Windows.h` 的接口，而把依赖于 `Windows.h` 的实现限制到 `my_module.cpp` 里。

具体的方法可以参考：

- [Adapter Pattern](Design-Patterns-Notes-2.md#Adapter)
- [Bridge Pattern](Design-Patterns-Notes-2.md#Bridge)
- [Dependency Injection](https://martinfowler.com/articles/injection.html)

## 写在最后 [no-number]

> 2019/12/21 补充：
> 
> 除了 C 语言的宏污染，滥用 C++ 的运算符重载也很可怕（参考：[《恐怖的C++语言》陈皓](https://coolshell.cn/articles/1724.html)）

非常感谢大家的支持！现在 **公众号 `BOTManJL`** 已经开通了评论功能！

感谢 [@WalkerJG](https://github.com/WalkerJG) 提出的修改意见。

如果有什么问题，**欢迎交流**。😄

Delivered under MIT License &copy; 2017, BOT Man
