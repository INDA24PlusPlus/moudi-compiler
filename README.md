# Build

1. git clone https://github.com/INDA24PlusPlus/moudi-compiler
2. cd moudi-compiler
3. git submodule init
4. git submodule update
5. ```./start.sh c```

# Run

1. Build the compiler
2. ```./start.sh {SOURCE CODE PATH} [--ast | --time]```

# Flags
- ast: Print out the constructed AST
- time: Print out the time it took for each compilation pass

# BNF
```code
<program> ::= <function>+

<function> ::= "fn" <whitespace>+ <ID> <opt_whitespace> <function_arg_list> <opt_whitespace> <scope>
<function_arg_list> ::= "(" <function_arg>* ")"
<function_arg> ::= <opt_whitespace> <ID> <opt_whitespace> "," <function_arg> | <opt_whitespace> <ID> <opt_whitespace>

<scope> ::= <opt_whitespace> "{" <newline>* <scope_element>* <newline>* "}" <opt_whitespace>
<scope_element> ::= <newline>* <statement> <newline>+
<statement> ::= <expr> | <declaration> | <for> | <if> | <ret>

<expr> ::= "#" <ID> | <value> | <string> | <binary> | <unary>
<binary> ::= <opt_whitespace> <expr> <opt_whitespace> <operator>+ <opt_whitespace> <expr>
<unary> ::= <opt_whitespace> <expr> <opt_whitespace> <operator>+ | <opt_whitespace> <operator>+ <opt_whitespace> <expr> <opt_whitespace>

<declaration> ::= "let" <whitespace> <ID> <opt_whitespace> "=" <opt_whitespace> <expr>

<if> ::= "if" <whitespace> <expr> <opt_whitespace> <scope> <else>*
<else> ::= "else" <whitespace> <if> | "else" <opt_whitespace> <scope>

<for> ::= "for" <opt_whitespace> <for_tail>
<for_tail> ::= <expr> <opt_whitespace> ":" <opt_whitespace> <expr> <opt_whitespace> <scope> | <expr> <opt_whitespace> <scope>
<ret> ::= "return" <whitespace> <expr>

<string> ::= "\"" (<digit> | <letter>)* "\""
<value> ::= <ID> | <digit>+
<ID> ::= <letter> <letter_or_digit_or_underscore>*

<letter_or_digit_or_underscore> ::= <letter_or_digit> | "_"
<letter_or_digit> ::= <letter> | <digit>
<opt_whitespace> ::= " "*
<whitespace> ::= " "+
<newline> ::= "\n"
<operator> ::= "+" | "-" | "*" | "/" | "|" | "=" | "<" | "<" | "(" | ")"
<letter> ::= [A-z]
<digit> ::= [0-9]
```
