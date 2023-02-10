# Compound Library

The compound library contains a number of common utility functions for use with compound values, such as arrays, dictionaries and strings. These functions will allow you to manipulate these values in ways that would otherwise be quite difficult and inefficient using just Toy.

The compound library is currently under development.

The compound library can usually be accessed with the `import` keyword:

```
import compound;
```

## _concat(self, other)

This function only works when self and other are matching compounds (both arrays, dictionaries or strings). It returns a new compound of that kind, with the content of `other` appended to the content of `self`.

## _containsKey(self: dictionary, key)

This function returns `true` if `self` contains the given `key`, otherwise it returns false.

## _containsValue(self, value)

This function returns `true` if `self` contains the given `value`, otherwise it returns false.

## _every(self, func: fn)

This function takes either an array or a dictionary as the `self` argument, and a function as `func`. The argument `func` must take two arguments - the first is the index/key of the array/dictionary, and the second is the value. The contents of `self` are passed into `func`, one element at a time, until `func` returns `false`, at which point this function returns `false`. Otherwise this function returns `true`.

## _forEach(self, func: fn)

This function takes either an array or a dictionary as the `self` argument, and a function as `func`. The argument `func` must take two arguments - the first is the index/key of the array/dictionary, and the second is the value. The contents of `self` are passed into `func`, one element at a time.

```
import compound;

fn p(i, x) {
    print x;
}

var a = [1, 3, 5];

a.forEach(p); //prints 1, 3, and 5 to stdout
```

## _filter(self, func: fn)

This function takes either an array or a dictionary as the `self` argument, and a function as `func`. The argument `func` must take two arguments - the first is the index/key of the array/dictionary, and the second is the value. The contents of `self` are passed into `func`, one element at a time, and the function returns a new compound for every element that `func` returned a truthy value for.

## _getKeys(self: dictionary)

This returns an array of all non-null keys stored within the dictionary. The order is undefined.

## _getValues(self: dictionary)

This returns an array of all values with non-null keys stored within the dictionary. The order is undefined.

## _map(self, func: fn)

This function takes either an array or a dictionary as the `self` argument, and a function as `func`. The argument `func` must take two arguments - the first is the index/key of the array/dictionary, and the second is the value. It returns an array with the results of each call - the order of the results when called on a dictionary are undefined.

```
import compound;

fn increment(k, v) {
    return v + 1;
}

var a = [1, 2, 3];

print a.map(increment); //prints [2,3,4];
```

## _reduce(self, default, func: fn)

This function takes either an array or a dictionary as the `self` argument, a default value, and a function as `func`. The argument `func` takes three arguments - the first is the accumulator, the second is the index/key and the third is the value. It applies the given function to every element of the array/dictionary, passing the result of each call as the accumulator to the next (the default value is used for the first call). Finally, the final value of the accumulator is returned to the caller.

```
import compound;

fn f(acc, k, v) {
	return acc + v;
}

var a = [1, 2, 3, 4];

print a.reduce(0, f); //prints "10"
```

## _some(self, func: fn)

This function takes either an array or a dictionary as the `self` argument, and a function as `func`. The argument `func` must take two arguments - the first is the index/key of the array/dictionary, and the second is the value. The contents of `self` are passed into `func`, one element at a time, until `func` returns `true`, at which point this function returns `true`. Otherwise this function returns `false`.

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

## _trimBegin(self: string, trimChars: string = " \t\n\r")

This is identical to `_trim(self, trimChars)`, except it is only applied to the beginning of the first argument.

## _trimEnd(self: string, trimChars: string = " \t\n\r")

This is identical to `_trim(self, trimChars)`, except it is only applied to the end of the first argument.
