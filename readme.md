## flang

flang is a functional language for data mangling.

It compiles to C, so it's 100% compatible with it and can import any library
you need that's written in that language.

The typesystem is lossly type, because it exists `unsafe_cast`, but it's
strong type if you don't use it. `unsafe_cast` is here to be compatible
with C way of doing things, not flang best practices.

## Project status

Under development, do not use it production.

The syntax is rather stable.

[Documentation](doc/)

---

### Develop flang

Wants to help?! Awesome! First install Clang (Gcc also works)

flang depends on [string.c](https://github.com/llafuente/string.c) and here [how to install string.c](https://github.com/llafuente/vagrant/blob/master/packages/stringc.sh)

There are some `sh`(s) that helps you in the development

    sh run.sh
    # generate the grammar and build
    # @build/flang is the executable

    sh test.sh
    # run all test, everything must pass, no skip, no flaky

    sh rerun.sh
    # just build and run something, this is mainly what you have to do
    # to develop, after this write a test
    # if you modify the parser grammar (BISON) use run.sh next time

    sh format.sh
    # This is the only requisite to merge
    # use clang-format to give all the project the same/consistent style

MSVS users. Replicate the sh above with some bat/ps1 file (piece of cake!)

### Usage

Pipeline usage: parser -> typesystem -> cprint -> clang compile

---

Flang is divided into a few libraries that can be swap with your custom
implementation if needed.

### libfl_ast

Functions to create and manage AST

### libfl_parser

Functions to parse files/string into AST with flang syntax.

### libfl_typesystem

Functions to validate and augment AST. At the end, everything has a type and
can be safely codegen.

### libfl_codegen_c

Functions to output a C file from AST.

*NOTE* flang_try_1_llvm branch has an experimental LLVM codegen, that it's
deprecated due the real complexity of having to deal with so many low level
staff. C compilers do that for us now.


# LICENSE

see LICENSE file
