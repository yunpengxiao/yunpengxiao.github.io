# Creational Patterns Notes

> Reading in 2017/7
>
> Favor object composition over class inheritance. —— GOF

[heading-numbering]

## [no-toc] [no-number] Related Notes

- Creational Patterns Notes
- [Structural Patterns Notes](Design-Patterns-Notes-2.md)
- [Behavioral Patterns Notes](Design-Patterns-Notes-3.md)

## [no-toc] [no-number] Covered Patterns

[TOC]

## Abstract Creation

### Abstract Factory

> Abstract creation of a **family of product**

#### [no-toc] [no-number] &sect; Roles

- **Abstract Factory** construct **Abstract Product**
- **Concrete Factory** construct **Concrete Product** family
- **Abstract Product** represent product interface
- **Concrete Product** represent a family of product

#### [no-toc] [no-number] &sect; Process

- **Client** call methods of **Abstract Factory**
- **Concrete Factory** create **Concrete Product** family
- **Client** retrieve **Abstract Product**

#### [no-toc] [no-number] &sect; Info Hidden

- **Abstract Factory** not know **Concrete Product** (not care)
- **Client** not know **Concrete Factory** and **Concrete Product**
  - (only use **Abstract Factory** to create **Abstract Product**)

#### [no-toc] [no-number] &sect; Uses

- Cross-platform solution
  - (different platform ~ different family of component)

#### [no-toc] [no-number] &sect; Comparison

- Implemented by
  - [sec|Factory Method] Factory Method (subclassing, compile-time)
  - [sec|Prototype] Prototype (object composition, runtime)

### Factory Method

> Subclass to **defer construction** of concrete product

#### [no-toc] [no-number] &sect; Roles

- **Abstract Creator** create **Abstract Product**
- **Concrete Creator** create **Concrete Product**
- **Abstract Product** represent product interface
- **Concrete Product** represent a family of product

#### [no-toc] [no-number] &sect; Process

- **Client** call methods of **Abstract Creator**
- **Abstract Creator** defer construction to **Concrete Creator**
- **Concrete Creator** create subclass-specific **Concrete Product**
- **Client** retrieve **Abstract Product**

#### [no-toc] [no-number] &sect; Info Hidden

- **Abstract Creator** not know **Concrete Product** (not care)
- **Client** not know **Concrete Creator** and **Concrete Product**
  - (only use **Abstract Creator** to create **Abstract Product**)

#### [no-toc] [no-number] &sect; Uses

- Create event handler
  - (different component create component-specific handler)
  - (different handler handle the same event on different component)
- Create iterator
  - (only aggregate itself know how to create its iterator)
  - (different iterator has the same interface for iteration)

#### [no-toc] [no-number] &sect; Comparison

- Different from _Static Factory Method_
- Used to implement [sec|Abstract Factory] Abstract Factory

### Prototype

> Prototype to **clone** new product

#### [no-toc] [no-number] &sect; Roles

- **Abstract Prototype** define interface to clone
- **Concrete Prototype** implement copy construction and initialization

#### [no-toc] [no-number] &sect; Process

- **Client** call methods of **Abstract Prototype**
- **Abstract Prototype** defer construction to **Concrete Prototype**
- **Concrete Prototype** clone itself and initialize the copy
- **Client** retrieve **Abstract Prototype**

#### [no-toc] [no-number] &sect; Info Hidden

- **Client** not know **Concrete Prototype**
  -  (only deal with interface of **Abstract Prototype**)

#### [no-toc] [no-number] &sect; Uses

- Palette
  - (clone prototype on the palette to create component)
  - (used to clone composited or decorated component)
- Registry with key
  - (clone prototype in manager to create component)
  - (used for dependency lookup)
- [C.130: For making deep copies of polymorphic classes prefer a virtual clone function instead of copy construction/assignment](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rh-copy)

#### [no-toc] [no-number] &sect; Comparison

- Used to implement [sec|Abstract Factory] Abstract Factory

### Builder

> Separate **construction procedure** from **product representation**

#### [no-toc] [no-number] &sect; Roles

- **Director** use the same construction procedure
- **Abstract Builder** define interface to accept build command
- **Concrete Builder** implement concrete build action
- **Product** have different representation

#### [no-toc] [no-number] &sect; Process

- **Client** create **Director** and **Concrete Builder**
- **Client** config **Director** with **Concrete Builder**
- **Director** call methods of **Abstract Builder**
- **Client** retrieve **Product** from **Concrete Builder**

#### [no-toc] [no-number] &sect; Info Hidden

- **Director** not know **Concrete Builder**
  - (only deal with interface of **Abstract Builder**)
- **Director** not know **Product**
  - (only **Client** care about **Product**)

#### [no-toc] [no-number] &sect; Uses

- Format conversion (the same source structure ~ different target)
- Parser (the same token flow ~ different abstract syntax tree)

## Maintain Instance

### Singleton

> Maintain **sole instance** at all places

#### [no-toc] [no-number] &sect; Roles

- **Singleton** define the interface to get instance (and set interface maybe)
- **Singleton Subclass** represent variation of product

#### [no-toc] [no-number] &sect; Process

- **Client** call methods of **Singleton**
- **Singleton** construct instance of **Singleton** at the first time
- **Client** retrieve instance of **Singleton** (or **Singleton Subclass**)

#### [no-toc] [no-number] &sect; Info Hidden

- **Client** not know **Singleton Subclass**
  - (retrieve **Singleton Subclass** from interface of **Singleton** class)

#### [no-toc] [no-number] &sect; Uses

- Resource manager/factory
  - (one place to manage distributed shared resource)
- Solution to _static-virtual member function_
  - (construct subclass according to initial config)

#### [no-toc] [no-number] &sect; Comparison

- vs. [&sect; Flyweight](Design-Patterns-Notes-2.md#Flyweight)
  - Used for mutable objects (non-const global)
