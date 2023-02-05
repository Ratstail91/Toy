# Compound Library

The compound library contains a number of common utility functions for use with compound values, such as arrays, dictionaries and strings. These functions will allow you to manipulate these values in ways that would otherwise be quite difficult and inefficient using just Toy.

The compound library is currently under development.

The compound library can usually be accessed with the `import` keyword:

```
import compound;
```

## _concat(self, other)

This function only works when self and other are matching compounds (both arrays, dictionaries or strings). It returns a new compound of that kind, with the content of `other` appended to the content of `self`.

## _getKeys(self: dictionary)

This returns an array of all non-null keys stored within the dictionary. The order is undefined.

## _getValues(self: dictionary)

This returns an array of all values with non-null keys stored within the dictionary. The order is undefined.

## _toLower(self: string)

This function returns a new string which is identical to the string `self`, except any uppercase letters are replaced with the corresponding lowercase letters.

## _toString(self)

This function returns a string representation of `self`. This is intended for arrays and dictionaries, but can theoretically work on any printable value.

If the resulting string is longer than `TOY_MAX_STRING_LENGTH` - 1, then it is truncated.

## _toUpper(self: string)

This function returns a new string which is identical to the string `self`, except any lowercase letters are replaced with the corresponding uppercase letters.

## _trim(self: string, trimChars: string = " \t\n\r")

This function returns a new string which is identical to the string `self`, except any characters at the beginning or end of `self` which are present in the argument `trimChars` are removed. The argument `trimChars` is optional, and has the following characters as the default value:

* The space character
* The horizontal tab character
* The newline character
* The carriage return character

These characters used because they are the only control characters currently supported by Toy.

