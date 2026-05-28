# Arcana

> **Shape the Language. Control the Machine.**

Arcana is an experimental, **metaprogramming-first** and **language-oriented** systems programming language.
It is designed for developers who find existing languages too rigid and want the power to extend their tool without sacrificing the performance of a
formal LLVM pipeline.

In Arcana, syntax is not a law-it is a malleable tool.

---

## Project Status: Early Alpha (Experimental) [v0.0.4]

Arcana is currently in a heavy development phase. It is a proof-of-concept for language-oriented programming and is **not suitable for production use yet**.


## The Philosophy

Most languages force you to wait for a committee to add a new feature; in Arcana, you are the committee that decides how the language should work.

A programming language is a tool to talk to the machine, to give it orders.

We are not its prisoners; we are programmers. We are the ones that command and demand that the machine do what we tell it to do.

That's why Arcana is being developed.

---

### The Arcane System

The heart of Arcana is the ability to define your own language constructs.

#### Example: Custom Loop

Define a loop that executes a block a specific number of times.

```arcn
arcane CustomLoop (loop: key, block: code) {

  rules [
    @simple: loop [ block ];

  ];

  @simple {

    loop [int a] <=> {

      while (a) {
        block;
        a = a - 1;

      }

    };

  }

}

func main() -> int {

  int x = 10;

  loop [10] {
    x = x + 9;

  }

  return x;

}
```

### Meta-directives and Chainable Arcanes

#### Example: For - Else

Create a ```for``` construct that allows an optional ```else``` block afterwards.

```arcn
arcane ForElse (for: key, for_block: code, else: key, else_block: code, cond: expr) {

  rules [
    @simple   : for  [ cond  for_block ];
    @with_else: else [      else_block ];

  ];

  chains [
    @simple -> @with_else? ;

  ];

  @simple {

    for [int* i] <=> {

      bool finished = false;

      while (cond) {
        for_block;

        *i = *i + 1;
        if (!cond) { finished = true; }

      }

      ?chain(@with_else) {
        if (finished) { else_block; }
      }

    };

  }

  @with_else {
    else <=> {};

  }

}

func main() -> int {

  int a = 0;
  int b = 1;

  for [&a] (a < 5) {
    b = b * 2;

  } else {
    b = b * 3;

  }

  return b;

}
```

### The Trait System

Traits represent the behavioral properties of a code block. You can define how a block executes and interacts with the machine's state. While Traits are
essential for defining new constructs within the Arcane system, they are universal primitives that can be applied to **any** block of code to enforce specific
behaviors.

#### Example: Do - While

The following example demonstrates how a post-condition behavior (a ```do-while``` pattern) is implemented by assigning the ```#loop``` trait to the code
block.

```arcn
arcane DoWhile (do: key, body: code, while: key, cond: expr) {

  rules [
    @standard: do [ body while cond ];

  ];

  @standard {
    do <=> #loop {
      body;
      if (!cond) { break; }

    };

  }

}

func main() -> int {

  int i = 4;
  int j = 1;

  do {
    i = i - 1;
    j = j * 2;

  } while (i);

  return j;

}
```

---

## Join the Arcane

Arcana is a call to action for anyone who believes a language should adapt to the programmer, not the other way around. We aren't just building a tool to write
code-we are building a way to **redefine the relationship between the developer and the machine**.

**We are looking for contributors.** Whether you are just starting your journey or you are someone who wants to break and rewrite the rules of what a language
should look like, **your perspective is what moves this project forward**.

**Let's build something impossible.**

---

## Contact:
- **Discord** `panqueque.boo`
- **Email** salastomasalejandro1@gmail.com
