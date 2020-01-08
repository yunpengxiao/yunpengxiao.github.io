# Implementation Patterns Notes

> Reading in 2018/2 - 2018/3
>
> Digest of _Implementation Patterns_, Kent Beck

## [no-toc] TOC

[TOC]

### Viewpoints of Code P. xv

- What will the **computer** do with this code?
- How can I **communicate** what I am thinking to people?

### Technical Debt P. xv

So much software development **money** is spent on **understanding** existing code.

### Responsibility of Programmer P. xvi

- As a programmer you have been given time, talent, money, and opportunity.
- What will you do to make responsible use of these **gifts**?

### Implementation Patterns Catalog P4

```
           ------ Class ------
           |                 |
    (similar logic)    (different data)
           |                 |
        Behavior           State
           |                 |
    (dividing logic)  (multi-valued data)
           |                 |
        Methods         Collections
```

### Program Laws P5

- Programs are **read more often** than they are written.
- There is **no** such thing as **“done”**. Much more investment will be spent modifying programs than developing them initially.
- They are **structured** using a basic set of **state** and **control flow** concepts.
- Readers need to understand programs **in detail** and **in concept**. Sometimes they move from detail to concept, sometimes from concept to detail.

### About Communication P10

A progam should read like a **book**. It should have plot, rhythm, and delightful little turns of phrase. —— Knuth’s _Literate Programming_

### Coding Priciples P13-17

- Local Consequences P13
- Minimize Repetition P14
- Logic and Data Together P14
- Symmetry (logic symmetry) P15
- Declarative Expression (communication) P16
- Rate of Change (temporal symmetry) P17
  - put logic or data that changes at the same rate together
  - separate logic or data that changes at different rates

### Coding Motivation P20

Code is by people and for people.

### Name of Class P24

Use class names to tell the **story** of your code.

### Old Adage in Software Development P24

Code to interfaces, not implementations.

### Interface vs Superclass P27

- Interface: Here’s how to access this kind of functionality
- Superclass: Here’s one way to implement this functionality

### Design Switch between Strategies P34

- with conditionals P36
- with subclasses P32
- with delegation P38

### Advantage of Class P42

A class bundles together related state.

### Advantage of Object P43

**Objects** are convenient packages of **behavior** which is presented to the outside world and **state** which is used to support that behavior.

### Metaphor for Programming Computers P44

- **State** changing over time.
- related: [sec|Metaphors of Computing by John Von Neumann P63]

### Object Design Problem P47

- **Most accesses** to an object’s state are **outside** the object.
- related: [sec|Too Many Setters is a Sign P128]

### Common Roles for Local Variables P51

- Collector: `return result(s)`
- Count: special collector, `return count`
- Explaining: could be refactored to helper methods
- Reuse: like `timestamp = now()`
- Element: `for (each : collection)`

### Common Roles for Fields P53

- Helper: used by many methods
- Flag: used to switch strategies, could be refactored ([sec|Design Switch between Strategies P34])
- Strategy: see [Strategy Pattern](../2017/Design-Patterns-Notes-3.md#Strategy)
- State: see [State Pattern](../2017/Design-Patterns-Notes-3.md#State)
- Components: object properties

### Variable Role-Suggesting Naming P60

- communicate the **role** a variable plays through its **name**
- communicate **lifetime, scope, and type** through **context** (with IDE)

### Metaphors of Computing by John Von Neumann P63

- A sequence of **instructions** that are executed one by one.
- related: [sec|Metaphor for Programming Computers P44]

### Intention and Implementation P69

The **distinction** between intention and implementation has always been important in software development.

### Usage Order of Control Flow P72

sequence > messages > iteration > conditionals > exceptions

### Mixture of Abstraction P77

One of the signs of a **poorly composed** method is a **mixture of abstraction** levels.

``` c
void compute() {
    input();
    flags|= 0x0080;
    output();
}
```

### Beauty of Object P83

One of the beauties of programming with objects is the variety of ways they provide to **express the differences** between **similar calculations**.

### Return Type P84

- `void Fn`: **procedure** that works by side-effect
- `Type Fn`: **function** returning a particular type of object
- related: [Separate Query from Modifier](Refactoring-Notes.md#Separate-Query-from-Modifier-P279)

### One Change at A Time P89

A change _here_ doesn’t cause an unanticipated problem _there_.

### Basic Idea of Collections P99

A collection **distinguishes** between objects **in the collection** and those **not in the collection**.

### Only Interesting Numbers in Computer P99

- **0**: **absence** of a field
- **1**: **presence** of a field
- **many**: a field holding a **collection**

### Too Many Setters is a Sign P128

- Rather than publish a setter, publish a method that is named for the **problem** the client **needs to solve** rather than the implementation.
- related: [sec|Object Design Problem P47]

### Orthogonal Design P130

Look at a framework as the **intersection** of all the useful **functionality** in a domain **rather than** the **union**.
