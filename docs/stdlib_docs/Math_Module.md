# **Module: math**

The math module is part of the Prometheus Standard Library. The Math Module provides fundamental mathematical constants and functions for trigonometric, logarithmic, and arithmetic operations.

**Note:** All trigonometric functions use **radians** for angles.

## **Variables**

| Name | Type | Description |
| :---- | :---- | :---- |
| PI | double | The ratio of a circle's circumference to its diameter (approx. ![][image1]). |
| TAU | double | The ratio of a circle's circumference to its radius (![][image2], approx. ![][image3]). |
| e | double | Euler's number, the base of natural logarithms (approx. ![][image4]). |

## **Functions**

### **Trigonometry & Geometry**

#### **sin(double x) \-\> double**

* **Parameters:** x (double) \- The angle in radians.  
* **Returns:** double \- The sine of x.  
* **Description:** Computes the sine of the given angle.

#### **cos(double x) \-\> double**

* **Parameters:** x (double) \- The angle in radians.  
* **Returns:** double \- The cosine of x.  
* **Description:** Computes the cosine of the given angle.

#### **tan(double x) \-\> double**

* **Parameters:** x (double) \- The angle in radians.  
* **Returns:** double \- The tangent of x.  
* **Description:** Computes the tangent of the given angle.

#### **asin(double x) \-\> double**

* **Parameters:** x (double) \- The sine value.  
* **Returns:** double \- The arcsine in radians.  
* **Description:** Computes the inverse sine of x.

#### **acos(double x) \-\> double**

* **Parameters:** x (double) \- The cosine value.  
* **Returns:** double \- The arccosine in radians.  
* **Description:** Computes the inverse cosine of x.

#### **atan(double x) \-\> double**

* **Parameters:** x (double) \- The tangent value.  
* **Returns:** double \- The arctangent in radians.  
* **Description:** Computes the inverse tangent of x.

#### **atan2(double x, double y) \-\> double**

* **Parameters:** \* x (double) \- The x-coordinate.  
  * y (double) \- The y-coordinate.  
* **Returns:** double \- The angle in radians between the positive x-axis and the point ![][image5].  
* **Description:** Computes the multi-quadrant inverse tangent.

### **Power & Logarithms**

#### **sqrt(double x) \-\> double**

* **Parameters:** x (double) \- The radicand.  
* **Returns:** double \- The square root of x.  
* **Description:** Computes the square root of the given value.

#### **log(double x) \-\> double**

* **Parameters:** x (double) \- The value.  
* **Returns:** double \- The natural logarithm (![][image6]).  
* **Description:** Computes the natural logarithm (base ![][image7]) of x.

#### **log10(double x) \-\> double**

* **Parameters:** x (double) \- The value.  
* **Returns:** double \- The base-10 logarithm.  
* **Description:** Computes the common logarithm of x.

#### **exp(double x) \-\> double**

* **Parameters:** x (double) \- The exponent.  
* **Returns:** double \- ![][image7] raised to the power of x.  
* **Description:** Computes the exponential function.

### **Arithmetic & Rounding**

#### **floor(double x) \-\> double**

* **Parameters:** x (double) \- The value.  
* **Returns:** double \- The largest integer less than or equal to x.  
* **Description:** Rounds x downwards.

#### **ceil(double x) \-\> double**

* **Parameters:** x (double) \- The value.  
* **Returns:** double \- The smallest integer greater than or equal to x.  
* **Description:** Rounds x upwards.

#### **abs(int n) \-\> int**

* **Parameters:** n (int) \- An integer.  
* **Returns:** int \- The absolute value of n.  
* **Description:** Returns the non-negative magnitude of an integer.

#### **fabs(double n) \-\> double**

* **Parameters:** n (double) \- A double-precision float.  
* **Returns:** double \- The absolute value of n.  
* **Description:** Returns the non-negative magnitude of a double.

#### **max(int a, int b) \-\> int**

