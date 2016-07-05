## experimental fork of flang


### libfl_ast

Functions to create and manage AST

### libfl_parser

Functions to parse files/string into AST

### libfl_typesystem

Functions to validate and augment AST

### libfl_codegen_c

Functions to output a C file from AST

### main.c

Pipeline usage: parser -> typesystem -> cprint -> clang compile
