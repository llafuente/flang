
```bash
sh autogen.sh
./configure
make
```



## Compile process (done)

* read file to string
* tokenize the string

## Compile process (todo)

* parse tokens
  * generate AST
  * emit parse errors


## tokenizer

parse a file with the information found at `tokens.c`.
Will add a new line at the end to help parser.
EOF is considered a token, and will be added.
