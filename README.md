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

### Side-Effect Clarity: Strict visual separation between code blocks '{}' (actions) and expressions '()' (values).
#### Note: An expression can change and/or assign the value of a variable (e.g., i++, (j = 2), etc. )


## Note: Not implemented, subject to changes
### Example: The Arcano System

```arc
arcano Maybe(maybe: key, name: string, code: code) {

  rules {
    maybe [ name? { code } ] // After the 'maybe' keyword, the syntax
                             // might have a string and then a block of code
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

### Example: The Spell System
```arc
spell AddOne([#]: prefix, expr1: expr) {
	rules {
		[[#] expr1]
	};
	
	[#] <=> {
		return (expr1 + 1);
	};


}

// Use:

var a = #4;  // a = 5

spell NullCheckOp(expr1: expr, [??]: op, expr2: expr) {
	rules { // Binary Operator
		[expr1 [??] expr2]
	
	};
	
	[??] <=> {
		if (expr1 != null) {
			return expr1;
		} else {
			return expr2;
		}
	
	};


}

// Use:
null  ?? true; // Will return true
false ?? true; // false
false ?? null; // false

```

## Contact:
- **Discord** `panqueque.boo`
- **Email** salastomasalejandro1@gmail.com
