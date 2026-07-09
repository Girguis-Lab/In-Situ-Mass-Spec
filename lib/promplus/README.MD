## **PROMPLUS Library** for Arduino

**README Written by:** _Christopher Andrews_.

**Modified by:** _Tyler Barnes_. 

### **What is the PROMPLUS library?**

The PROMPLUS.h library is a fork of the standard EEPROM.h library bundled in the Arduino core. The core library has been modified to perform redundant writes and reads, making sure that accessing the EEPROM always results in valid data, even if unexpected power loss has corrupted a location within EEPROM.   

It is recommended to always enable Brown Out Detection (BOD) when you are working with the EEPROM on your AVR project. Enabling BOD on AVR chips can help ensure EEPROM does not get corrupted. While it is very effective, it is not fullproof on it's own. 

Using this library in combination with BOD will yield rock solid EEPROM access in your next project. 

The interface is the exact same as the core library, all while utilizing redundant writes to ensure data validity. This library will work on many AVR devices like ATtiny and ATmega chips.

### **How to install it**

#### **Method 1** _(Recommended)_
To install the library, click on the `Sketch` tab within the Arduino IDE, click on `Include Library`, then click on `Manage Libraries...`. 

Once the library browser launches, type `promplus` into the search field, then click on the library's `Install` button. 

