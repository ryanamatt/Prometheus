# **Prometheus Development Roadmap**

**Disclaimer**: The features listed below are not in any particular order of when they will be added to the language.

## **1\. Core Language Enhancements**

* **Void Return Type**: Introduce void as a valid return type for functions that do not return a value.  
* **Lists**: Implement a generic list collection.  
  * **Syntax**: list\[type\] name \= \[elements\];  
  * **Examples**:  
    * list\[int\] nums \= \[1, 2, 3\];  
    * list\[int\] nums \= \[\];  
* **List Methods**:  
  * name.add(element): Appends an item to the end of the list.  
  * name.pop(): Removes and returns the last item.  
  * name.len(): Returns the number of elements in the list.  
* **Optional Function Parameters**: Allow parameters to have default values.  
  * **Syntax**: func type name(type param \= value) { ... }  
  * **Example**:  
    func int calculate(int a, int b \= 10\) {  
        return a \+ b;  
    }

* **Multiple Assignment & Returns**: Support returning multiple values from a function and unpacking them.  
  * **Syntax**: return val1, val2; and type x, type y \= functionCall();  
  * **Example**:  
    func int, int getCoords() {  
        return 10, 20;  
    }  
    int x, int y \= getCoords();

* **Module System (Imports)**: Enable code reuse and library management.  
  * **Syntax**: import "filename"; for local files or import "libName"; for standard libraries.

## **2\. Control Flow**

* **For Each Loop**: Simplified iteration over collections.  
  * **Syntax**: for (type item in listName) { ... }  
  * **Example**:  
    for (int n in nums) {  
        print(n);  
    }

* **Error Handling (Try/Catch/Throw)**: Robust mechanism for catching and raising runtime errors.  
  * **Syntax**:  
    try {  
        \# problematic code  
        if (x \< 0\) {  
            throw "Value cannot be negative";  
        }  
    } catch (str error) {  
        print("Caught error:", error);  
    }

## **3\. Standard Library**

* **Range Function**: Generates a list of integers for iteration.  
  * **Syntax**: range(start, end)  
  * **Usage**: for (int i in range(0, 10)) { ... }  
* **Math Library (math)**:  
  * Core mathematical functions: sqrt(x), sin(x), cos(x), tan(x).  
  * Constants: PI, E.  
* **File I/O Library (io)**:  
  * Functions for interacting with the filesystem:  
    * open(path, mode): Returns a file handle.  
    * read(handle): Reads content.  
    * write(handle, content): Writes content to file.  
    * close(handle): Closes the stream.  
* **Time Library (time)**:  
  * now(): Returns current system timestamp.  
  * sleep(ms): Pauses execution for specified milliseconds.  
  * format(timestamp, formatStr): Formats time for output.  
* **JSON Library (json)**:  
  * parse(jsonStr): Converts a JSON string into Prometheus lists/primitives.  
  * stringify(data): Converts Prometheus data structures into a JSON string.  
* **OS Library (os)**:  
  * args: A list of command-line arguments passed to the script.  
  * getenv(name): Retrieves environment variables.  
  * exit(code): Terminates the program with a status code.

## **4\. Future / Extensions**

* **Daedalus ML Library**: High-performance Machine Learning library.  
  * Porting existing C++ implementation to the Prometheus runtime.  
  * Support for neural networks, linear regression, and data preprocessing.