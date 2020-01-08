# Behavioral Patterns Notes

> Reading in 2017/9
>
> Encapsulate the concept that varies. —— GOF

[heading-numbering]

## [no-toc] [no-number] Related Notes

- [Creational Patterns Notes](Design-Patterns-Notes-1.md)
- [Structural Patterns Notes](Design-Patterns-Notes-2.md)
- Behavioral Patterns Notes

## [no-toc] [no-number] Covered Patterns

[TOC]

## Decouple Sender-Receiver

### Command

> Decouple **sender** from **receiver** by encapsulating invocation

#### [no-toc] [no-number] &sect; Roles

- **Abstract Command** define request handling interface
- **Concrete Command** refer to **Receiver** and corresponding action
- **Receiver** implement action to handle request
- **Sender** issue request (async with **Client**)

#### [no-toc] [no-number] &sect; Process

- **Client** create **Concrete Command** with **Receiver**
- **Client** config **Sender** with **Concrete Command**
- **Sender** send request to **Abstract Command** (async)
- **Concrete Command** defer request to **Receiver**

#### [no-toc] [no-number] &sect; Info Hidden

- **Abstract Command** not know **Sender**
  - (not care about who issue the request)
- **Sender** not know **Concrete Command**
  - (only deal with interface of **Abstract Command**)
- **Sender** not know **Receiver**
  - (indirected by and only communicate with **Command**)
- **Receiver** not know **Sender** and **Command** (not care)

#### [no-toc] [no-number] &sect; Uses

- Support undoable operation (with unexecute interface)
- Support logging operations (able to recreate system crash)
- Support transaction (rollback at failing)
- Enrich raw callback function (like lambda and functor in C++)

#### [no-toc] [no-number] &sect; Comparison

- **Sender** not know how to handle request -> defer to **Command**
- **Command** indirect request of **Sender** to **Receiver**

### Chain of Responsibility

> Decouple **receiver** from **actual handler** by chaining potential handler

#### [no-toc] [no-number] &sect; Roles

- **Abstract Handler** define request handling interface
- **Concrete Handler** handle request or forward to successor
- **Receiver** issue request (async with **Client**)

#### [no-toc] [no-number] &sect; Process

- **Client** config **Receiver** with **Concrete Handler**
- **Receiver** send request to **Abstract Handler** (async)
- **Concrete Handler**
  - handle request if able to handle
  - defer request to successor if unable to handle

#### [no-toc] [no-number] &sect; Info Hidden

- **Abstract Handler** not know **Receiver**
  - (not care about who issue the request)
- **Receiver** not know **Concrete Handler**
  - (only deal with interface of **Abstract Handler**)
- **Receiver** not know who actually handle the request
  - (request may be forwarded to the end of the chain)
- **Concrete Handler** not know its successor
  - (simply forward to its successor if necessary)

#### [no-toc] [no-number] &sect; Uses