* **Parameters:** a (int), b (int) \- Two integers.  
* **Returns:** int \- The larger of the two values.

#### **min(int a, int b) \-\> int**

* **Parameters:** a (int), b (int) \- Two integers.  
* **Returns:** int \- The smaller of the two values.

#### **fmax(double a, double b) \-\> double**

* **Parameters:** a (double), b (double) \- Two doubles.  
* **Returns:** double \- The larger of the two values.

#### **fmin(double a, double b) \-\> double**

* **Parameters:** a (double), b (double) \- Two doubles.  
* **Returns:** double \- The smaller of the two values.

#### **clamp(int val, int lo, int hi) \-\> int**

* **Parameters:** \* val (int) \- The value to clamp.  
  * lo (int) \- The lower bound.  
  * hi (int) \- The upper bound.  
* **Returns:** int \- The clamped value.  
* **Description:** Ensures val is within the range ![][image8].

### **Comparison & Logic**

#### **is\_even(int n) \-\> bool**

* **Parameters:** n (int) \- The integer to check.  
* **Returns:** bool \- true if n is even, false otherwise.

#### **is\_odd(int n) \-\> bool**

* **Parameters:** n (int) \- The integer to check.  
* **Returns:** bool \- true if n is odd, false otherwise.

[image1]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADgAAAAUCAYAAADY6P5TAAADXUlEQVR4Xu2WS0hVURSGr1hg9KKHiXrvPdcHWVCY3EpsEA2ksLBBVESjQKgGQWZQ4EQcRhlkk3DSC6lZo2oiYQlCNehBRJgNCmogVBQpFIh9/z17w3Z7zz1CE4P7w8/Ze61/7bPWPvtxEokiiihiQaC+vn5FOp0+HQTBQCaT6a2trV2PucTXFQKxTalUardvz4MS3tEJT/oOg1JyOYw/4ztk4z1Hq6urk9LRLaupqWlEf1xtV4u9wTYaCRpGsAvhKp4n6P+BZxMxRaJtJuYc2udwhvZ5X+NDMWgnXW0ymVxCvx3fFXyfjT/rxgnYd8DfepfDSbjPkWkCjxE/ZoP64TSzv199FRmECX+FG53AOTAFtqPbqxfFFcjKWInukRLzC8TWZia5x4w1p0DZ8L2F7+BL9L08K/NoVPSpnIFGn14IO9RvaGhYTnsE/tTXdYOjYAeNKbAETRe8WEgru/FHFXjVt7sIwg+mCWzPGbLZ7OKqqqq1NEvV50tuQvAdDpeXly9zg6MwnwLN0rwEWwpp/7VA3nNzVoEudNjgHMT5CeEW3x+FuALN0tQBlonTzqPAe/AWHFeesEdL3GryFlhRUbEUw10F4PyAaE/CfNH5ICZpbfoz+A6pE6ONLRA+paB69TlN1zD2M02eVqJs9I+oQHhgdrQBgg04v8DbKtz350OhpNnH2/H12QQKaYVCBWoMPye03UF4sraob1bLEPHXXZ0LHQaDmoVM9F01C1FJaw9ju6alaW1RWotCBeaD0c/ALmvjC69WDbkZ0fIR7Qy7QVrP1lYIUUnzos2M8Tod7pUc0U2YhH4ZW6sbE1WglmMQXl9v6urq1nn6/HewTcwf0G5U2G9t+vScsFWJPJd/VIH5EKeNKhBbJfzoFxiES3TG3uP6yyH2MTV0JrgeUjjfY7ihAozAblxdFU3WRvtV4Kx1F85Edfs+HySyDd1UlNYUOCWd51qUDpd7szUoZ/pP0A/Z/J1c7uRENNrgOMILmfAEug9/yG4H0l6i/4DgMQ6NwNq1R4PwQJpx+A2OuLMsqI99FE472gnGbLWnuIl1x9LYl+0YencQ/oToZ6EDvoCj5t80B5PrQzhgbUq0LBP+Jh1kkJ3uflxoUG7KUbkG4a/knOtMGlbAVt9exP+Gv1O+O9zADOidAAAAAElFTkSuQmCC>

