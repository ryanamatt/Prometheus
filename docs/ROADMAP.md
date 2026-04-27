# **Prometheus Development Roadmap**

**Disclaimer**: The features listed below are not in any particular order of when they will be added to the language.

* **Multiple Assignment & Returns**: Support returning multiple values from a function and unpacking them.  
  * **Syntax**: return val1, val2; and type x, type y \= functionCall();  
  * **Example**:  
    func int, int getCoords() {  
        return 10, 20;  
    }  
    int x, int y \= getCoords();

* **Module System (Imports)**: Enable code reuse and library management.  
  * **Syntax**: import "filename"; for local files or import "libName"; for standard libraries.

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

## **Standard Library**

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

## ** Future / Extensions**

* **Daedalus ML Library**: High-performance Machine Learning library.  
  * Porting existing C++ implementation to the Prometheus runtime.  
  * Support for neural networks, linear regression, and data preprocessing.