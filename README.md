# rsl2 and granite foundation

(defunct)

this is second version of RSL, a complete rewrite. active 1997-1999. 
[RSL-1](https://github.com/russellholt/rsl-1) was a very different beast.

this is a snapshot of version `2.5-b22`

## what's this?

"granite foundation" was a web app server and programming language (RSL v2) that had very tight
integration between the dynamic language RSL and the implementation language, C++. It had a scalable
process architecture, a JSON-like object serialization interface, template based code generator,
dynamically generated documentation, double ref counted memory management and smart pointers in C++

### what's here

- core_rsl: startup files for different server applications
- cryptogrpahy: crypto integration
- doc: some documentation
- granitecore: the language rsl and its runtime system and process architecture
- D: C++ integration, smart pointers
- nsapi: Netscape web server integration
- packages: various libraries (eg packages/templatizer is the code generator)

### what might be interesting here?

- the lexer, parser, building an AST
- low level C++ integration, loading shared libraries and instantiating C++ objects of uknown type
- code generator, code rewriter
- high level C++ integration between the dynamic language RSL and C++, sharing methods and data
  members
- inheritance and method dispatch
- formal parameters with expected values can define a method
- the fact that a lot of low level operations can be written in C++ directly but can manipulate
  objects directy accessible from RSL

### what kinda sucks?

the core language is fun and interesting, but IMHO it is too traditional, not boundary pushing.
Looks too much like C++. too many compromises to look and feel like C++ or Java. public, private,
protected come to mind.
 
the c++ integration is interesting but the D thing (note that there are a lot of projects called D,
this one is mine) may have taken it a bit too far.

### what's not here, what doesn't work

- this was built using Rogue Wave
  [Tools.h++](http://www.roguewave.com/portals/0/products/legacy-hpp/docs/tlsref/index.html), a set of C++ data structures, which I think was meant
  to be compatible with the then in-progress but not yet standardized C++ stl (as of 1996-97). So
  maybe it could work with some minor modifications. I haven't tried.

- the shared library system might not work on modern Linux systems, it was simultaneously developed
  under Slackware and SunOS 4, and later ported to Solaris and Irix, using g++, cygwin, and Sun compilers.
  Who knows.

- crypto requires RSA products, not here

- nsapi obviously required the old Netscape server

- no applications, demo code, client specifics, etc.


### if all those issues were solved, what could this be used for?

- it could probably be a runtime to run programs written in plain RSL, or a combination of RSL and
C++, standalone, as a single threaded server, or as a multiprocess cluster.

- one could write an interface to a web server and write web apps.

- the ECI interface could be slightly modified to serve JSON. The syntax has one minor difference,
  objects have a classname associated with them, there are no plain objects. That also could be
  easily changed.

## original readme:


    How to make the GF2.5 system
    --- -- ---- --- ----- ------
    1) cd to the granite directory
    2) Determine which compilier to use
    3) makeit set
        respond with sun, cyg, or gnu
    4) makeit system 

## other versions


