# Arcana

> **Shape the Language. Control the Machine.**

Arcana is an experimental, **metaprogramming-first** and **language-oriented** systems programming language.
It is designed for developers who find existing languages too rigid.
In Arcana, syntax is not a law-it is a malleable tool.

---

## Project Status: Early Alpha (Experimental) [v0.0.3]
Arcana is currently in a heavy development phase. **Not suitable for production use yet.**

## The Philosophy
Most languages force you to wait for a committee to add a new feature. Arcana gives you the **Arcane** system:
a way to expand the compiler's Abstract Syntax Tree (AST) directly from your source code.

### Side-Effect Clarity: Strict visual separation between code blocks '{}' (actions) and expressions '()' (values).
#### Note: An expression can change and/or assign the value of a variable (e.g., ++i, (j = 2), etc. )

### Example: The Arcane System

```arc
arcane Twice_TwiceIf (twice: key, twice_if: key, expr1: expr, block1: code) {

  rules [
    @simple: twice [ block1 ];
    @eval  : twice [ expr1 block1 ];
  ];

  @simple {
    twice () <=> {
      block1;
      block1;
      };

  }

  @eval {
    twice_if (int a) <=> {
      if (a) {
        block1;
        block1;
      }
    };
  }
}

func main() -> int {

  int x = 1;
  int y = 2;
  int z = 3;
 
  twice {
  x = x + 3;
  }
 
  twice_if (1) {
  y = y + 3;
  }
 
  twice_if (0) {
  z = z + 3;
  }
 
  // x = 7
  // y = 8
  // z = 3
 
  return x + y + z;
}
```

## Contact:
- **Discord** `panqueque.boo`
- **Email** salastomasalejandro1@gmail.com
