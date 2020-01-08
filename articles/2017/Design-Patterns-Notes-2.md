# Structural Patterns Notes

> Reading in 2017/8
>
> Program to an interface, not an implementation. —— GOF

[heading-numbering]

## [no-toc] [no-number] Related Notes

- [Creational Patterns Notes](Design-Patterns-Notes-1.md)
- Structural Patterns Notes
- [Behavioral Patterns Notes](Design-Patterns-Notes-3.md)

## [no-toc] [no-number] Covered Patterns

[TOC]

## Indirect Implementation

### Adapter

> Convert **existing interface** to **conform new interface**

#### [no-toc] [no-number] &sect; Roles

- **Target** provide interface for **Client**
- **Adaptee** expose existing interface (for **Target**)
- **Adapter** act as **Target** but defer request to **Adaptee** by
  - public inherit interface of **Target** (for **Client**)
  - **Adaptee** side:
    - private inherit implementation of **Adaptee**
    - maintain instance of **Adaptee**

#### [no-toc] [no-number] &sect; Process

- **Client** call methods of **Target** (on **Adapter**)
- **Adapter** defer request to **Adaptee**
- **Adaptee** do actual work

#### [no-toc] [no-number] &sect; Info Hidden

- **Client** not know **Adaptee**
  - (only deal with interface of **Target** / **Adapter**)
- **Target** and **Adaptee** not know **Adapter** / each other
  - (not exist in the same system)
  - (**Adapter** connect them)

#### [no-toc] [no-number] &sect; Uses

- Mix alternative library to new toolkit
  - (old library exposed as old interface)
- Model and view model in MVVM
  - (convert model to what view expect)

#### [no-toc] [no-number] &sect; Comparison

- vs. [sec|Bridge] Bridge
  - Used to adapt existing and incompatible interface
  - Not change/update existing implementation

### Bridge

> Decouple **abstraction** from **implementation**

#### [no-toc] [no-number] &sect; Roles

- **Abstraction** provide interface for **Client**
- **Abstract Implementor** provide interface for **Abstraction**
- **Concrete Implementor** implement specific operation

#### [no-toc] [no-number] &sect; Process

- **Client** call methods of **Abstraction**
- **Abstraction** defer request to **Abstract Implementor**
- **Concrete Implementor** do specific operation

#### [no-toc] [no-number] &sect; Info Hidden

- **Client** not know **Concrete Implementor**
  - (only deal with interface of **Abstraction**)
- **Abstraction** not know **Concrete Implementor**
  - (only deal with interface of **Abstract Implementor**)
- **Implementor** not know **Abstraction** (not care)

#### [no-toc] [no-number] &sect; Uses

- Cross-platform solution
  - (the same abstraction component ~ platform-specific implementation)
- Select optimal implementation at runtime
  - (determine implementation according to current environment)

#### [no-toc] [no-number] &sect; Comparison

- vs. [sec|Adapter] Adapter
  - Provide stable abstraction interface
  - Vary evoluating implementation independently

### Facade

> **Encapsulate subsystem** to provide **high-level interface**

#### [no-toc] [no-number] &sect; Roles

- **Facade** provide interface for **Client**
- **Subsystem** implement bussiness logic

#### [no-toc] [no-number] &sect; Process

- **Client** call methods of **Facade**
- **Facade** defer request to **Subsystem**

#### [no-toc] [no-number] &sect; Info Hidden

- **Subsystem** not know **Facade** (not care)
- **Client** not know **Subsystem** (not care)

#### [no-toc] [no-number] &sect; Uses

- Compiler's compile interface (hide lexer/parser detail)
- Large system (define multi-layered facade inside)

## Composition

### Composite

> Define **uniform interface** for **part-whole component**

#### [no-toc] [no-number] &sect; Roles

- **Component** provide interface for both **Primitive** and **Composite**
- **Primitive** define leaf component
- **Composite** define composition of children **Component**

#### [no-toc] [no-number] &sect; Process

- **Client** call methods of **Component**
- **Composite** defer request to children **Component**
  (and perform additional operation)
- **Primitive** handle request directly

#### [no-toc] [no-number] &sect; Info Hidden

- **Client** not know **Primitive** and **Composite**
  - (only deal with interface of **Component**)
- **Component** may know interface of **Composite**
  - (when **Component** refer to **Composite** as parent)
- **Primitive** and **Composite** not know each other (parallel)

#### [no-toc] [no-number] &sect; Uses

