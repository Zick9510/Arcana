# Arcana

> **Shape the Language. Control the Machine.**

Arcana is an experimental, **metaprogramming-first** and **language-oriented** systems programming language.
It is designed for developers who find existing languages too rigid.
In Arcana, syntax is not a law-it is a malleable tool.

---

## Project Status: Early Alpha (Experimental)
Arcana is currently in a heavy development phase. The core Lexer and Parser are capable of handling expressions,
and the Semantic Checker is currently being implemented. **Not suitable for production use.**

## The Philosophy
Most languages force you to wait for a committee to add a new feature. Arcana gives you the **"Arcano"** system:
a way to expand the compiler's Abstract Syntax Tree (AST) directly from your source code.

-- **Side-Effect Clarity:** Strict visual separation between code blocks '{}' (actions/effects) and expressions '()' (values).
--


## Example: The "Arcano" System
```
```

```arc
arcano Maybe(maybe: key, name: string, code: cod) {

  rules {
    maybe [ name? { code } ] // After the 'maybe' keyword, the syntax might have a string and then a block of code
  };

  maybe <=> {
    if (random() > 0.5) {
      code;
    }
  };
}

// Use:
maybe "Hi" { // Not 100% certain it will execute the following block
  print(1 + 2);
  print(2 + 3);
}

maybe {
  print(2 */ 3); // 2 ** (1 / 3)
}

```


