# rsl2 and granite foundation

this is the second version of rsl2, a complete rewrite. active 1997-1999.

specifically, gf 2.5 b22


## what's this?

"granite foundation" was a web app server and programming language (RSL v2) that had very tight integration
between the dynamic language RSL and the implementation language, C++. It had a scalable process
architecture, double ref counted memory management and smart pointers in C++, template based code
generator, dynamically generated documentation

- core_rsl: startup files for different server applications
- cryptogrpahy: crypto integration
- granitecore: the language rsl and its runtime system and process architecture
- D: C++ integration, smart pointers
- nsapi: Netscape web server integration
- packages: various libraries (eg packages/templatizer is the code generator)

## original readme:


    How to make the GF2.5 system
    --- -- ---- --- ----- ------
    1) cd to the granite directory
    2) Determine which compilier to use
    3) makeit set
        respond with sun, cyg, or gnu
    4) makeit system 

