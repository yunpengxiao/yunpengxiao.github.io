# Refactoring Notes

> Reading in 2017/10 - 2018/2
>
> Digest of _Refactoring_, Martin Fowler

## [no-toc] TOC

[TOC]

### How to begin to write (or talk at a conference) P1

- With **principles** it is too easy to make **generalizations**, too hard to figure out how to apply things.
- An **example** helps make things **clear**.

### Refactor before Adding Feature (two-hats) P7

- When you find you have to add a feature to a program, and the program's code is not structured in a convenient way to add the feature,
- **first** refactor the program to make it easy to add the feature,
- **then** add the feature.

### Test before Refactoring P8

- Before you start refactoring, check that you have a **solid suite of tests**.
- These tests must be **self-checking**.

### Refactor in Small Steps P13

- Refactoring changes the programs in small steps.
- If you make a mistake, it is **easy to find** the **bug**.

### Readable P15

- Any **fool** can write code that a **computer** can **understand**.
- **Good** programmers write code that **humans** can **understand**.

### Programmers are Lazy P56

- Try **not remember** **anything** I **can look up**,
- because I'm afraid my **brain** will get **full**.

### Programmers' Good Habits P57

I'm not a great programmer; I'm just a good programmer with great habits. —— Kent Beck

### Indirection in Computer Science P61

Computer Science is the discipline that believes all problems can be solved with one more layer of indirection. —— Dennis DeBruler

### Upfront Design P67

With design I can think very fast, but my thinking is full of little holes. —— Alistair Cockburn

### Measure Performance P69

Even if you know exactly what is going on in your system, measure performance, don't speculate. —— Ron Jeffries

### Good Performance P69

The secret to fast software, in all but hard real-time contexts, is to **write tunable** software first and **then to tune** it for sufficient speed.

### Semantic Distance P77

- A heuristic we follow is that whenever we feel the need to comment something, we write a method instead...
- The key here is
  - **not method length**
  - but the semantic distance between **what** the method **does** and **how** it **does** it.

### About Changes P80

- Divergent change is **one class** that suffers **many** kinds of **changes**
- Shotgun surgery is **one change** that alters **many classes**
- Ideally, there is a **one-to-one** link between common changes and classes

### About Comment P88

When you feel the need to write a comment, first try to refactor the code so that any comment becomes superfluous.

### How Most Programmers Spend Their Time P89

- writing code -> small fraction of time
- figuring out what ought to be going on -> some time
- designing -> some time
- debugging -> most time

### About Boldface Highlight P105

Too much defeats the purpose.

### Design Patterns and Refactoring P107

Design Patterns... provide **targets** for your refactorings. —— Gang of Four

### About Responsibility P141

- One of the most fundamental decision in object design is deciding **where to put responsibilities**.
- I've been working with objects for more than a decade, but I still **never get it right** the **first** time.
- That used to bother me, but now I realize that I can **use refactoring** to **change my mind** in these cases.

### About Encapsulation P157

- One of the **keys to objects** is encapsulation
- Objects need to **know less** about other parts of the system
- When things change, **fewer** objects need to be told about the **change**

### About Repetition P162

Repetition is the root of all software evil.

### Mutable vs. Immutable P179/P183

- **Reference object** (mutable/class in C#/aggregation)
  - stands for one object in the real world
  - use the object identity to test equality
  - like customer or account
- **Value object** (immutable/struct in C#/composition)
  - defined entirely through their data values
  - need to override the equals/hashCode method to tell equality
  - like date or money

### Maintain Unidirectional association P198

Let **one class take charge** because it keeps all the logic for manipulating the association in **one place**:

1. If both objects are reference objects and the association is one to many, then the object that has the one reference is the controller. (That is, if one customer has many orders, the order controls the association.)
2. If one object is a component of the other, the composite should control the association.
3. If both objects are reference objects and the association is many to many, it doesn't matter whether the order or the customer controls the association.

### About Naming P273

Remember your code is for a **human first** and a **computer second**.

### Separate Query from Modifier P279

A good rule to follow is to say that any method that **returns** a value should **not** have observable **side effects**.

### About Research P381

If someone (a reviewer of a paper, an attendee at a talk) comments, "I don't understand" or just doesn't get it, it's **our fault**. It is our responsibility to work hard to develop and communicate our ideas. —— William Opdyke (Ralph Johnson)