#### **Method 2**
Alternatively, the PROMPLUS.h library can be downloaded from the releases section of the [github repository](https://github.com/Tyler-Barnes/promplus). 

Once the .zip file has been downloaded, click on the `Sketch` tab within the Arduino IDE, click on `Include Library`, then click on `Add .ZIP Library...`. 

The resulting dialog will allow you to locate the library within your computer's directories, and open it. 


### **How to use it**
To add its functionality to your sketch you'll need to reference the library header file. You do this by adding an include directive to the top of your sketch.

```c++
#include <PROMPLUS.h>

void setup(){

}

void loop(){

}

```

The library provides a global variable named `PROM`, you use this variable to access the library functions. The methods provided in the PROM class are listed below.

**By including the PROMPLUS.h library, you will automatically be including the standard EEPROM.h library as well, allowing access to the unmodified functionality of the original.**

### **Library functions**

#### **`PROM.read( address )`** 

This function allows you to read a single byte of data from the eeprom.
Its only parameter is an `int` which should be set to the address you wish to read.

The function returns an `unsigned char` containing the value read.

It should be noted that the address passed into the method and the actual address the data resides may differ as a result of the redundant write/reads in the modified library. The address value you pass into the method will in actuality be multiplied by the number of duplicates that the library writes. Since by default the library writes 3 total bytes for each requested write, the following 2 bytes after the modified address should, in theory, contain the exact same data. 

- Passing `0` into the address argument will access address locations `0`, `1`, and `2` on the EEPROM.
- Passing `1` into the address argument will access address locations `3`, `4`, and `5` on the EEPROM.  

When reading, each redundant value gets compared, and the value that repeats the most in memory is returned as valid data. 

#### **`PROM.write( address, value )`** 

The `write()` method allows you to write a single byte of data to the EEPROM.
Two parameters are needed. The first is an `int` containing the address that is to be written, and the second is a the data to be written (`unsigned char`).

When the method executes, it writes the desired value 3 times to the EEPROM to ensure data validity. 

This function does not return any value.

#### **`PROM.update( address, value )`** 

This function is similar to `PROM.write()` however this method will only write data if the cell contents pointed to by `address` is different to `value`. This method can help prevent unnecessary wear on the EEPROM cells.

This function does not return any value.

### **An important note on how indexing works**

In the standard EEPROM.h library, the address you pass into the methods will be the actual address accessed wthin the EEPROM. 

In this modified library, the address that you pass in will be multiplied by the number of redundancies that it is designed to write/read. 

In PROMPLUS.h: 
- The address `0` is equal to `0x00` in EEPROM.
- The address `1` is equal to `0x03` in EEPROM.
- etc...

If for some reason you want to increase the number of redundant writes that the library performs, you can do so. Please see the note at the bottom of this file regarding `PROM.duplicates`.  


#### **`PROM.put( index, object, <address> )`** 

This function will write any object to the EEPROM.

Two parameters are required to call this function, and a third is optional. 

- The first argument is an `int` and controls the index. It will increment through memory at the width denoted by the datatype that you pass into argument 2. This allows you to use this index to iterate through data as if it was an array of the same datatype. 
- The second argument is the data object you'd like to write. It can be datatypes larger than or equal to `uint8_t` like `uint16_t`, `double`, etc. 
- The third and optional argument is an `int` containing the base address that the method will start from. The default value is `0`.  This `address` argument follows the same rules that the address of `PROM.write()` and `PROM.read()` follow; it is multiplied by the number of redundancies that the library writes to the EEPROM. So, the address accessed by `PROM.read(2)`, is the same address accessed by `PROM.get(0, val, 2)`.  

This function uses the _update_ method to write its data, and therefore only rewrites changed cells.

This function returns a reference to the `object` passed in. It does not need to be used and is only returned for convenience.

#### **`PROM.get( index, object, <address> )`** 

This function will retrieve any object from the EEPROM.
The parameter list is the same as `PROM.put()`.

This function returns a reference to the `object` passed in. It does not need to be used and is only returned for convenience.

### **An important note about indexing with the `PROM.put()` and `PROM.get()` methods**

In the standard EEPROM.h library, the index you pass in is the actual address within the EEPROM. This in my opinion can lead to issues if you are expecting the indexing to act like an array and respect the width of the given datatype. 

I have modified the behavior to index based on the `sizeof()` the datatype you pass in. 

I did this so that it behaves just like indexing into an array of a given type. This means that you will not have to keep track of the `sizeof()` your datatypes when you want to iterate through the data as if it were an array. 

However, just like an array, you will need to keep the datatype consistant throughout to keep the index aligned.   

You can expect these two examples to index the exact same way. 
```c++
uint16_t array[5]; 

uint16_t number = 1000; 

for (int i = 0; i < 5; i++) {
  array[i] = number * i; 
}
```
```c++

uint16_t number = 1000; 

for (int i = 0; i < 5; i++) {
  PROM.put(i, number * i);
}
```

The resulting data for either of the above examples will equal:
```c++
{0, 1000, 2000, 3000, 4000}
```

In this example: 
- The index of `0` points to the address `0x00` in the EEPROM. 
- The index of `1` points to the address `0x06` in the EEPROM. 

This is because each write will write three times for redundancy, and the datatype being written is 2 bytes wide. 

`2 * 3 = 6.`

The memory dump of the `PROM.put()` example looks like: 
```bash
0x0000: 00,00,00,00,00,00,E8,E8,E8,03,03,03,D0,D0,D0,07
0x0010: 07,07,B8,B8,B8,0B,0B,0B,A0,A0,A0,0F,0F,0F,FF,FF
0x0020: FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF
```

The only other thing I would add to the above example is a base address since you may not always want to start indexing from address `0x00` in the EEPROM. We will add the address we want to start from to the third argument field.

```c++

uint16_t number = 1000; 

for (int i = 0; i < 5; i++) {
  PROM.put(i, number * i, 0x01); // base address can be anything you want
}
```

```bash
0x0000: FF,FF,FF,00,00,00,00,00,00,E8,E8,E8,03,03,03,D0
0x0010: D0,D0,07,07,07,B8,B8,B8,0B,0B,0B,A0,A0,A0,0F,0F
0x0020: 0F,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF,FF
```

#### **Subscript operator: `PROM[address]`** 

This operator allows using the identifier `PROM` like an `unsigned char` array.  
EEPROM cells can be read _and_ **_written_** directly using this method.

The only valid datatypes for use with this operator are `unsigned char`, or `uint8_t`. 

This operator returns a reference to the EEPROM cell.

```c++
uint8_t val;

//Read first EEPROM cell.
val = PROM[0];

//Write first EEPROM cell.
PROM[0] = val;

//Compare contents
if(val == PROM[0]){
  //Do something...
}
```

#### **`PROM.length()`**

This function returns an `unsigned int` containing the number of cells in the EEPROM. The value returned is divided by the number of duplicate writes that the library utilizes. 

If the total number of byte sizes cells in the EEPROM is 1024, then the PROM.length() call will yield the result 341, since the default number of redundant writes in the library is 3. Note that the value returned is the result of integer math, and any float remainders are truncated. 

---

### **Advanced features**

This library uses a component based approach to provide its functionality. This means you can also use these components to design a customized approach. Two background classes are available for use: `PRef` & `PPtr`.

#### **`PRef` class**

This object references an EEPROM cell.
Its purpose is to mimic a typical byte of RAM, however its storage is the EEPROM.
This class has an overhead of two bytes, similar to storing a pointer to an EEPROM cell.

```C++
PRef ref = PROM[10]; //Create a reference to 11th cell.

ref = 4; //write to EEPROM cell.

unsigned char val = ref; //Read referenced cell.
```

#### **`PPtr` class**

This object is a bidirectional pointer to EEPROM cells represented by `PRef` objects.
Just like a normal pointer type, this type can be dereferenced and repositioned using 
increment/decrement operators.

```C++
PPtr ptr = 10; //Create a pointer to 11th cell.

*ptr = 4; //dereference and write to EEPROM cell.

unsigned char val = *ptr; //dereference and read.

ptr++; //Move to next EEPROM cell.
```

#### **`PROM.begin()`**

This function returns an `PPtr` pointing to the first cell in the EEPROM.  
This is useful for STL objects, custom iteration and C++11 style ranged for loops.

#### **`PROM.end()`**

This function returns an `PPtr` pointing at the location after the last EEPROM cell.  
Used with `begin()` to provide custom iteration.

**Note:** The `PPtr` returned is invalid as it is out of range. In fact the hardware causes wrapping of the address (overflow) and `PROM.end()` actually references the first EEPROM cell.

#### **`PROM.duplicates`**

If for whatever reason you would like to increase the number of redundant writes, then you can change the member variable `PROM.duplicates`. Note that you need at least three for the library to properly be able to compare each stored value and determine which value is valid. The default value is `3`. 

```c++
PROM.duplicates = 4; 
```