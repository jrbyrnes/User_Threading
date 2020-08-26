# User_Threading

Introducing YAThreadLib!

The purpose of this project is largely selfish. I developed it as a mechanism to understand the internals of multi-threading. Additionally, it serves as an outlet to test features of C++. That said, I do intend to eventually turn this project into a full library that offers a rich API for multi-threading.

As is, the engine splits some user supplied functions into threads. These threads all belong to the same process, and all belong to the same kernel thread, sol, we are therefore computing concurrently -- not in parallel -- with this lirbary.

## Build

As of now, building is left up to buildee. If I didn't mention before, the current state of this project is nowhere near production ready. A build note, though: an adress translation is needed for thread context in userspace -- this requires some assembly. As a result, only certain CPU architectures are supported -- currently, 32 and 64 bit x86.

This has been tested on a x86-64 system running Ubuntu. As of now, other systems are not gauranteed to work as they have not been tested.
