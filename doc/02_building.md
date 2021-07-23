# How to build this software {#building} #

As mentioned in the description of [How to use this software](@ref using), this software is intended to be run on a Raspberry Pi with special hardware or on any Windows or Linux computer with a USB optical probe.  


The software uses [CMake](https://cmake.org/) to simplify building the software.  After downloading the source code, the simplest way to create the software is to navigate to the `c12test` directory use these two commands:

    cmake -S . -B build
    cmake --build build

## Building documentation ## 

### HTML documentation ###

Building documentation is optional and off by default.  To build the documentation, additional software tools including [`Doxygen`](https://www.doxygen.nl/) are required.  To configure the build to enable this, include `-DBUILD_DOCS=ON` when generating the build scripts.  Specifically, to build *only* the documentation (and not the software) one could use these two commands:

    cmake -DBUILD_DOCS=ON -S . -B build
    cmake --build build -t doc

The `-t` specifies the build target which in this case is `doc`.  By default, this creates only the HTML version of the documentation.  To view the generated documentation open the `build/doc/html/index.html` file in a web browser.

### HTML documentation ###

To build the `pdf` version, first build the HTML version and then run this:

    cmake --build build -t pdf

The documentation will then be in a file: `build/doc/latex/refman.pdf`

## Building and running automated tests ## 

Building automated tests are also optional and off by default.  To build the tests, additional software tools including [`googletest`](https://github.com/google/googletest) are required.  To configure the build to enable this, include `-DBUILD_TESTING=ON` when generating the build scripts.  To build the tests, on must build the software first (which is the default target) and then build the tests.  The commands to do this are:

    cmake -DBUILD_TESTING=ON -S . -B build
    cmake --build build
    cmake --build build -t test

Note that it is possible to specify both `-DBUILD_TESTING=ON` and `-DBUILD_DOCS=ON` in the same build setup.

When the tests are run in this way, the output is very terse:

```
Running tests...
Test project /home/ejb/projects/epri/current/C12_compliance/repo/c12test/build
    Start 1: C12TableTests
1/1 Test #1: C12TableTests ....................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.00 sec

```

While it claims to only have run a single test, this is due to a quirk in the nomenclature used by the googletest tool.  In fact, it has run one test suite which includes over two dozen individual tests.  To run and see all of the tests, go into the build directory and run `ctest` in verbose mode:

    cd build
    ctest -V


## Further reading ##

[How to use the software](@ref using)
