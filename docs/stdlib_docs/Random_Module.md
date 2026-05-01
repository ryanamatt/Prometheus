# **Module: random**

The math module is part of the Prometheus Standard Library The random module provides utilities for generating pseudorandom numbers and managing random seeds. 

## **Functions**

### **Seeding & State**

#### **seed(int n) \-\> void**

* **Parameters:** n (int) \- The seed value to initialize the generator.  
* **Returns:** void  
* **Description:** Sets the seed for the pseudorandom number generator. Providing the same seed will produce the same sequence of numbers, which is useful for debugging or reproducible simulations.

#### **get\_seed() \-\> int**

* **Parameters:** None  
* **Returns:** int  
* **Description:** Returns the last seed value that was manually set using the seed function.

### **Random Number Generation**

#### **random() \-\> double**

* **Parameters:** None  
* **Returns:** double  
* **Description:** Returns a random floating-point number between 0.0 (inclusive) and 1.0 (exclusive).

#### **randint(int min, int max) \-\> int**

* **Parameters:** \* min (int) \- The lower bound.  
  * max (int) \- The upper bound.  
* **Returns:** int  
* **Description:** Returns a random integer between min and max, inclusive of both endpoints.

#### **uniform(double min, double max) \-\> double**

* **Parameters:**  
  * min (double) \- The lower bound.  
  * max (double) \- The upper bound.  
* **Returns:** double  
* **Description:** Returns a random floating-point number within the range \[min, max).

#### **choice\_bool() \-\> bool**

* **Parameters:** None  
* **Returns:** bool  
* **Description:** Returns a random boolean value (true or false) based on a 50% probability threshold.