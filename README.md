# Arcana

> **Shape the Language. Control the Machine.**

Arcana is an experimental, **metaprogramming-first** and **language-oriented** systems programming language.
It is designed for developers who find existing languages too rigid and want the power to extend their tool without sacrificing the performance of a formal
LLVM pipeline.

In Arcana, syntax is not a law-it is a malleable tool.

---

## Project Status: Early Alpha (Experimental) [v0.0.4]

Arcana is currently in a heavy development phase. It is a proof-of-concept for language-oriented programming and is **not suitable for production use yet**.


## The Philosophy

Most languages force you to wait for a committee to add a new feature. Arcana gives you the **Arcane** system:
a way to expand the compiler's Abstract Syntax Tree (AST) directly from your source code.

Arcana inverts this relationship. Through the **Arcane** and **Trait** systems, the language becomes a...

### Side-Effect Clarity
Strict visual separation between logic and data:

* Code Blocks {}: Represent actions and side effects.
* Expressions (): Represent values.

#### Note: An expression can modify or assign values (e.g., ++i, (j = 2), (int k = 3)).

---

### The Arcane System:

The heart of Arcana is the ability to define your own language constructs.

#### Example: Custom Loop

Define a loop that executes a block a specific number of times without the boilerplate of a traditional ```for``` loop.

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

### The Trait System

Traits represent the behavioral properties of a code block. You can define how a block executes and interacts with the machine's state. While Traits are
essential for defining new constructs within the Arcane system, they are universal primitives that can be applied to **any** block of code to enforce specific
behaviors.

#### Example: Do While

The following example demonstrates how a post-condition behavior (a ```do-while``` pattern) is implemented by assigning the ```#loop``` trait to the code
block.

```arcn
arcane DoWhile (do: key, body: code, while: key, cond: expr) {
  rules [
    @standard  : do [ body while cond ];

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

**We are looking for contributors.** Whether you are just starting your journey or you're someone who wants to break and rewrite the rules of what a language
should look like, **your perspective is what moves this project forward**.

**Let's build something impossible.**

---

## Contact:
- **Discord** `panqueque.boo`
- **Email** salastomasalejandro1@gmail.com
