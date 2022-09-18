# Types

The type system in toy is opt-in, but allows a lot of robust checks at runtime when needed. Types themselves are first-class citizens. To retreive the type of an existing variable, use the `typeof` keyword.

```
print typeof value;
```

The types available are:

| Type | Signature | Description |
| --- | --- | --- |
| null | null | Represents a lack of any meaningful value |
| boolean | bool | Either true or false |
| integer | int | Any whole number. The limits are implementation dependent |
| float | float | Any floating point number. The limits are implementation dependent |
| string | string | A series of characters, forming text |
| array | n/a | A series of values arranged sequentially in memory, indexable with an integer |
| dictionary | n/a | A series of key-value pairs stored in a hash-table, indexable with the keys |
| function | fn | A chunk of reusable code, which can potentially return a value of some kind |
| type | type | The type of types |
| any | any | Can hold any value |

## Specifying Types For Variables

To specify a type for a variable, use `:` followed by the signature. In this example, the variable `total` can only ever hold integers (or  `null`):

```
var total: int = 0;
```

To specify the type of an array or dictionary, use some variation of these signatures:

```
var array: [int] = [1, 2, 3]; //an array of integers

var dictionary: [string : int] = ["key":42]; //a dictionary of key-value pairs
```

Complex, hard-to-write types can be stored in variables, like so:

```
//define a variable called "entry"
var entry: type = astype [string: [string]];

//define a phonebook which follows the above signature
var phonebook: entry = [
    "Lucy": ["1234", "Cabbage Ln"],
    "Bob": ["5678", "Candy Rd"]
];
```

## Const

Const-ness, or the ability to fix the value of a variable, is part of the type system. To define a constant, follow the type signature with the `const` keyword:

```
var ANSWER: int const = 42; //answer will never change
```

You can also set the members of an array or dicitonary as const, or the entire compound:

```
var members: [int const] = [1, 2, 3]; //1, 2 and 3 cannot be changed, but "members" can be modified or re-assigned

var everything: [int] const = [4, 5, 6]; //everything is now const
```

## Astype

Due to the syntax of Toy, when storing a complex type into a varable, you may need to use the `astype` keyword to differentiate the value from an array or dictionary.

```
var t: type = astype [int]; //t is a type, representing an array of integers
var u: type = [int]; //Error! it tried to assign an array with the sole entry "int"
```

## First-Class Citizens

Types are first-class citizens. What this means is that they can be used just like any other value, as well as being stored in variables, and even returned from functions.

```
fn decide(question) {
    if (question) {
        return int;
    }
    else {
        return float;
    }
}

var t = decide(true);

var number: t = 0; //what if it had been false?
```
