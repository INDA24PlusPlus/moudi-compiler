#pragma once


enum Operators {
    OP_NOT_FOUND,
    PARENTHESES,

    INCREMENT,
    DECREMENT,
    CALL,

    UNARY_MINUS,

    MULTIPLICATION,
    DIVISION,
    REMAINDER,

    ADDITION,
    SUBTRACTION,

    LESS_THAN,
    LESS_EQUAL_TO,
    GREATER_THAN,
    GREATER_EQUAL_TO,

    EQUAL,
    NOT_EQUAL,

    LOGICAL_AND,
    LOGICAL_OR,

    ASSIGNMENT,
};

enum OP_mode {
    UNARY_PRE, // an operator taking one operand preceeding itself: ++a
    UNARY_POST,// an operator taking one operand proceeding itself: a++
    BINARY, // an operator taking two operands on either side of the operator
    OP_TYPE_ANY
};

enum OP_associativity {
    LEFT,
    RIGHT
};

enum OP_enclosed {
    NORMAL,
    ENCLOSED, // an operator where the last operand is enclosed by certain characthers 
              // separated by a \0 to separate the starting characthers to the ending
};

const static struct Operator {
    enum Operators key;
    enum OP_mode mode;
    char precedence;
    enum OP_associativity associativity;
    enum OP_enclosed enclosed;
    char enclosed_offset;
    char * str;
} op_conversion [] = {
    {OP_NOT_FOUND, UNARY_PRE, 0, LEFT, NORMAL, 0, ""},
    {PARENTHESES, UNARY_PRE, 0, LEFT, ENCLOSED, 2, "(\0)"},
    
    {INCREMENT, UNARY_PRE, 1, LEFT, NORMAL, 0, "++"},
    {INCREMENT, UNARY_POST, 1, LEFT, NORMAL, 0, "++"},
    {DECREMENT, UNARY_PRE, 1, LEFT, NORMAL, 0, "--"},
    {DECREMENT, UNARY_POST, 1, LEFT, NORMAL, 0, "--"},

    {CALL, BINARY, 1, LEFT, ENCLOSED, 2, "(\0)"},
    
    {UNARY_MINUS, UNARY_PRE, 2, RIGHT, NORMAL, 0, "-"},
    
    {MULTIPLICATION, BINARY, 3, LEFT, NORMAL, 0, "*"},
    {DIVISION, BINARY, 3, LEFT, NORMAL, 0, "/"},
    {REMAINDER, BINARY, 3, LEFT, NORMAL, 0, "%"},

    {ADDITION, BINARY, 4, LEFT, NORMAL, 0, "+"},
    {SUBTRACTION, BINARY, 4, LEFT, NORMAL, 0, "-"},

    {LESS_THAN, BINARY, 6, LEFT, NORMAL, 0, "<"},
    {LESS_EQUAL_TO, BINARY, 6, LEFT, NORMAL, 0, "<="},
    {GREATER_THAN, BINARY, 6, LEFT, NORMAL, 0, ">"},
    {GREATER_EQUAL_TO, BINARY, 6, LEFT, NORMAL, 0, ">="},

    {EQUAL, BINARY, 7, LEFT, NORMAL, 0, "=="},
    {NOT_EQUAL, BINARY, 7, LEFT, NORMAL, 0, "!="},

    {LOGICAL_AND, BINARY, 11, LEFT, NORMAL, 0, "&&"},
    {LOGICAL_OR, BINARY, 12, LEFT, NORMAL, 0, "||"},

    {ASSIGNMENT, BINARY, 15, RIGHT, NORMAL, 0, "="},
};
