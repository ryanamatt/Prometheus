# **Prometheus Programming Language Specification**

Welcome to the official documentation for **Prometheus**. This document serves as the central repository for the language's syntax, features, and standard library.

## **1\. Introduction**

Prometheus is a **procedural, interpreted language** designed for simplicity and directness. It focuses on clear syntax and predictable execution, making it an ideal choice for scripting and educational purposes.

## **2\. Getting Started**

* **Installation**: Prometheus is currently available as an interpreted environment. Run the binary followed by your script name.  
* **Hello World**:  
  print("Hello World");

## **3\. Basic Syntax**

### **General Rules**

* **Semicolons**: Every statement must terminate with a semicolon ;.  
* **Blocks**: Code blocks are delimited by curly braces { }.  
* **Comments**: Single-line comments start with the \# symbol.

```{Prometheus}
  # This is a comment in Prometheus  
  int x = 1; # This is also a comment
```

### **Variables and Operators**

* **Declaration**: Variables are declared by specifying the type followed by the identifier. 

```{Prometheus}
  int x = 10;  
  double y = 20.5;  
  bool isActive = true;  
  str s = "Hello World";
```

* **Arithmetic Operators**:  
  * \+ : Addition  
  * \- : Subtraction  
  * \* : Multiplication  
  * / : Division (**Note**: Always returns a double)  
  * % : Modulo (Remainder)  
  * \*\*: Exponentiation (Power)  
* **Comparison Operators**:  
  * \==, \!=, \>, \<, \>=, \<=  
* **Unary Operators**:  
  * x++; (Increment)  
  * x--; (Decrement)  
* **Logical Operators**:  
  * &&, ||, \!

### **Data Types & Casting**

* **Primitive types**: int, double, bool, str.  
* **Type Casting**:  
  * int(value), double(value), str(value), bool(value).

## **4\. Control Flow**

* **Conditionals**: if, elif, and else. Parentheses are required.  
* **Loops**: while and for loops.  
  * **For Loop**: Requires a trailing semicolon in the update expression.  

```{Prometheus}
    for (int i \= 0; i \< 10; i++;) {  
        print(i);  
    }
 ```

## **5\. Functions**

* **Definition**: Uses the func keyword.  

```{Prometheus}
  func int add(int x, int y) {  
      return x \+ y;  
  }
```

* **Return types**: Every function requires a primitive return type. void is not supported.

## **6\. Standard Library**

### **I/O Functions**

* **print(...)**: Outputs values to the console.  
  * Supports multiple arguments separated by commas (adds a space between them).  
  * **Automatically adds a newline** after the statement is executed.

```{Prometheus}
print("The answer is:", 42); # Outputs: The answer is: 42
```

* **input(prompt)**: Displays a prompt and waits for user input.  
  * **Always returns a str**. Use casting to convert to other types.

```{Prometheus}
str name = input("Enter your name: ");  
int age = int(input("Enter your age: "));
```

## **Appendix**

* **Keywords List**: int, double, bool, str, if, elif, else, while, for, print, input, func, return.