## experimental fork of flang


### libast_fl

Functions to create and manage AST

### libparser_fl

Functions to parse files/string into AST

### libtypesystem_fl

Functions to validate and augment AST

### libcprint_fl

Functions to output a C file from AST

### main.c

Pipeline usage: parser -> typesystem -> cprint -> clang compile
