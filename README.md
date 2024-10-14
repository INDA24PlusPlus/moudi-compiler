# Build

- git clone https://github.com/INDA24PlusPlus/moudi-compiler
- cd moudi-compiler
- git submodule init
- git submodule update
- ./start.sh c

# Run

- Build the compiler
- ./start.sh PATH/TO/YAY/FILE

# BNF
<program> ::= <function>+

<function> ::= "fn" <whitespace>+ <ID> <opt_whitespace> <function_arg_list> <opt_whitespace> <scope>
<function_arg_list> ::= "(" <function_arg>* ")"
<function_arg> ::= <opt_whitespace> <ID> <opt_whitespace> "," <function_arg> | <opt_whitespace> <ID> <opt_whitespace>

<scope> ::= "{" <newline>* <scope_element>* <newline>* "}"
<scope_element> ::= <newline>* <statement> <newline>+
<statement> ::= <expr> | <declaration> | <for> | <if> | <ret>

<expr> ::= <value> | <binary> | <unary>
<binary> ::= <opt_whitespace> <expr> <opt_whitespace> <operator>+ <opt_whitespace> <expr>
<unary> ::= <opt_whitespace> <expr> <opt_whitespace> <operator>+ | <opt_whitespace> <operator>+ <opt_whitespace> <expr> <opt_whitespace>

<declaration> ::= "let" <whitespace> <ID> <opt_whitespace> "=" <opt_whitespace> <expr>

<if> ::= "if" <whitespace> <expr> <opt_whitespace> <scope> <else>*
<else> ::= "else" <whitespace> <if> | "else" <opt_whitespace> <scope>

<for> ::= "for" <opt_whitespace> <for_tail>
<for_tail> ::= <expr> <opt_whitespace> ":" <opt_whitespace> <expr> <opt_whitespace> <scope> | <expr> <opt_whitespace> <scope>
<ret> ::= "return" <whitespace> <expr>

<value> ::= <ID> | <digit>+
<ID> ::= <letter> <letter_or_digit_or_underscore>*

<letter_or_digit_or_underscore> ::= <letter_or_digit> | "_"
<letter_or_digit> ::= <letter> | <digit>
<opt_whitespace> ::= " "*
<whitespace> ::= " " <whitespace> | " "
<newline> ::= "\n"
<operator> ::= "+" | "-" | "*" | "/" | "|" | "=" | "<" | "<" | "(" | ")"
<letter> ::= "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" | "J" | "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" | "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z" | "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i" | "j" | "k" | "l" | "m" | "n" | "o" | "p" | "q" | "r" | "s" | "t" | "u" | "v" | "w" | "x" | "y" | "z" 
<digit> ::= "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