- GUI component (treat composite and primitive uniformly)
- Task composition (one task can contain others)
- Define abstract syntax tree
  - (different gramma component share the same interface)
  - (base of [&sect; Interpreter](Design-Patterns-Notes-3.md#Interpreter))

#### [no-toc] [no-number] &sect; Comparison

- vs. [sec|Decorator] Decorator
  - Focus on object aggregation for representation

### Decorator

> **Attach responsiblity** without changing interface

#### [no-toc] [no-number] &sect; Roles

- **Abstract Component** provide interface for **Client**
- **Concrete Component** define (leaf) component entity
- **Decorator**
  - conform to interface of **Abstract Component**
  - attach additional responsibility to its **Component**

#### [no-toc] [no-number] &sect; Process

- **Client** establish concrete decorated **Component**
- **Client** call methods of **Abstract Component**
- **Concrete Decorator** defer request to child **Abstract Component**
  and perform additional operation
- **Concrete Component** handle request as normal

#### [no-toc] [no-number] &sect; Info Hidden

- **Client** not know **Concrete Component** and **Decorator**
  - (only deal with interface of **Abstract Component**)
- **Concrete Component** and **Decorator** not know each other (parallel)

#### [no-toc] [no-number] &sect; Uses

- GUI component (add border and scrollbar to a view)
- Pipeline operation (encode/encrypt stream before writing to file)

#### [no-toc] [no-number] &sect; Comparison

- vs. [sec|Composite] Composite
  - Focus on attaching responsibility
  - Degenerate composition (only one child component)
- vs. [sec|Proxy] Proxy
  - Focus on adding responsibility dynamically and recursively

### Proxy

> **Intercept access** without changing interface

#### [no-toc] [no-number] &sect; Roles

- **Abstract Subject** provide interface for **Client**
- **Concrete Subject** define concrete object
- **Proxy**
  - conform to interface of **Abstract Subject**
  - intercept request to **Concrete Subject**

#### [no-toc] [no-number] &sect; Process

- **Client** establish concrete proxified **Subject**
- **Client** call methods of **Abstract Subject**
- **Proxy** defer request to another **Abstract Subject** under control
- **Concrete Subject** handle request as normal

#### [no-toc] [no-number] &sect; Info Hidden

- **Client** not know **Concrete Subject** and **Proxy**
  - (only deal with interface of **Abstract Subject**)
- **Concrete Subject** not know **Proxy**
  - (**Proxy** act as man-in-middle and control the request traffic)
- **Proxy** may not know **Concrete Subject**
  - (may only deal with interface of **Abstract Subject**)

#### [no-toc] [no-number] &sect; Uses

- Remote proxy (**not** keep reference to **Concrete Subject**)
  - Remote calling
- Virtual proxy (**may** keep reference to **Concrete Subject**)
  - Lazy loading
  - Copy on write
- Protection proxy (**do** keep reference to **Concrete Subject**)
  - Access control
  - Exclusive lock
- Resource management (**do** keep reference to **Concrete Subject**)
  - Smart pointer
  - Lock guard

#### [no-toc] [no-number] &sect; Comparison

- vs. [sec|Decorator] Decorator
  - Focus on controlling access non-dynamically

## Maintain Instance

### Flyweight

> **Share** substantial **fine-grained objects**

#### [no-toc] [no-number] &sect; Roles

- **Abstract Flyweight** define interface to accept extrinsic state
- **Concrete Flyweight** store intrinsic state to be sharable
- **Flyweight Factory** create and manage **Flyweight** instance

#### [no-toc] [no-number] &sect; Process

- **Client** call methods of **Flyweight Factory**
- **Flyweight Factory** construct instance of **Concrete Flyweight** firstly
- **Client** retrieve instance of **Abstract Flyweight**
- **Client** pass extrinsic state to **Abstract Flyweight** to do work
- **Concrete Flyweight** use both extrinsic and intrinsic state to do work

#### [no-toc] [no-number] &sect; Info Hidden

- **Flyweight Factory** not know **Concrete Flyweight** (not care)
- **Client** not know **Concrete Flyweight**
  - (only deal with interface of **Abstract Flyweight**)
  - (only retrieve object by key)

#### [no-toc] [no-number] &sect; Uses

- GUI component (sharing font and style bitmap)
- Manage [&sect; State](Design-Patterns-Notes-3.md#State) /
  [&sect; Strategy](Design-Patterns-Notes-3.md#Strategy) object
  (retrieve by key)

#### [no-toc] [no-number] &sect; Comparison

- vs. [&sect; Singleton](Design-Patterns-Notes-1.md#Singleton)
  - Used for immutable objects (const global)
