# toy_lexer.h

This header defines the structure `Toy_Lexer`, which can be bound to a piece of source code, and used to tokenize it within a parser.

## Defined Functions

### void Toy_initLexer(Toy_Lexer* lexer, const char* source)

This function initializes a lexer, binding it to the `source` parameter; the lexer is now ready to be passed to the parser.

