# 关于写代码的几点思考

> 2017/10/29
>
> When you feel the need to write a comment, first try to refactor. —— Martin Fowler

## [no-toc] TOC

[TOC]

## 写在前面 TL;DR

虽然我写代码的时间不长，但也折腾过前端后端 PC 端移动端，做过几次外包和学生项目，敲过几个 naive 的框架和库，填过坑、救过火，~~带上注释~~ 应该也写过 _100k_ 行代码了。

> 学而不思则罔。——《论语》

这几年的心得：

- 花了半年、一年学习某个技术
- 感觉自己学会了这个技术
- 和有经验的人交流之后，发现还有很多需要学习的地方
- 然后重新回去学习
- 感觉自己真的学会了这个技术
- 和更强的人交流，发现还能更进一步
- 于是，重复上述过程。。。

学习 **进步** 的基础在于通过 **交流** 看到自己的不足，而交流的基础在于 **总结** 自己学过的知识，总结知识的一个很好的方式就是 **写作**。

> 关于写作，时间是一个要紧事。你只要想写，时间一定是可以挤出来的。你打游戏有时间，追女孩有时间，学习、写作怎么也不应该没时间。问题的关键在于你对这件事的欲望有多深，或者说兴趣点有多高。—— [陈皓](https://coolshell.cn)

所以，我真的希望 通过这些文字和更多的人交流、一起提升；也希望 阅读过我的文章的人（包括若干年后的自己），能 **多多指教**~ 😁

## 《重构》读后感

最近在看 Martin Fowler 的《重构》。虽然里边有很多看不懂的长难句和冷笑话，但是这本书写的很生动，而且总能给人一种 “啊哈” 的惊喜。另外，也很喜欢 Martin 这位谦称自己为 “分享者” 的大师。

### 磨刀不误砍柴工

> When you find you have to add a feature to a program, and the program's code is not structured in a convenient way to add the feature, first refactor the program to make it easy to add the feature, then add the feature. —— Martin Fowler
>
> 当你需要给程序添加新功能，但基于现有的代码结构并不能方便的添加时，首先重构代码，让它易于添加新功能，然后添加新功能。

很多时候，一开始的设计往往没有考虑到未来的需求变化。当需要实现新需求时，如果直接在原来的设计上修修补补，很容易导致原本很华丽的设计，一点点褪去漂亮的颜色，然后逐渐的腐烂掉。

例如，前两个月看过一份几万行的代码：有一个基础模块，模块内部能注册自己（模块需要注册才能被外部使用）；另外又添加了一个扩展模块，实现对基础模块的扩展能力；由于注册模块的能力放在基础模块内部，添加这个扩展模块就导致了需要修改已有的基础模块。—— 从而构成了扩展模块和基础模块的 **双向依赖**。

OMG! 原本让模块自己注册自己的设计非常的漂亮，经过这个功能扩展之后，整个代码开始变得混乱了。。。（我觉得一般可以利用 [控制反转 _(IoC)_](Thinking-Scalability.md#控制反转-计算可扩展性) 的原则重构这种注册的能力 😂）

随着扩展模块的增加，代码的结构越来越混乱，而增加一个功能的成本也越来越高。。。

### 人总会犯错

> Before you start refactoring, check that you have a solid suite of tests. These tests must be self-checking. —— Martin Fowler
>
> 开始重构前，确认是否已经有了一套完备的测试，而且测试必须能明确的指出错误。

人会疲劳，疲劳会导致在细节上出错；细节上的错误不容易被察觉，而且往往可能导致更大的问题。而测试（自动化测试，因为人工测试也有人的参与）可以尽快的发现问题，并且指出错误。

保证代码可靠性需要有两个维度：一个是 **代码本身** 检查异常，另一个是 **测试** 结果是否符合预期。这两个维度都需要良好的设计：代码尽可能的考虑到所有可能出现的正常和异常情况，测试覆盖到代码考虑到的正常和异常情况。

即便如此，程序仍可能出现问题：因为测试完全覆盖，只是说明代码里考虑到的情况都能被正确的处理；而代码里考虑到的情况只是编写者能考虑到的部分，可能有遗漏。

所以，人永远是不可靠的，而提升个人水平能让自己变得更可靠。

### 不要过于相信直觉

> Even if you know exactly what is going on in your system, measure performance, don't speculate. —— Ron Jeffries
>
> 即使你非常了解自己的系统，也要测量性能，而不是假设。

许多时候，我们往往会纠结于一些小事，然后忽略了一些重要的问题，从而导致了错误。错误出现后，我们也常常根据自己的 “丰富经验” 推断出错的位置。

许多有经验的 “算法专家” 可能会给你指出一条明路：这里的 $O(n^2)$ 需要优化为 $O(n*log_{2}n)$ 的算法；这里应该用上 [KMP 算法](https://en.wikipedia.org/wiki/Knuth–Morris–Pratt_algorithm)；这里应该。。。也许，顺便称道一下当年他们打比赛的时候如何通过使用这些奇技淫巧拿了高分。。。

然而问题并不一定是那样的。例如，有一次遇到了一个问题，一个算法递归了几层就错误的停止了。。然后看了代码，最后发现是因为栈溢出导致的问题。代码里边使用了一个大数组，然后分配在栈上。。。OMG again! 由于不是每次都会进入递归的路径，所以这个问题可能不会出现。

``` cpp
void fn () {
    std::array<int, 65536> array;  // on stack!
    ...
    fn ();                         // recursive
}
```

- `std::array<int, 65536>` 内部有一个 `int [65536]` 成员变量
- 
- 另外，可以利用编译器的 `sizeof` 功能计算出对象的大小

``` cpp
// following results are calculated on MSVC 2017 Update 4
constexpr auto sizeOfIntVec = sizeof (std::vector<int>);      // 16
constexpr auto sizeOfIntArr = sizeof (std::array<int, 1024>); // 4 * 1024

using Str = std::string;
constexpr auto sizeOfStrVec = sizeof (std::vector<Str>);      // 16
constexpr auto sizeOfStrArr = sizeof (std::array<Str, 1024>); // 28 * 1024
```

最后解决的办法是：

- 把 `std::array` 改为 `std::vector` 在堆上分配内存
- 对数据分块处理，并通过 `stream` 方式进行 IO

正如我们老板经常说的，“一切以来源可靠的数据说话”，而不是单凭经验办事。

## 写在最后

> Happy Coding. —— Erich Gamma

我是一个注重方法论的人，做任何事情之前都会先思考如何去做，然后再去做。而写代码实际上是将生活中的方法论应用到软件上，let's enjoy it. 😉

![Happy Coding](Thinking-Coding/Happy-Coding.jpg)

本文仅是我对 这几年写代码 的一些理解。如果有什么问题，**欢迎交流**。😄

Delivered under MIT License &copy; 2017, BOT Man
