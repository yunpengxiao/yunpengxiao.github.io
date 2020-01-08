# Node 单元测试的 Stub 总结

> 2017/9/9
>
> 走出舒适区，and sharpen your teeth.

[heading-numbering]

**2017/11/25 更新**：

- 本文使用的技术不是 _Mock_ 而是 _Stub_，详见 [Martin 的博客](https://martinfowler.com/articles/mocksArentStubs.html)
- 本文使用的单元测试方法为 常规的 **状态检查** _(state verification)_ 测试，而不是基于 Mock 风格的 **行为检查** _(behavior verification)_ 测试
- [Mocha 测试框架](http://mochajs.org/#assertions) 提供了基于 Mock 风格的扩展

## TOC [no-number] [no-toc]

[TOC]

## 为什么

最近花了两周时间敲完了一个 Node 包（重构前 C++ 版本近 10k 行），然后完成了这个模块几百个 case 的单元测试。

而在设计单元测试时，需要重写一些不属于自己代码的关键模块（暴露相同的接口，替换为自己的实现），从而避免外部依赖导致的 **副作用** _(side effect)_。例如，需要依赖于一些读写数据库、发起 HTTP 请求资源的代码，我们可以通过设置模块接口的 **桩代码** _(Stub)_，实现一套简单的逻辑。

而 Node 主要使用 JavaScript 作为编程语言，拥有 **脚本语言动态性** 的优势。借助这个特性，我们可以方便的编写模块接口的 Stub 代码。（对于静态语言，要求模块的划分、代码的编写具有可测试性）

## Module 的设计模式

根据 [Node.js 设计模式](https://www.nodejsdesignpatterns.com/) 总结，Node 中的模块定义的模式大致可以分为四种。

> 所有代码载自 [Node.js 设计模式](https://www.nodejsdesignpatterns.com/)

### 导出名字

相对于定义一个 **命名空间**，然后将 **相似的功能** 挂载到同一个命名空间底下。这种方法和 _CommonJS_ 的 `exports` 兼容。

``` javascript
// file logger.js
exports.info = function(message) {
    console.log('info: ' + message);
};
exports.verbose = function(message) {
    console.log('verbose: ' + message);
};

// file main.js
var logger = require('./logger');
logger.info('This is an informational message');
logger.verbose('This is a verbose message');
```

### 导出函数

如果模块比较简单，只有 **单一的功能**，那么可以导出为一个函数。

``` javascript
// file logger.js
module.exports = function(message) {
    console.log('info: ' + message);
};

// file main.js
var logger = require('./logger');
logger('This is an informational message');
```

### 导出构造函数

可以将模块封装为一个 **类**，然后导出类的构造函数。（导出 `class` 同理）

> `prototype` 和 `constructor` 参考：
>
> - https://developer.mozilla.org/en-US/docs/Web/JavaScript/Inheritance_and_the_prototype_chain
> - http://www.haorooms.com/post/js_constructor_pro

``` javascript
// file logger.js
function Logger(name) {
    this.name = name;
};
Logger.prototype.info = function(message) {
    this.log('info: ' + message);
};
Logger.prototype.verbose = function(message) {
    this.log('verbose: ' + message);
};
module.exports = Logger;

// file logger.js
var Logger = require('./logger');
var dbLogger = new Logger('DB');
dbLogger.info('This is an informational message');
var accessLogger = new Logger('ACCESS');
accessLogger.verbose('This is a verbose message');
```

### 导出对象实例

可以通过导出一个 **对象实例**，实现 Node 中的 _Singleton_ 模式。

``` javascript
// file logger.js
function Logger(name) {
    this.count = 0;
    this.name = name;
};
Logger.prototype.log = function(message) {
    this.count++;
    console.log('[' + this.name + '] ' + message);
};
module.exports = new Logger('DEFAULT');

// file main.js
var logger = require('./logger');
logger.log('This is an informational message');
```

## 依赖 Module 的方式

软件工程中，常见的模块之间常见的依赖方式有：

- **硬编码** _(hardcoded)_
  - 在代码中，**直接指定** 依赖的模块
- **依赖注入** _(dependency injection)_
  - 由模块外部的 **调用者传入** 模块内部需要的依赖
- **依赖查找** _(dependency lookup)_
  - 定义一个统一的 **资源管理器**，通过服务定位的方式，查找依赖
  - 可以通过 **硬编码** 或 **依赖注入** 的方式访问 **资源管理器**

## Stub 的方法

**单元测试** 常常使用 **白盒测试** 的方法，目标是构造测试用例，尽可能覆盖模块的所有 **路径**（**黑盒测试** 则是尽可能覆盖所有 **等价类**）。

给被依赖模块设置 Stub 时，我们需要在测试我们自己代码之前，完成依赖的修改；并在测试结束后，完成依赖的恢复，避免污染其它地方对同一模块的依赖。（当然，我们只需要覆盖 **被测试代码**，不需要知道被依赖模块如何实现）

以流行的 [Mocha 测试框架](http://mochajs.org/) 为例（测试前执行 `before`，测试后执行 `after`），设置测试 Stub 流程为：

``` javascript
before(function() {
    // Setup Stub
});

describe(caseName, function() {
    // Test
});

after(function() {
    // Restore Stub
});
```

我们针对

- **依赖模块** 的三种方式
- **被依赖模块** 四种导出模式

设计不同的 Stub 具体方法。

### 硬编码 - 导出名字

这种情况最简单：只需要 **替换** 被测试代码里使用的、**命名空间下的方法**。

``` javascript
const depModule = require('./depModule');
const _func = depModule.func;

before(function() {
    // Setup Stub
    depModule.func = function () { ... };
});

after(function() {
    // Restore Stub
    depModule.func = _func;
});
```

### 硬编码 - 导出构造函数

这种情况稍微复杂一些：需要 **替换** 被测试代码里使用的、**类的方法**（`prototype` 的属性）。

``` javascript
const depModule = require('./depModule');
const proto = depModule.prototype;
const _func = proto.func;

before(function() {
    // Setup Stub
    proto.func = function () { ... };
});

after(function() {
    // Restore Stub
    proto.func = _func;
});
```

即使被测试代码里使用的 `depModule` 对象已经实例化，修改 `depModule.prototype` 的属性，可以在递归检查 `prototype` 链时实现 Stub —— 不需要重新加载被测试代码的模块。

### 硬编码 - 导出对象实例

由于对象已经被初始化，所以我们不能修改对象本身。但是可以修改对象的、我们需要访问的方法。

#### 重载 对象本身 的方法

**添加** 被测试代码里使用的、**对象本身的方法**（非 `prototype` 属性）：被测试代码在调用对应方法时，不需要递归检查 `prototype` 链。

``` javascript
const depModule = require('./depModule');

before(function() {
    // Setup Stub
    depModule.func = function () { ... };
});

after(function() {
    // Restore Stub
    delete depModule.func;
});
```

#### 修改 `__proto__` 的属性

和 [sec|硬编码 - 导出构造函数] 类似，通过 **替换** 被测试代码里使用的、**类的方法** 实现。不同的是：这里需要利用 `Object.getPrototypeOf` 定位到对应的 `prototype` 对象。

``` javascript
const depModule = require('./depModule');
const proto = Object.getPrototypeOf(depModule);
const _func = proto.func;

before(function() {
    // Setup Stub
    proto.func = function () { ... };
});

after(function() {
    // Restore Stub
    proto.func = _func;
});
```

> 其中，`Object.getPrototypeOf(depModule)` 等价于
> - [`depModule.constructor.prototype`](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object/constructor)（不一定满足，基于已经良好定义 `constructor` 的假设）
> - [`depModule.__proto__`](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object/proto)（标准还未定义，但 Node 支持）

### 硬编码 - 导出函数

这种情况最为复杂：**不能动态修改** 一个定义好的 **函数内部代码**。

但是 Node 为我们提供了 **对模块的操作**。基于这些操作，我们可以

- 动态的修改模块导出的内容（`module.exports`）
- 强制过期模块缓存，强制下次使用时重新加载（`require.cache`）

被依赖模块（`adder.js`）导出一个累加两数的函数：

``` javascript
// file adder.js
module.exports = function (a, b) {
    return a + b;
};
```

被测试模块（`add-one.js`）导出一个使用 `adder` 加一的函数：

``` javascript
// file add-one.js
const adder = require('./adder');

module.exports = function (num) {
    return adder(num, 1);
};
```

测试代码（`test.js`）模拟 `adder` 取相反数进行测试：

``` javascript
// file test.js
const _depModule = require('./adder');
const depCache = require.cache[require.resolve('./adder')];

before(function () {
    // Setup Stub
    depCache.exports = function (a, b) {
        return 0 - a - b;
    };
    // Force to reload next time
    delete require.cache[require.resolve('./add-one')];
});

describe('Test:', function () {
    it('should pass', function () {
        assert(require('./add-one')(5) == -6);
    });
});

after(function () {
    // Restore Stub
    depCache.exports = _depModule;
    // Force to reload next time
    delete require.cache[require.resolve('./add-one')];
});
```

当代码进入测试前、离开测试后，`adder` 仍是正常的两数相加函数。

### 非硬编码 - 依赖注入、依赖查找

对于非硬编码的依赖，情况就非常简单了：我们可以通过

- **传入** Stub 后的依赖实例（依赖注入)
- **替换** Stub 后的依赖资源，并在测试结束后 **恢复**（依赖查找）

分别实现灵活的 Stub。

## [no-toc] [no-number]

本文是我学习 Node 测试时 Stub 的 **个人理解**。对本文有什么问题，**欢迎斧正**。😉

This article is published under MIT License &copy; 2017, BOT Man
