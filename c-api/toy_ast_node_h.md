# toy_ast_node.h

This header defines the structure of the nodes used in the Abstract Syntax Tree, known as `Toy_ASTNode`. Most of what is defined here is intended for internal use, so is not documented here.

This header doesn't need to be included directly, as it is included in [toy_parser.h](toy_parser_h.md).

## Defined Functions

### void Toy_freeASTNode(Toy_ASTNode* node)

This function cleans up any valid instance of `Toy_ASTNode` pointer passed to it. It is most commonly used to clean up the values returned by `Toy_scanParser`, after they have been passsed to `Toy_writeCompiler`, or when the node is an error node.