[image2]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABMAAAAVCAYAAACkCdXRAAABoElEQVR4Xu2TMUgDQRBF71BBUVCQcJjc5XKHjZXFgTYKFqK9lYWF2NjYaApRELQ2CpJG0ljYWgREUGJlJVZ2tiqClRZC7CS+MbuwbhIJtjrw2Z0/f3/mdieO8x+/jjAMh8AWKOVyuR3f94dtTVsRRdEYJpUgCCbZj7I/AzWQp+za+pZBBz3ZbLbMwSXSDuEymcwg3d3AVakl1pHWoT7vAbxJVwa/qbpbM/U/RpIkXXRxQAcXYqx58nUxk9XUox1QDTTA1JnRSfEEfHB4Sgj5UfKC6rYpLI96YDBOsQpKYgLlwm2TF7mKkP08+92w3lGRfFb2to8Tx3E/hUtw7Hler3DpdDrgcxcc9bLUCuTTUmc94sH8byYS0gXFQ8T78sp2XYLOPOoVdLGY0NVVQ1faiOKGo0YE0QizN2Pq1CeesnbL2LC/tc1ciDyCVdlrknwZfk7naibP9Qsrs3tzpORyFyHfwROCRw3yV9YJLZR/CNyL5pSZPNTKl0BaDOtDW2uCZ7kbbabu81oeSXJ1f3dgT2vaDrmnVCrVZ3JirMbnr8QnFnF0MJcHWXIAAAAASUVORK5CYII=>

[image3]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADgAAAAUCAYAAADY6P5TAAAEG0lEQVR4Xu2WXYhVVRTHz2WmB9PQwcbB+dp37lhDoI0yiSjmS5b2MDIokaRvQYlIQpEa9TAPDmGK5KSVIYXIYAWDTw6DRA75UDngRygTpfjBkIwyBTETpEy33/+etW/bM6d7xV4M5g9/zllrr7X2WvusvfeJoilMYQoPDOrr66c5516Cn8Cuurq6+qRNGpqamlqwf8/83oBzkzYgk81mlzDWDT9sbGxsb2tre6iMzQs1NTXTEzZFZGOsT+o9lFdRIOAT8BwOb6kwBef9dHNz85zAZxLwWQd7Cdba0NDwNO9n4W24NjDLIL8JD2DnRLAb9uVyuZlmU8l8+5wtLHgM+RvkC7L3gSzPzfBrOIHNYT8WQnkx9mVBUBEKpOIQM1o15K/gOIW23e36D5i4BptT2KxEzEhHYvPQ/QJ/rq2tbZCOwucjH2tpaXnE+1q39PkvYImPIp+orq6eYboNMA+7vZ/ssOnguQwOpxWovMipv9iBCNsVXM7eCN0zOO9QIkXPBFQ8PuPwsoKaWl+rxxJ73uzaeT/DhLMD90jJoX9d79bmI/C8t+N9rcU5FPrZ2Fx4La1Ay+tHdUGkFkH4Dg4qsGhtWZF0TMJ8ewn4mV91wRLPqzDJPJcj31HL4/O42WTRf49+UeA3K4ijhdoP/1Kh3sajTIE5xm7AS8XWsCIPwLfhB/AizguTzuVA8Cp8B+GI3+Q6TFx8AOVdvD/3wlMuJXGDDptVjP8K96QcRiULBBny2Kb5wjYLV0oT7EIeuteT1AO/F/Gb0ASR7UuB/fgwY19YkeIQC/Bk4FqA9jO8zvhN7A/i92jSRihToFDBeFdY4MUwmO2bPAFeDb1KwU7HIXw6w1W3L7gHHnHx4aCTVkXqEFsexghQwdi72PymopODZQpUe29h7ISSakX4HQ6E+ygoMC3AJNh+PI7f1iixf4mxibGTPr4OLnQ7XPyl+//tIOP0XYzNH/BC8roqVSAxn5VP4RS3o/7yfynQF6f2jKwtibsCeaG/ctI6Af0WJalk1a48u8O29UW4lOuqVIHoDymWlysRjsJBglR5ZVqLqhBWtTYK9pbaD9v3XeLAQN6pL6BF430AbgjHBftC32prKFGbr5hwsH3uusKEUgVarK6iwj7prWCVJh0yuj6Qz8M/4VLpVBx2ndLhe90TeRhe5T0nOxf/vg0Efy1CYY/JX0JjfBffQl7tDdBtdPF18xFipdcLvkDYEwULLrBwa1x8C2QLCjsE3oFXCPaKi6+JYQyf8k72JfoY/0mHiXTBCudTWOwI7THeP3Zx0a/xfNnFf0qf+n9Na3N1kvQa16KMYf95uDDIK128gBPBXGPU8gPzLDAzLZ6uiVHvV4A2JYYdFPZcqZ/c+4WPL/rfuATUOUIpm3tG8s9pCv9H/A3wgmNkx/1CPwAAAABJRU5ErkJggg==>

