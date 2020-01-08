# 代码之美

> 2017/11/12
>
> A progam should read like a book. It should have plot, rhythm, and delightful little turns of phrase. —— Knuth’s _Literate Programming_

每个人都有着对美的不同定义，也同时追求着自己心中的那份 **“美”**。

诗人能用文字勾勒出优美的意境，画家能用画笔绘出精美的图案，音乐家能用音符奏出悦耳的旋律。而写代码的人也有着自己对美的定义 ——

> 美的代码是 **优雅** _(elegant)_ 的。

C++ 之父 Bjarne Stroustrup 常常用 **优雅** 这个词来形容让人赏心悦目的代码。

能经得起考验的美，从远处看是美的，到近处看也是美的。优雅的代码也是一样 —— 从整体看，结构很美；从细节看，行文也很美。

## 结构之美 —— 设计

> Modularity is something every software designer does in their sleep. —— Scott Shenker
>
> 模块化是每个软件设计师在梦中思考的问题。

优雅的代码有着良好设计的结构 —— **封装细节以应对变化，分配职责以减少依赖** —— 一方面易于理解和维护，另一方面能减少 “看不见的坑”。

和建筑师设计的楼宇一样，如果结构混乱，不仅看起来不美观，而且许多看不见的问题往往也藏匿在混乱中。只有理清结构，才能把这些问题暴露出来，从而减少潜在的隐患。（常见的有资源生命周期问题）

领域驱动设计 _(Domain Driven Design, DDD)_ 的一个目标也强调了这一点：让分析模型和实现模型保持一致，使业务设计者能 **读懂** 代码。（而我觉得这只是一个软件工程中的 “神话” 😂）

## 行文之美 —— 编码

> Any fool can write code that a computer can understand. Good programmers write code that humans can understand. —— Martin Fowler
>
> 任何傻瓜都可以写出计算机能理解的代码，而优秀程序员可以写出人能读懂的代码。

优雅的代码能很好的利用每一个字符 —— **推敲语句的顺序，揣度命名的用词，追求风格的统一**，在字里行间透出优美的韵律。

代码和诗词曲赋一样，正确的语句顺序可以更好的传达作者的设计思想，恰当的命名可以缩短名字和实现间的 语义距离 _(semantic distance)_，而统一的风格能减少读者的阅读难度。

阅读优雅的代码就和读一首诗、听一首曲、赏一幅画一样，能学习到作者传达的思想，感受到他/她当时的喜怒哀乐。

## 写在最后

尽管许多人并不追求代码之美，但优雅的代码一方面可以减轻团队在技术上的负债，另一方面是一件艺术品，值得用心去打磨。

而在不断打磨自己代码的路上，我发现：当你对一个问题了解的越多，就会发现还有更多东西需要了解。

> 推荐阅读：Martin Fowler 的[《重构》](../2018/Refactoring-Notes.md)及 Kent Beck 的[《实现模式》](../2018/Implementation-Patterns-Notes.md)—— 什么是优雅的代码，为什么代码优雅，如何让代码优雅。

如果有什么问题，**欢迎交流**。😄

Delivered under MIT License &copy; 2017, BOT Man
