# Arcana: Ideas
This document serves as a "living diary" for potential features, ergonomic experiments and design shifts in the
Arcana language. Ideas here are not yet implemented but are under consideration.

Please reach out if you have an idea that you'd like to see implemented (see ## Contact in README.md)


## Floating Point Semantics
### Just a short hand
Traditional bit-wise equality (==) for floats is often a source of bugs. Arcana aims to solve this at a language
level.

#### The Fuzzy Equality Operator: ~=
Syntax: [float1 ~= float2]
Semantic: [abs(float1 - float2) < epsilon]

Note: epsilon would be user-definable or a default value if no value is provided


## Left-Handed Sorcery (Ergonomic Keywords)
### Who said a programming language's keywords should take the whole keyboard?
To improve accessibility and reduce finger travel distance, Arcana explores "Left-Handed Keywords"--aliases
for common types and control structures that can be typed entirely with the left hand (QWERT / ASDFG / ZXCVB ).


#### Types
[vd] -> [void ] (void)
[wd] -> [int]   (word)
[ft] -> [float] (float/decimal)
[tf] -> [bool]  (true/false)

#### Keywords
[br]  -> [if]     (branch)
[re]  -> [else]   (residual)
[as]  -> [while]  (as)
[fc]  -> [func]   (function)
[ret] -> [return] (return)

#### Symbols

[CS] -> [/-] (Comment start)
[CE] -> [-/] (Comment end)

[start] -> [{] (Start)
[ed]    -> [}] (End)

[dt] -> [.] (Dot)
[dd] -> [:] (Colon)
[cc] -> [,] (Comma)
[sc] -> [;] (Semicolon)

[se] -> [(] (Start expression)
[ee] -> [)] (End expression)

[arr] -> [->] (Arrow)

[va] -> [=] (Variable Assignment)

[ad] / [add] -> [+]  (Addition)
[st]       -> [-]  (Subtract)
[tes]      -> [*]  (Times)
[dv]       -> [/]  (Div)
[wer]      -> [**] (Power)
[rt]       -> [*/] (Root)

[ac] -> [[ ]] (Access start)
[ae] -> [[ ]] (Access end)

[eq] -> [==] (Equal)
[gt] -> [>]  (Greater than)
[ge] -> [>=] (Greater or equal than)

[feq] / [fz] -> [~=] (float equal) / (fuzzy)

### Rationale:
While some aliases might result in more verbose expressions (e.g., ac se ... ee add se ... ee ae), they
eliminate the need for complex hand movements and key combinations, making programming accessible to everyone.

## Casting: A different approach
### More casting options...
Everyone is familiar with the C-style casting:
[(Type)(Expression)]

But what if you had multiple options?
#### Old Style Cast Syntax:    (Type)(Expression)
#### Static Cast Syntax:       {Type}(Expression)
#### Reinterpret Cast Syntax: \[Type](Expression)