- Handling user event (only handle what it can, and forward what it can't)
- Handling redraw event (only draw what is in its context)

#### [no-toc] [no-number] &sect; Comparison

- **Receiver** not know how to handle request -> defer to **Handler**
- **Handler** (in a chain)
  - defer to successor **Handler** if not know how to handle request
  - handle request if know how to handle request

### Observer

> Decouple **receiver** from **multi handler** by broadcasting notification

#### [no-toc] [no-number] &sect; Roles

- **Abstract Subject** provide interface to attach/detach **Observer**
- **Abstract Observer** provide interface to get notified from **Subject**
- **Concrete Subject** publish to **Observer** (async with **Client**)
- **Concrete Observer** subscribe to and interact with **Subject**

#### [no-toc] [no-number] &sect; Process

- **Client** attach **Abstract Observer** to **Abstract Subject**
- **Concrete Subject** send request to **Abstract Observer** (async)
- **Concrete Observer** get and (handle or ignore) notification through
  - (Pull Model) querying state of **Concrete Subject**
  - (Push Model) retrieving argument passed by **Concrete Subject**

#### [no-toc] [no-number] &sect; Info Hidden

- **Observer** not know **Subject**
  - (not care about who issue the request)
- **Subject** not know **Concrete Observer**
  - (only deal with interface of **Abstract Observer**)
- **Concrete Observer** refer to **Concrete Subject** in _Pull Model_

#### [no-toc] [no-number] &sect; Uses

- Model and view in MVC architecture (model changes notify view)

#### [no-toc] [no-number] &sect; Comparison

- **Subject** be **Receiver** while **Observer** be **Handler**
- **Subject** not know how to handle request -> defer to **Observer**

### Mediator

> Decouple **communication between receivers** by centralization

#### [no-toc] [no-number] &sect; Roles

- **Abstract Mediator** define interface to receive **Colleague** notification
- **Concrete Mediator** establish communication between **Colleague**
- **Colleague** interact with **Abstract Mediator** (async with **Client**)

#### [no-toc] [no-number] &sect; Process

- **Client** create **Concrete Mediator** with **Colleague**
- **Colleage** send request to **Abstract Mediator** (async)
- **Concrete Mediator** handle request and send response to **Colleage**

#### [no-toc] [no-number] &sect; Info Hidden

- **Abstract Mediator** not know **Colleague**
  - (not care about who issue the request)
- **Colleague** not know **Concrete Mediator**
  - (only deal with interface of **Abstract Mediator**)
- **Concrete Mediator** know all **Colleague**
  - (coordinate all kinds of component to work together)

#### [no-toc] [no-number] &sect; Uses

- Widget-Pane in GUI
  - (Widget be **Colleague** while Pane be **Mediator**)
  - (Pane: create widgets -> show component -> handle interaction)
- Message center in subscribe-publish system
  - (provide acyclic callback ensurance)
  - (clarify dependency between components)
- Decouple cyclic dependency in [sec|Observer] Observer
  - (introduce **Mediator** to maintain dependency)
  - (dependent objects be **Subjects** while **Mediator** be **Observer**)

#### [no-toc] [no-number] &sect; Comparison

- **Colleague** be **Receiver** while **Mediator** be **Handler**
- **Colleague** not know how to handle request -> defer to **Mediator**

## Encapsulate Behavior

### State

> Encapsulate **state-specific behavior** to **alter at runtime**

#### [no-toc] [no-number] &sect; Roles

- **Context** provide interface for **Client** and maintain instance
- **Abstract State** provide all behavior interface for **Context**
- **Concrete State** implement state-specific behavior

#### [no-toc] [no-number] &sect; Process

- **Client** initialize **Context** with **Concrete State**
- **Client** call methods of **Context**
- **Context** defer request to **Abstract State**
- **Concrete State** handle request and access **Context**
- State Transition
  - **Context** handle transition uniformly
  - **Concrete State** update **State** instance of **Context**

#### [no-toc] [no-number] &sect; Info Hidden

- **Context** is friend of **Abstract State**
  - (access representation)
  - (**State** also access to **Context** for data)
- **Context** not know **Concrete State**
  - (only deal with interface of **Abstract State**)

#### [no-toc] [no-number] &sect; Uses

- Simplify conditional statement (delegation rather than state checking)
- Implement automaton (behavior changes along with state changes)
- Polymorphic tool (different behavior for different state, like magic pen)

#### [no-toc] [no-number] &sect; Comparison

- vs. [sec|Strategy] Strategy
  - **Context** defer whole request to **State**
  - Provide several methods (series of behavior)
  - Trigger transition to following state
  - like [&sect; Abstract Factory](Design-Patterns-Notes-1.md#Abstract-Factory)

### Strategy

> Encapsulate a **family of algorithm** with similar behavior

#### [no-toc] [no-number] &sect; Roles

- **Context** provide interface for **Client** and maintain instance
- **Abstract Strategy** provide common functionality interface for **Context**
- **Concrete Strategy** implement algorithm behavior

#### [no-toc] [no-number] &sect; Process

- **Client** config **Context** with **Concrete Strategy**
- **Client** call methods of **Context**
- **Context** delegate algorithm related behavior to **Abstract Strategy**

#### [no-toc] [no-number] &sect; Info Hidden

- **Context** not know **Concrete Strategy**
  - (only deal with interface of **Abstract Strategy**)
- **Strategy** not know **Context**
  - (only know information algorithm needed)

#### [no-toc] [no-number] &sect; Uses

- Simplify conditional statement (delegation rather than state checking)
- Strategy (only part of behavior) vary frequently (dependency injection)

#### [no-toc] [no-number] &sect; Comparison

- vs. [sec|State] State
  - **Context** defer only algorithm behavior to **State**
  - Provide single method (single responsibility)
  - Updated by others after construction
  - like [&sect; Factory Method](Design-Patterns-Notes-1.md#Factory-Method)
- vs. [sec|Template Method] Template Method
  - Use delegation to inverse control (IoC)

### Template Method

> Abstract **algorithm's skeleton** and **subclass to override primitive**

#### [no-toc] [no-number] &sect; Roles

- **Abstract Class** define
  - public non-virtual _template method_
  - protected [pure] virtual _primitive operation_
- **Concrete Class** override _primitive operation_

#### [no-toc] [no-number] &sect; Process

- **Client** call methods of **Abstract Class**
- **Concrete Class** do specific work by overriding

#### [no-toc] [no-number] &sect; Info Hidden

- **Client** not know _primitive operation_

#### [no-toc] [no-number] &sect; Uses

- Framework (let user to override only _primitive operation_)

#### [no-toc] [no-number] &sect; Comparison

- vs. [sec|Strategy] Strategy
  - Use inheritance to inverse control (IoC)

## Traverse Aggregate

- Aggregate be
  - collection (uniform-typed, list, array...)
  - composite (abstract-typed,
    [&sect; Composite](Design-Patterns-Notes-2.md#Composite))
  - tuple (product-typed)

### Iterator

> Decouple **traversal** on aggregate from its **representation**

#### [no-toc] [no-number] &sect; Roles

- **Aggregate** define interface to create iterator
  ([&sect; Factory Method](Design-Patterns-Notes-1.md#Factory-Method))
- **Iterator** define basic iteration operations
  - External iterator (First / Next / IsDone / GetItem)
  - Internal iterator (IoC pattern)

#### [no-toc] [no-number] &sect; Process

- **Client** create **Iterator** of **Aggregate**
- **Client** call methods of **Iterator**
- **Iterator** keep track of iteration and compute next position

#### [no-toc] [no-number] &sect; Info Hidden

- **Iterator** is friend of **Aggregate** (access representation)
- **Aggregate** not know **Iterator** (not care)
- **Client** not know representation of **Aggregate** / **Iterator**
  - (only treat them as concrete object)

#### [no-toc] [no-number] &sect; Uses

- Sequential access of collection (STL container)
- Yield operation for I/O (co-routine)

#### [no-toc] [no-number] &sect; Comparison

- vs. [sec|Visitor] Visitor
  - Provide external/internal iteration over _collection_
- Internal iterator implemented by
  - [sec|Strategy] Strategy (pass strategy into traverse method)
  - [sec|Template Method] Template Method (override process item method)

### Visitor

> Decouple **element of aggregate** from its **operation**

#### [no-toc] [no-number] &sect; Roles

- **Abstract Visitor** provide interface to visit all **Concrete Element**
- **Concrete Visitor** implement operation on visiting **Concrete Element**
- **Abstract Element** provide interface to accept **Visitor**
- **Concrete Element** pass this to **Visitor** on acception
- **Aggregate** provide interface to accept **Visitor** for **Client**

#### [no-toc] [no-number] &sect; Process

- **Client** create **Concrete Visitor**
- **Client** call methods of **Aggregate** using **Abstract Visitor**
- **Aggregate** pass **Abstract Visitor** to internal **Abstract Element**
- **Concrete Element** accept **Abstract Visitor** by passing this to it
- **Concrete Visitor** access **Concrete Element** and accumulate result

#### [no-toc] [no-number] &sect; Info Hidden

- **Visitor** know all **Concrete Element**
  - (must cover all kinds of **Concrete Element** for visiting interface)
- **Element** not know **Concrete Visitor**
  - (only deal with interface of **Abstract Visitor** and accept it)

#### [no-toc] [no-number] &sect; Uses

- Internal iteration over non-uniform element
- Solution to _double dispatch_
  - (type of **Element** bind with type of **Visitor** action dynamically)
  - (**Element** dispatch firstly, **Visitor** dispatch secondly)

#### [no-toc] [no-number] &sect; Comparison

- vs. [sec|Iterator] Iterator
  - Provide internal iteration over _composite_ and _tuple_
- vs. [sec|Interpreter] Interpreter
  - Separate operation from class of element
  - Element be less than operation

### Interpreter

> Add **interpret interface** to abstract syntax tree
> ([&sect; Composite](Design-Patterns-Notes-2.md#Composite))

#### [no-toc] [no-number] &sect; Roles

- **Abstract Expression** define interface to interpret expression
- **Nonterminal Expression** represent nonterminal symbols in gramma
- **Terminal Expression** represent terminal symbols in gramma
- **Context** provide input/output context for interpreter

#### [no-toc] [no-number] &sect; Process

- **Client** build **Abstract Expression**
  with **Nonterminal** and **Terminal Expression**
- **Client** initialize **Context**
  and call methods of **Abstract Expression**
- **Nonterminal** and **Terminal Expression** access data in **Context**

#### [no-toc] [no-number] &sect; Info Hidden

- **Context** not know **Expression** (passive)

#### [no-toc] [no-number] &sect; Uses

- Match regular expressions
  - (context: remaining input sequence)
  - (slower than automaton solution)
- Evaluate expressions
  - (context: variable mapping in expression)
  - (evaluate constrant in compiler or varialbe in interpreter)

#### [no-toc] [no-number] &sect; Comparison

- vs. [sec|Visitor] Visitor
  - Distribute operation over class of element
  - Operation (only interpretation) be less than element (expression)

## Encapsulate Representation

### Memento

> Encapsulate **object representation** to support undo

#### [no-toc] [no-number] &sect; Roles

- **Memento** represent a snapshot of **Originator**
- **Originator** provide interface to create and restore **Memento**

#### [no-toc] [no-number] &sect; Process

- **Client** create **Memento** from **Originator**
- **Client** restore **Originator** with **Memento** (async)
- **Originator** update internal state using **Memento**

#### [no-toc] [no-number] &sect; Info Hidden

- **Originator** is friend of **Memento** (access representation)
- **Memento** not know **Originator** (not care)
- **Client** not know representation of **Memento** / **Originator**
  - (only treat them as concrete object)

#### [no-toc] [no-number] &sect; Uses

- Scoped save-restore operation
  - SaveDC and RestoreDC in Windows GDI
- Undoable operation in [sec|Command] Command
- Memento-based iteration ([sec|Iterator] Iterator)
  - Interface: `aggregate.next(iterState)` ~ `iter.next()`
  - Reverse friendship between aggregate and iterator/memento
