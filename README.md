# Arcana

> **Shape the Language. Control the Machine.**

Arcana is an experimental, **metaprogramming-first** and **language-oriented** systems programming language.
It is designed for developers who find existing languages too rigid and want the power to extend their tool without sacrificing the performance of a formal
LLVM pipeline.

In Arcana, syntax is not a law-it is a malleable tool.

---

## Project Status: Early Alpha (Experimental) [v0.0.3]

Arcana is currently in a heavy development phase. It is a proof-of-concept for language-oriented programming and is **not suitable for production use yet**.


## The Philosophy

Most languages force you to wait for a committee to add a new feature. Arcana gives you the **Arcane** system:
a way to expand the compiler's Abstract Syntax Tree (AST) directly from your source code.


### Side-Effect Clarity
Strict visual separation between logic and data:

* Code Blocks {}: Represent actions and side effects.
* Expressions (): Represent values.

#### Note: An expression can change and/or assign the value of a variable (e.g., ++i, (j = 2), (int k = 3)).

---

### The Arcane System

The heart of Arcana is the ability to define your own language constructs.


#### Example 1: Custom Keyword

Create a ```twice``` keyword that executes a block twice and an evaluated ```twice_if``` that checks a condition first.


```arcn
arcane Twice_TwiceIf (twice: key, twice_if: key, expr1: expr, block1: code) {

  rules [
    @simple: twice    [       block1 ];
    @eval  : twice_if [ expr1 block1 ];
  ];

  @simple {
    twice <=> {
      block1;
      block1;
    };
  }

  @eval {
    twice_if <=> {
      if (expr1) {
        block1;
        block1;
      }
    };
  }

}

func main() -> int {

  int x = 1;
  int y = 2;
 
  twice {
    x = x + 3;
  }
 
  twice_if (1) {
    y = y + 3;
  }
 
  return x + y;

}
```

#### Example 2: Custom Loop

You aren't restricted to ```while``` or ```for```. You can define iteration logic that fits your specific data structures.


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

---

## Join the Arcane

Arcana is a call to action for anyone who believes a language should adapt to the programmer, no the other way around. We aren't just building a tool to write
code--we are building a way to **redefine the relationship between the developer and the machine**.

**We are looking for contributors.** Whether you are just starting your journey or you're someone who wants to break and rewrite the rules of what a language
should look like, **your perspective is what moves this project forward**.

**Let's build something impossible.**

---

## Contact:
- **Discord** `panqueque.boo`
- **Email** salastomasalejandro1@gmail.com
