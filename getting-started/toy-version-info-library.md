# Toy Version Info Library

The toy_version_info library simply provides version info about the current build of Toy.

The toy_version_info library can usually be accessed with the `import` keyword:

```
import toy_version_info;
import toy_version_info as toy_version_info; //can be aliased
```

## Defined Variables

### major

This variable is the major version number of Toy at the time of compilation.

### minor

This variable is the minor version number of Toy at the time of compilation.

### patch

This variable is the patch version number of Toy at the time of compilation.

### build

This variable is a string representing the date and time that the interpreter was compiled.

### author

This variable contains the name of Toy's lead author, and his game studio.
