# Mila compiler

## Building

Install dependencies:
```bash
sudo apt install llvm llvm-dev clang git cmake zlib1g-dev
```

Build:
```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

## Running

Run:
```bash
./milac input.mila && ./a.out
```

Help:
```bash
./milac --help
```

# Mila programming language specification

## Grammar (Syntax)

Grammar is in grammar.txt file.

## Types

### Primitive types

| Name    | Syntax    | Description           |
|---------|-----------|-----------------------|
| integer | `integer` | 32-bit integer        |
| real    | `real`    | 64-bit floating-point |

### Complex types

| Name  | Syntax                             | Description                                                                                                    |
|-------|------------------------------------|----------------------------------------------------------------------------------------------------------------|
| array | `array [l .. u] of primitive_type` | Declares an array of size (l - u + 1) where l, u are integers and l is the start index and u is the last index |

## Expressions

Operators precedence and associativity just like you would expect in C.

### Binary infix operators

| Operator | Description           |
|----------|-----------------------|
| `+`      | addition              |
| `-`      | subtraction           |
| `*`      | multiplication        |
| `/`      | division              |    
| `=`      | equal                 |
| `>`      | greater than          |
| `<`      | less than             |
| `<>`     | not equal             |
| `>=`     | greater than or equal |
| `<=`     | less than or equal    |
| `:=`     | assignment            |
| `or`     | bitwise or            |
| `and`    | bitwise and           |
| `mod`    | modulo                |
| `div`    | integer division      |

### Unary prefix operators

| Operator | Description |
|----------|-------------|
| `-`      | minus       |
| `not`    | bitwise not |

### Unary postfix operators

| Operator | Description     |
|----------|-----------------|
| `[]`     | array subscript |
| `()`     | function call   |

## Important keywords

| Keyword     | Description               |
|-------------|---------------------------|
| `program`   | named module              |
| `var`       | variable declaration      |
| `const`     | constant definition       |
| `procedure` | procedure declaration     |
| `function`  | function declaration      |
| `forward`   | forward declaration       |
| `break`     | break from loop           |
| `exit`      | exits function or program |

### Control flow

| Control                                                                  | Description         |
|--------------------------------------------------------------------------|---------------------| 
| `begin ([[statement]];)+ end`                                            | compound statement  |
| `while [[boolean expression]] do [[statement]]`                          | while loop          |
| `for [[range assignment]] do [[statement]]`                              | for loop            |
| `if [[boolean expression]] then [[statement]]` <br> `else [[statement]]` | condition branching |

## Built-in functions

| Name         | Description                                                             |
|--------------|-------------------------------------------------------------------------|
| `write`      | Write variable of primitive type to standard output                     |
| `writeln`    | Write variable of primitive type to standard output and append new line |
| `readln`     | Read to variable of primitive type from a line of standard input        |
| `to_integer` | Converts variable to integer type                                       |
| `to_real`    | Converts variable to real type                                          |

## Punctuation

| Symbol | Description         |
|--------|---------------------|
| `;`    | statement delimiter |
| `:`    | type denotation     |
| `(`    | opening parenthesis |
| `)`    | closing parenthesis |
| `[`    | opening brackets    |
| `]`    | closing brackets    |
| `..`   | interval denotation |
| `.`    | dot                 |
| `,`    | comma  			          |