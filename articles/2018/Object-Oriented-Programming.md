# 浅谈面向对象编程

> 2018/3/24
>
> Code is by people and for people. —— Kent Beck

[heading-numbering]

## [no-toc] [no-number] TOC

[TOC]

## 写在前面

### 很多人不关心代码可读性

对于很多人，包括我，最开始写代码的时候，对代码的认识就是：写一些特殊的符号，让机器去执行，从而达到效果。这时候，我们关心的是：**机器** 如何 **执行** 我们写的代码。

例如，我曾经会设计这样的函数 —— 在一个循环内完成两件事：一边实现某个操作，一边统计结果并返回。

[code||pet-and-count-cats]

``` csharp
int PetAndCountCats (List<Pet> pets) {
    int count = 0;
    for (var pet in pets) {
        if (pet.GetType () == Pet.CAT) {
            pet.Pet ();   // Pet cats
            ++count;      // Count cats
        }
    }
    return count;
}
```

[align-center]

代码 [code|pet-and-count-cats] - 一个循环内完成两件事

随着代码写的越来越多，我们发现：一般 **读代码** 的时间比 **写代码** 的时间要长。所以，我们在写代码的时候开始关心：如何通过代码来 **传达** 我们的 **想法**。

对于上边的例子，[把查询和修改分开 (Separate Query from Modifier)](Refactoring-Notes.md#Separate-Query-from-Modifier-P279)，变为 `PetCats` 和 `CountCats`，更有利于传达我们的想法 —— 因为一般 **有返回值** 的方法只作为 **查询**，没有副作用；**没有返回值** 的方法作为 **修改**，才会产生副作用。（见 [笔记](Implementation-Patterns-Notes.md#Return-Type-P84)）

### 很多人用着面向对象语言，却只写着面向过程程序

此外，很多人，包括我，最开始学习的编程语言是 **面向过程语言**，在使用面向对象语言编写程序的时候，仍然只用着最简单的控制流 —— 把所有的逻辑写成一个 **超长的函数**，而且只用了 **条件、循环** 实现相应功能。

而面向对象语言引入了 **类和对象**，把对外表现的 **行为** _(behavior)_ 和用于支持行为的 **状态** _(state)_ 封装在一起，实现功能的模块化。（见 [笔记](Implementation-Patterns-Notes.md#Advantage-of-Object-P43)）

同时，面向对象语言为我们提供了 **丰富的控制流** 描述方法，更符合人们直观的想法 —— 顺序 _(sequence)_，消息 _(messages)_，迭代 _(iteration)_，条件 _(conditionals)_，异常 _(exceptions)_。（见 [笔记](Implementation-Patterns-Notes.md#Usage-Order-of-Control-Flow-P72)）

### 为什么要写这篇文章

- 2018 年，读了 Martin Fowler 的[《重构》](../2018/Refactoring-Notes.md)、Kent Beck 的[《实现模式》](../2018/Implementation-Patterns-Notes.md)，深有 **感触**
- **分享** 给大家，并收集 **反馈**

### 从一个例子开始

> 正如 Martin Fowler 所说，抽象的原理不如具体的例子更能说明问题。（见 [笔记](Refactoring-Notes.md#How-to-begin-to-write-or-talk-at-a-conference-P1)）

假设我们在给一个桌面宠物游戏加入一些功能：

- 支持两个玩家参与游戏
- 每个玩家可以养多只宠物（猫🐱 / 狗🐶）
- 每个玩家可以给自己或对方的宠物喂食、洗澡
- 需要分别统计猫和狗的数量

根据直观的 **面向过程** 的想法（虽然使用了 C++ 的 STL），我们可以：

- 定义一组变量，用于存放每个玩家的宠物、猫/狗粮和洗发水
- 定义一组函数，用于统计猫/狗数量、给宠物喂食、给宠物洗澡

[code||imperative]

``` cpp
std::vector<Pet> pets_of_keeper1, pets_of_keeper2;
PetFood pet_food_of_keeper1, pet_food_of_keeper2;
Shampoo shampoo_of_keeper1, shampoo_of_keeper2;

int CountCats (std::vector<Pet> pets);
int CountDogs (std::vector<Pet> pets);
void FeedPets (std::vector<Pet> &pets, PetFood &pet_food);
void CleanPets (std::vector<Pet> &pets, Shampoo &shampoo);
```

[align-center]

代码 [code|imperative] - 面向过程

本文将围绕这个例子，从封装、继承、多态，介绍如何使用面向对象的方法实现上述功能。

## 封装 Encapsulation

面向对象语言引入了 **类和对象**，可以把一组密切相关的 **逻辑** _(logic)_ 和对应的 **数据** _(data)_ 放在一起，从而实现模块化。（见 [笔记](Implementation-Patterns-Notes.md#Coding-Priciples-P13-17)）

放在同一个类里的逻辑和数据，往往解决了 **同一领域** 的问题，**变化频率一致** —— 理想情况下，当这个领域问题变化时，只需要修改对应的这个类，就可以实现所有的更改。（见 [笔记](Refactoring-Notes.md#About-Changes-P80)）

封装是面向对象编程的核心，有两个重要的优势（见 [笔记](Refactoring-Notes.md#About-Encapsulation-P157)）：

- 对象只需要 **更少** 的 **“了解”** 系统的其他部分，减少对象以外的依赖
- 当领域问题变化时，只有 **少数** 几个对象需要 **修改**，就可以实现这个变化

### 例子：状态/数据的封装

代码 [code|imperative] 中，为了表示 3 类不同的状态，使用 `*_of_keeper1` 和 `*_of_keeper2` 构成了 2 组 **平行的、独立的** 数据。很容易发现，每一组数据都可以捆绑在一起，组成 `struct PetKeeper`；利用捆绑后的结构，可以很方便的定义多组平行的、结构相同的数据 `keeper1`, `keeper2`。从而实现对状态/数据的封装。

[code||encapsulate-data]

``` cpp
struct PetKeeper {
    std::vector<Pet> pets;
    PetFood pet_food;
    Shampoo shampoo;
};

PetKeeper keeper1, keeper2;
```

[align-center]

代码 [code|encapsulate-data] - 封装状态/数据

### 例子：行为/逻辑的封装

进一步的，代码 [code|imperative] 中的 4 个函数是对 `struct PetKeeper` 数据进行操作的逻辑，解决了 **同一领域的问题、变化频率一致**。所以，在代码 [code|encapsulate-data] 的基础上，把这些逻辑和数据封装在一起，构成了完整的类 `class PetKeeper`。从而实现对行为/逻辑的封装。

[code||encapsulate-logic]

``` cpp
class PetKeeper {
    std::vector<Pet> pets;
    PetFood pet_food;
    Shampoo shampoo;
public:
    int CountCats () const;
    int CountDogs () const;
    void FeedPets ();
    void CleanPets ();
};
```

[align-center]

代码 [code|encapsulate-logic] - 封装行为/逻辑

## 继承 Inheritance

在面向对象设计中，继承常用于对领域模型进行 **分类**（例如 `Cat` / `Dog` 都属于 `Pet`）。而在面向对象编程中，继承提供了实现 **共享逻辑** 的最简单方法（子类可以使用父类的 `public/protected` 字段/方法）。

但是，Alan Snyder 在 _Encapsulation and inheritance in object-oriented languages_ 一文中，提出了 **继承破坏封装** _(inheritance breaks encapsulation)_ 的观点 —— 子类使用了父类的 `public/protected` 字段/方法，父类的被依赖部分一旦发生改变，子类也被迫需要修改。

所以，人们常说应该 **面向接口编程，而不是面向实现** _(code to interfaces, not implementations)_（见 [笔记](Implementation-Patterns-Notes.md#Old-Adage-in-Software-Development-P24)）—— 子类尽可能避免过度依赖于父类的实现，尤其是两者变化不可控的时候。（在 Java/C# 语言中，专门引入了 `interface` 的概念）

### 例子：继承实现

为了避免重复，我们可以把两个类的 **共享逻辑** 抽出到一个父类里，然后这两个类继承于该父类。例如，猫和狗吃东西的逻辑有共同之处 —— 吃了食物后，需要一段时间消化食物，然后饥饿值下降。消化食物的逻辑 `DigestAfterEating`、饥饿值的状态 `hunger_points` 是猫和狗共有的，可以提到父类 `Pet` 里，从而避免分别在 `Cat` 和 `Dog` 里重复代码。

[code||inherit-implementation]

``` cpp
class Pet {
    // ...
protected:
    int hunger_points;
    void DigestAfterEating ();
};

class Cat : public Pet {
    // ...
public:
    void Eat (PetFood &pet_food) {
        // cat specific logic
        DigestAfterEating ();  // shared logic
        --hunger_points;       // shared data
    }
};

class Dog : public Pet {
    // similar to Cat
};
```

[align-center]

代码 [code|inherit-implementation] - 继承实现

但是，代码 [code|inherit-implementation] 中的 `Pet` 一旦有修改，`Cat` 和 `Dog` 可能会被动的受到这个修改的困扰（例如 `Pet::DigestAfterEating` 要求传入参数，`Cat::Eat` 和 `Dog::Eat` 需要修改为传入参数的形式）。

## 多态 Polymorphism

多态，很难顾名思义，是指通过 **重写函数** _(overriding)_ / **实现接口** _(implementing)_ 的方式，让同一个 **消息** _(messages)_ 实现不同的逻辑。

使用消息控制流，是面向对象编程的一个重要特性：

- 两个对象之间用消息调用，能更丰富的表达我们的想法
- 基于多态，消息对未来不可预见的扩展是开放的 —— 改变消息的接收者，不需要改变消息的发送者 —— **发送者** 只关心消息的 **意图**，而 **接收者** 才需要关心消息的 **实现**（见 [笔记](Implementation-Patterns-Notes.md#Intention-and-Implementation-P69)）

利用多态，我们可以更灵活的设计逻辑策略的切换（见 [笔记](Implementation-Patterns-Notes.md#Design-Switch-between-Strategies-P34)）：

- 对于面向过程语言，我们只能通过 **条件** _(conditionals)_ 实现逻辑的切换
- 基于多态，我们可以通过 **派生** _(subclasses)_ 和 **委托** _(delegation)_ 实现
  - 派生一般通过继承实现；而委托一般通过组合实现
  - 派生切换的逻辑，在对象生命周期内不能再次改变；而委托可以多次修改

### 例子：条件逻辑切换

很多人，包括我，喜欢使用面向过程语言里的 **测试条件** _(testing conditionals)_ 实现逻辑策略切换。例如，在实现喂食功能 `PetKeeper::FeedPets`，而对于不同的动物有着不同的喂食逻辑时，我们会使用 `if/switch` 语句先判断 `pet` 的类型，然后针对不同类型进行处理。

[code||conditionals]

``` cpp
void PetKeeper::FeedPets () {
    for (Pet &pet : pets) {
        if (pet.GetType () == Pet.CAT) {
            // cat eating pet_food logic
        }
        else if (pet.GetType () == Pet.DOG) {
            // dog eating pet_food logic
        }
    }
}
```

[align-center]

代码 [code|conditionals] - 条件逻辑切换

这段代码并不是一个良好的设计：

- 在函数中，`PetKeeper` 作为消息的发送者，`Pet` 是消息的接收者 —— `PetKeeper` 操作 `Pet` 的对象实现喂食逻辑
- 破坏了封装性
  - 消息的 **发送者** 不仅需要关心消息的意图，还需要关心消息处理逻辑的 **实现**
  - 一个类 `PetKeeper` **过度访问** 另一个类 `Pet` 的数据/实现，在 `PetKeeper` 里实现了应该在 `Pet` 里实现的功能，类的职责划分不恰当，是重构的一个信号（见 [笔记](Implementation-Patterns-Notes.md#Object-Design-Problem-P47)）
- 不易于扩展
  - 当我们需要引入一个新的宠物类型（例如，兔子🐰）的时候，就需要 **修改** 消息的 **发送者** `PetKeeper` 的 `PetKeeper::FeedPets` 实现，即加入 `if (pet.GetType () == Pet.RABBIT)` 分支

### 例子：派生逻辑切换

利用面向对象的方法，我们可以实现基于多态的派生逻辑切换。例如，同样是实现喂食功能 `PetKeeper::FeedPets`，我们只需要给 `Pet` 定义一个统一的接口 `Eat`，接收 `PetKeeper` 发送的消息；对于不同动物的不同逻辑，我们可以通过 **重写函数** _(overriding)_ / **实现接口** _(implementing)_ 实现。

[code||subclasses]

``` cpp
class Pet {
    // ...
public:
    virtual void Eat (PetFood &pet_food) = 0;
};

class Cat : public Pet {
    // ...
public:
    void Eat (PetFood &pet_food) override {
        // cat specific logic
    }
};

void PetKeeper::FeedPets () {
    for (Pet &pet : pets) {
        pet.Eat (pet_food);  // only care intention
    }
}
```

[align-center]

代码 [code|subclasses] - 派生逻辑切换

相对于代码 [code|conditionals]，这个设计有着极大的优势：

- 在函数中，`PetKeeper` 作为消息的发送者，`Pet` 是消息的接收者 —— `PetKeeper` 操作 `Pet` 的对象实现喂食逻辑
- 有良好的封装性
  - 消息的 **发送者只需要关心消息**（简单的 `Eat` 接口）的意图，不需要关心处理逻辑的实现
  - 消息的 **接收者只负责处理消息**，和发送者没有过多的耦合，尽职尽责
- 易于扩展
  - 当我们需要引入一个新的宠物类型（例如，兔子🐰）的时候，只需要在 `Rabbit` 类里实现 `Eat` 接口即可；消息 **发送者** 代码 **不需要修改**

### 例子：委托逻辑切换

类似于代码 [code|subclasses]，把 `Pet::Eat` 委托到 `PetImpl::EatImpl` 函数：

- 对于`PetKeeper::FeedPets` 的消息调用是透明的，和派生相比，仅仅是 **实现上** 的不同
- 相对于派生实现的优势在于，可以在 `Pet` 对象 **生命周期内切换逻辑** —— 将变量 `pet_impl` 赋值为 `CatImpl` 对象时，可以实现猫的逻辑；赋值为 `DogImpl` 对象时，就可以动态切换为狗的逻辑（虽然在这个业务场景下没有实际意义，猫生出来不会变成狗。。。）

[code||delegation]

``` cpp
class PetImpl {
public:
    virtual void EatImpl (PetFood &pet_food) = 0;
};

class CatImpl : public PetImpl {
    // ...
public:
    void EatImpl (PetFood &pet_food) override {
        // cat specific logic
    }
};

class Pet {
    // ...
    PetImpl *pet_impl;
public:
    void Eat (PetFood &pet_food) {
        pet_impl->EatImpl (pet_food);
    }
};
```

[align-center]

代码 [code|delegation] - 委托逻辑切换

## 写在最后 [no-number]

由于经验、篇幅、时间限制，更多关于面向对象编程的问题讨论，可参考 Martin Fowler 的[《重构》](../2018/Refactoring-Notes.md)和 Kent Beck 的[《实现模式》](../2018/Implementation-Patterns-Notes.md)。

如果有什么问题，**欢迎交流**。😄

Delivered under MIT License &copy; 2018, BOT Man