[image4]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADgAAAAUCAYAAADY6P5TAAADV0lEQVR4Xu2WX4hMURzH77SriCKssbMzc2dm/WmTlBER8UJeUCjFJnlZDyi2iCgelChFkrYtJJusBx7I4s2L8KJsJB5oax+EklUetvX5zf2dcebsvXdu7Qs13/p1z/n+fud7fr97zj3nel4DDTTwT8D3/VbsJNZTKBROZ7PZeW5MGPL5/AHsLPHzVaNq7e3tcwhpsuOJm4L+HmyGzRsUi8WFjD0neaDbmU6np7oxSXMVLdNYTvCTXC63hvYS2g+wMawbd6p2WC2Y4IbGhtlgJpOZTQIzidtBwtfhvmOfsFZXC24bcXd4drS1tWWJP0j7JTn5JiZprkar8kYRugex19O3jfgsnC/gRvCVzSAXLS0t03SCW1iPMS1kWCaROC1wC0ktw3fbDymQZNP4HklhFp0i7jJjT0knaa41WjKRTvhD3ohRpn/cD97MYcO50LFXmGCyzctqwZ/3QlZfV3xcgZIc3DvZ6g5/FL5X2klzrdEql8uTmPQi5IA9qQqPydNwLmRliFlqc8QvgusrlUrTbd4gpsCSH6z6B2LWCicatB+z8pulnzRXW8vEuGjGeRcbRXCd64yCbtl+xqxwfQZRBYIUiR2RRDXZa9h9v/45EJZrVcsOrEISxDmC9chbc/1RQLSTMX00m12fQUyBgib4M6ZI7Aua672YAmNyrWhZ/QCyLXA8xW6GHdFR0HHPmbDL9dmIKVAOlP2yJXmu0tWTIkdp73ZiK4jJtaplccEeR+wqzgtyYtU46wCxjX5wkq12fTaiCpSVgnvDtZJTSlZgp2hib+VktOPjcg3R+juABI55egQT1MEHvqEaFANiL2mBkdeKIKpA+r2iYXMCfXHfbN16uYZpyZJ2M+CQtA1Jvwt+q+nLlkAkY8cIZHv4wVYZZuKS7XMRVaDy474ZucvgX0kBStXN1dVKQeyB+IUNkeBnY37w5ipbTi5U+q+x39hKM1ggfytwg2GJu9DJh5wL3ZOrQDTwFyza5GYOrkS51mhJQprYWIhVV0SvgIf03xetXyeBfB/4PqrOuALlnxT+GfbT0h7Fhshhn4Y15YOj/St2AtuF9cMNEDNXAkRb5zAattm7p6ql/YmDD31xMfi7iDzSk0B2CsltwrbzSSzwJqAnWi7XwP+GP+lzSVkl/E8oAAAAAElFTkSuQmCC>

