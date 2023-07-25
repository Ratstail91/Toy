# Developing Toy

Here you'll find some of the implementation details.

## Bytecode Header Format

The bytecode header format must not change.

Every instance of Toy bytecode will be divided up into several sections, by necessity - however the first one to be read is the header. This section is used to define what version of Toy is currently running, as well as to prevent any future version/fork clashes.

The header consists of four values:

* TOY_VERSION_MAJOR
* TOY_VERSION_MINOR
* TOY_VERSION_PATCH
* TOY_VERSION_BUILD

The first three are single unsigned bytes, embedded at the beginning of the bytecode in sequence. These represent the major, minor and patch versions of the language. The fourth value is a null-terminated string of unspecified data, which is *intended* but not required to specify the time that the langauge's compiler was itself compiled. The build string can hold arbitrary data, such as the current maintainer's name, current fork of the language, or other versioning info.

There are some strict rules when interpreting these values (mimicking, but not conforming to [semver.org](https://semver.org/)):

* Under no circumstance, should you ever run bytecode whose major version is different - there are definitely broken APIs involved.
* Under no circumstance, should you ever run bytecode whose minor version is above the interpreter's minor version - the bytecode could potentially use unimplemented features.
* You may, at your own risk, attempt to run bytecode whose patch version is different.
* You may, at your own risk, attempt to run bytecode whose build version is different.

All interpreter implementations retain the right to reject any bytecode whose header data does not conform to the above specification.

The latest version information can be found in [toy_common.h](https://github.com/Ratstail91/Toy/blob/main/source/toy_common.h)
