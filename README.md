
# User_Threading

Introducing YAThreadLib!

This project offers an engine to run concurrent threads simultaneously. As seen in threading.cpp, the `Dispatcher` object takes as a parameter an array of function pointers. After constructing a `Dispatcher` with this parameter, one can call `Dispatcher.runThreads()` which will begin scheduling the functions to be executed concurrently. 


## Build

As of now, building is left up to the builder (I plan include a build system in the next commit -- that said, if you're onn and x86 system `g++ threading.cpp -lpthread` should get the job done). If I didn't mention before, the current state of this project is nowhere near production ready. A build note, though: an adress translation is needed for thread context in userspace -- this requires some assembly. As a result, only certain CPU architectures are supported -- currently, 32 and 64 bit x86.

This has been tested on a x86-64 system running Ubuntu. As of now, other systems are not gauranteed to work as they have not been tested.


## Disclaimer

The purpose of this project was mainly for self learning. I developed it as a mechanism to understand the internals of multi-threading. Additionally, it served me as an outlet to test features of C++. At this point, I am not sure if I will put in the effort to turn this into a production ready library as similar libraries exist. 
