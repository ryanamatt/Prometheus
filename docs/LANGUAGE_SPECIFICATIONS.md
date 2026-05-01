# **Prometheus Programming Language Specification**

Welcome to the official documentation for **Prometheus**. This document serves as the central repository for the language's syntax, features, and standard library.

## **1\. Introduction**

Prometheus is a **procedural, interpreted language** designed for simplicity and directness. It focuses on clear syntax and predictable execution, making it an ideal choice for scripting and educational purposes.

## **2\. Getting Started**

* **Installation**: Prometheus is currently available as an interpreted environment. Run the binary followed by your script name.  
* **Hello World**: 

```{Prometheus}
  print("Hello World");
```

## **3\. Basic Syntax**

### **General Rules**

* **Semicolons**: Every statement must terminate with a semicolon ;.  
* **Blocks**: Code blocks are delimited by curly braces { }.  
* **Comments**: Single-line comments start with the \# symbol.

```{Prometheus}
  # This is a comment in Prometheus    
  int x = 1; # This is also a comment
```

### **Modules and Imports**

Prometheus supports modularity through two distinct keywords: import for local files and use for built-in standard modules.

* **User Imports**: import fileName;  
  Includes functions and variables defined in external .pm files.  
* **Standard Modules**: use moduleName;  
  Enables built-in language modules.

```{Prometheus}
  import myFunctions; # Import local file  
  use math;           # Use standard math module  
  use random;         # Use standard random module

  double root = sqrt(16); # Functionality from 'math'
```

See what the modules documentation here.

* **[Math Module Documentation](stdlib_docs/Math_Module.md)**
* **[Random Module Documentation](stdlib_docs/Random_Module.md)**

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
  * \*\* : Exponentiation (Power)  
* **Comparison Operators**:  
  * \==, \!=, \>, \<, \>=, \<=  
* **Unary Operators**:  
  * x++; (Increment)  
  * x--; (Decrement)  
* **Logical Operators**:  
  * &&, ||, \!

### **Data Types & Casting**

* **Primitive types**: int, double, bool, str, void.  
* **Complex types**: list\[type\].  
* **Type Casting**:  
  * int(value), double(value), str(value), bool(value).

### **Lists**

Lists are ordered collections of a single type. They use the syntax list\[type\].

* **Declaration & Initialization**:

```{Prometheus}
  list[int] nums;   
  list[int] scores = [];  
  list[str] names = ["Alice", "Bob", "Charlie"];
```

## **4\. Control Flow**

* **Conditionals**: if, elif, and else. Parentheses are required.  
* **Loops**:  
  * **While Loop**: while (condition) { ... }  
  * **Standard For Loop**: Requires a trailing semicolon in the update expression.

```{Prometheus}  
    for (int i = 0; i < 10; i++;) {    
        print(i);    
    }
```

  * **For-Each Loop**: Iterates over elements in a list or a list-returning function.

```{Prometheus}  
    for (int n : [1, 2, 3]) {  
        print(n);  
    }

    for (int i : range(5)) {  
        print(i);  
    }
```

## **5\. Functions**

* **Definition**: Uses the func keyword followed by the return type. 

```{Prometheus}
  func int add(int x, int y) {  
      return x + y;  
  }
```

* **Return types**: int, double, bool, str, void, or list[type].  
* **Void Functions**: May omit return or use return;.

## **6\. Standard Library**

### **I/O Functions**

* **print(...)**: Outputs values with a trailing newline.

```{Prometheus}
print("The answer is:", 42); # Outputs: The answer is: 42
```

* **input(prompt)**: Displays prompt and returns a str.

```{Prometheus}
str name = input("Enter your name: ");  
int age = int(input("Enter your age: "));
```

### **Utility Functions**

* **range(...)**: Generates list[int]. Overloads: range(stop), range(start, stop), range(start, stop, step).

### **Built-in Modules**

* **math**: Provides mathematical constants and functions (e.g., sqrt, sin, cos).  
* **random**: Provides random number generation utilities.

## **Appendix**

* **Keywords List**: int, double, bool, str, void, list, if, elif, else, while, for, print, input, func, return, range, import, use.