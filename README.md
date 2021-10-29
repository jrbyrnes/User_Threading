
# User_Threading

Introducing YAThreadLib!

This project offers an engine to run concurrent threads simultaneously. As seen in threading.cpp, the `Dispatcher` object takes as a parameter an array of function pointers. After constructing a `Dispatcher` with this parameter, one can call `Dispatcher.runThreads()` which will begin scheduling the functions to be executed concurrently. 


## Build

Adress translation is needed to get correct addresses for the functions' stack -- this implies an architecture dependency. Due to limited resources, only certain CPU architectures are supported -- x86_64. Note: the code can easily support 32 bit Intel architectures, however it has not been tested.

This has been tested on a x86-64 system running Ubuntu. 

To play with the code, clone it, then navigate to the main directory "USER_THREADING". Then

```
mkdir build && cd build
cmake ..
make
```

If using an x86_64 architecture, an executable "Threading Example" will be created. Running this will demonstrate how the threading library works.


## Disclaimer

The purpose of this project was mainly for self learning. I developed it as a mechanism to understand the internals of multi-threading. Additionally, it served me as an outlet to test features of C++. At this point, I am not sure if I will put in the effort to turn this into a production ready library as similar libraries exist. 