[image5]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAC0AAAAYCAYAAABurXSEAAADZUlEQVR4Xu2WTUhUURiGRzSwP1LKBh1nro7QEAUVA5H0Q4uKoizahGC0KKpVEaRFUuDGRdEfEtjCkIKIaIJAioigoCApCFxUUgQZUbgKpIIW/Tyv91w7c+aOjgvHTS+8nHvO+51zvvPzfedGIv8xjaiqqppTV1dX7rYXC5OeH+PliUSiN5lMznO1YqG+vt4DGZWuloNYLFaL8SOcXuJqxUY8Hl+LH3cm2rwSjC6y0x2uME0oYQMv4dNxVxgDK1uK0aBKV5su4E8jfAPrXW0UWhHi3UkFwBQDnyrx6QVscTUFX7kchu2uFqCmpibOINtUqs79n4/9Fljt2o6HdDo9gwBbx1hpqqWWpOtZ6bRpt3vgdel2u4RqOESnpizBRwmLOoLWDfdg9xqegdeoH6N8hxMpt1MYFFRyALaacfYFGnOsp/4VNtp9zA14kkql5trtEtIIw5RrsgRf24h2MmJWyuBXqX+Ciz3/dL6bXZsQ9D2IfUttbW2M8oMd9NQ7Na4ymNVF8zfRPuS5J6pJ4cewyWk7zA4t0nc0Gp1N54cwQ7UMbTUT74g4R5oHZfQ7pcnp00z5wzO7al3PnJgyTn+hTNrt4zptQx3NAPnT0MSQ8zdgf5CDg3G9kJgyTo9wBZe5QkFOkw63M8DPRMg1KhTB1YCdQZvnB/Qvxt1g2wrG6dDroZV+VucswU/we9Ev63+A7y5NqIklNjQ0LES7QEaZFXTgeCvEf0NkQxvj+XEwFvRagBd2BXytBb5np6NZApMuQHilQLHbZagOcEAZgvIlfKwFRPwFtWlQy142w54fqKEPgo4ZbSRwmrrH91sv5D4LtLfn05QjexG77EblVJMt+tDvw0Oen+wz8DbaCdkE9ubfZQD+tnfShuzRuj3/pbsCB+GfRHic6P5n8mijx9oMnyf8BG+jVNfAWqlbzwFj7Pdyr9oolIHMZlRoHOx2elYmsUGbfve0uBxtFOaFe8pgm11tktDunNWErkDbCs9/QHpUN//ND6j3ESczXXsWvxv9pn2aOTDZ4VbYAIVCi4anI+6zGxnLEt/gVs1BeV6n6z4ogtnEe9z5la7mIgiuNn27YgHQg7Mr3z+weZzOwWewXzGSZ4P069CBzVF9u2IOdBQYt9JplasVC5z4JhZ0IFKIw/8xhfgLQcrdzV7ZzA8AAAAASUVORK5CYII=>

[image6]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAABAAAAAXCAYAAAAC9s/ZAAAA2UlEQVR4XmNgGDaAUVpaWlhUVJQHXYIgkJeXzwbi/yAsJydXji5PFJCRkVEFGvCKbAOAmiWB+OHgMcDY2JhVUVFRHIjNFBQUHEB8BQgIAKoxBiphxmsASCWQfRAauCeA7BlAughIxwHpu0A8C2QoTgOggBEothSI/wHN80BSWwXEb4FYE64ShwEglywEil+VkpISgYmB1ADFvkK9AgEEDDiAnMCGlgGwQDysrq7OCxPEMABoSwZQ4Is8ND8A8XFg0raXh0QXTOwLUEMMSA5J7BcQ98GtGwUDCABDr2EJmyv0mAAAAABJRU5ErkJggg==>

