# cppi - cpp inspector
[WIP, not usable yet]
C++11 parser for reflection or other tooling.

This is a work-in-progress.
The goal is to parse only declarations and their [[attributes]]. Function bodies and expressions are ignored whenever possible.
Implemented using a draft of C++11 standard.

This library is not 100% standard compliant and there's no goal to make it so. If it works - it works.

This is a work in progress. Not yet usable.

# Limitations
- Only ASCII source
- Trigraphs are not supported

# Usage
```
cppi::context ctx;
ctx.parse("my_souce_file.hpp");
```
