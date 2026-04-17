# **Prometheus Programming Language**

**Prometheus** is a lightweight, statically-typed, interpreted programming language written in C++. Designed with a clean, C-style syntax, it features robust support for core data types, mathematical operations, and control flow.

## **Features**

* **Static-ish Typing**: Explicitly declare int, double, str, and bool.  
* **C-Style Syntax**: Familiar curly-brace blocks and semicolon statement termination.  
* **Functions**: Support for typed return values and parameter passing.  
* **Control Flow**: if/elif/else logic, while loops, and for loops.  
* **Built-in Functions**: print() for output and input() for user interaction.  
* **Type Casting**: Explicit casting using int(), double(), and str() functions.

## **Syntax Overview**

### **Variables & Types**

Prometheus supports common primitive types:

```{Prometheus}
int x = 10;  
double pi = 3.14;  
str greeting = "Hello Prometheus";  
bool isActive = true;
```

### **Mathematical Operators**

Supports standard arithmetic and power operations:

* \+, \-, \*, /, %  
* \*\* (Power)  
* \++, \-- (Increment/Decrement)

### **Control Flow**

**Conditional Statements:**

```{Prometheus}
if (x > 10) {  
    print("Large");  
} elif (x == 10) {  
    print("Equal");  
} else {  
    print("Small");  
}
```

**Loops:**

```{Prometheus}
for (int i = 0; i < 5; i++;) {  
    print("Iteration:", i);  
}

while (x > 0) {  
    x--;  
}
```

### **Functions**

Functions are declared with a return type, name, and typed parameters:

```{Prometheus}
func int add(int a, int b) {  
    return a + b;  
}

int result = add(5, 10);
```

## **Example: FizzBuzz**

A classic implementation in Prometheus:

```{Prometheus}
for (int i = 1; i < 101; i++;) {  
    if (i % 3 == 0 && i % 5 == 0) {  
        print("FizzBuzz");  
    }  
    elif (i % 3 == 0\) {  
        print("Fizz");  
    }  
    elif (i % 5 == 0) {  
        print("Buzz");  
    }  
    else {  
        print(i);  
    }  
}
```

## **Technical Architecture**

The Prometheus interpreter is built using a standard three-stage pipeline:

1. **Lexer (lexer.cc)**: Tokenizes source text into a stream of symbols, identifying keywords, literals, and operators.  
2. **Parser (parser.cc)**: Performs recursive descent parsing to generate an Abstract Syntax Tree (AST).  
3. **Interpreter (interpreter.cc)**: Traverses the AST and executes logic using a visitor pattern, managing variables in an unordered\_map of std::variant.

### **Building and Running**

To compile the Prometheus interpreter:

```{Bash}
Makefile
```

To run a .prm file:

```{Terminal}
./prometheus filename.prm
```

## **Error Handling**

Prometheus includes custom exception handling for:

* **Lexer Errors**: Invalid characters or malformed numbers.  
* **Parse Errors**: Syntax violations and unexpected tokens.  
* **Runtime Errors**: Undefined functions, type mismatches, or missing variables.