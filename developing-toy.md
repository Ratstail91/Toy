# Developing Toy

Toy's current version began as a specification, which changed and evolved as it was developed. The original specification was extremely bare-bones and not intended for a general audience, so this website is actually intended not just to teach how to use it, but also aspects of how to expand on the current canonical version of the language.

Here you'll find some of the implementation details, which must remain intact regardless of any other changes.

## Bytecode Header Format

Every instance of Toy bytecode will be divvied up into several sections, by necessity - however the only one with an actual concrete specification is the header. This section is used to define what version of Toy is currently running, as well as to prevent any future version clashes.

The header consists of four values:

* TOY_VERSION_MAJOR
* TOY_VERSION_MINOR
* TOY_VERSION_PATCH
* TOY_VERSION_BUILD

The first three are single unsigned bytes, embedded at the beginning of the bytecode in sequence. These represent the major, minor and patch versions of the language. The fourth value is a null-terminated string of unspecified data, which is *intended* but not required to specify the time that the langauge's compiler was itself compiled. The build string can also hold arbitrary data, such as the current maintainer's name, current fork of the language, or other versioning info.

There are some strict rules when interpreting these values (mimicking, but not conforming to semver.org):

* Under no circumstance, should you ever run bytecode whose major version is different - there are definitely broken APIs involved.
* Under no circumstance, should you ever run bytecode whose minor version is above the interpreter's minor version - the bytecode could potentially use unimplemented features.
* You may, at your own risk, attempt to run bytecode whose patch version is different.
* You may, at your own risk, attempt to run bytecode whose build version is different.

All interpreter implementations retain the right to reject any bytecode whose header data does not conform to the above specification.

The latest version information can be found in [common.h](https://github.com/Ratstail91/Toy/blob/0.6.0/source/common.h#L7-L10)

## Embedded API

The functions intended for usage by the API are prepended with the C macro `TOY_API`. The exact value of this macro can vary by platform, or even be empty.

In addition, the macros defined in [literal.h](https://github.com/Ratstail91/Toy/blob/0.6.0/source/literal.h) are available for use when manipulating literals. These include:

* `IS_*` - check if a literal is a specific type
* `AS_*` - use the literal as a specific type
* `TO_*` - create a literal of a specific type
* `IS_TRUTHY` - check if a literal is truthy
* `MAX_STRING_LENGTH` - the maximum length of a string in Toy (can be altered if needed)