[image7]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAkAAAAYCAYAAAAoG9cuAAAAyElEQVR4XmNgGAVkAUVFRXE5OTlfIG1nbGzMiiKppKSkJi8vfwKIlysoKEQA6WogXi0jI8MJ060PFHgNlKwEchlBYioqKqJAsa0gkxlAKoGczUD8BIgVoZrEgeypQFwK1gQ15RMQ/wK65RHUyi4g2xhmKgOQ4wIU/Aeky8EC2ICsrKwpUNE3kI/Q5aSkpLiAFDPIV/xQK1qRFUA1L5eWlhYGCwB9ZQAUuADEq4F4FkgT0OR+cXFxbmSNIMCsrKwsBsIgNrrk8AUAaiwqfp9Z45kAAAAASUVORK5CYII=>

[image8]: <data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADUAAAAYCAYAAABa1LWYAAADWElEQVR4Xu2XO2hUQRSGbzCCLzCicTX7uLsbm6CSYkFQJKSIpRLSKMROiE0qQ9RGsBFEFIwgNrGwCFoEFDS+sBAUsVQbGwNRghYiVgpRkvj9e2dgMnt3s1c3IUJ++Jm558ycOWfmnNnZIFjF/4WmdDq9NQzDnWr17Q9YaSiVSmsLhUJKPufz+XW+Pmhtbd2EcgKOwWF3kJ0cO7ExKG8o9lt8RS3kI4zAp7lc7rCvLweF8paitrK2trYNDL6DbB7O0e9x5zQC2L1s7IunfL0PfNzBuCG1VqaA6g7KAtkFOM1uZnxdI4DtPvgbxw76Oh+MG/U3OHFQSrkwSssJ9V1do4Dta3Aqk8mkfZ0PyoCh4YHAqfnEQTG4iOyLTsuVNwqmlp/DcT6bfX09+JugepD9qlZPOj3YDXupwayvXwzY7oDfsH9Gl5Gco+3S5eSOM5dVF+wMvJs5cVA6IZ2UTsyVaxHkw/ANugHafvgWDgUJfg6y2ewR5szCZ9i5gQ/H6L+C90nH9RpTLBY38z2G7iztFDzh2kgUVLV6UkByAPl75biVh1HBf8fRPVa2GMKonmaxfzQwm6FTQ/bR+oLuJP1+1ZyC4vu8ayNRUDqdMKaejPOz6I+7chlGPh+7QAycerrrppvWg5NKRz6b6Z+TX+YUf8L9jpnEQVXUk3N6FSlpdrTuoEKnnqysxsWh4G7D10pHR54sqDCmnqQPo9Twr3gtOg6nYcGRV4Wpp5mc8/tEv4TsB+x3x4ZmA+CgKxfqDso5ETnfwqSL3G7bzFtrUmPd+eh3I/9KezUwtSGbtd6RYbRpC36fVC92Y7B1CA6YsYMmqA7GdFu5UHdQ1nktrMKnvR5E6aATGcXIY3s7pVKpjbnoOfXIpoZ5HOs2nAm9GhDi0syV6YmGPyOsscs8155YOe0lbaK1VXdQoIn+FfgBPiDIfXasnkvIXsIX8Cbz3tEO2yAF4+BDOCedlVvo1gSfw4Xp1IRzp82a49jttQq+hyRHf4+2z5mTKCih/HqWzpFZlHXt7e3b6a/xlRa56LKpqIMgCmBLEDNX6R63ZjV50qD+GWbnK9KvkVjWoEixTuyNumm5FFjWoLTQUv1dcVE1KHuDwU9q9e2PWWkgA/bqkjI+xz64V7HS8Qe4ehhoSVqZDwAAAABJRU5ErkJggg==>