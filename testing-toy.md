# Testing Toy

Toy uses GitHub CI for comprehensive automated testing - however, all of the tests are under `test/`, and can be executed by running `make test`. Doing so on linux will attempt to use valgrind; to disable using valgrind, pass in `DISABLE_VALGRIND=true` as an environment variable.

The tests consist of a number of different situations and edge cases which have been discovered, and should probably be thoroughly tested one way or another. There are also several "-bugfix.toy" scripts which explicitly test a bug that has been encountered in one way or another. The libs that are stored in `repl/` are also tested - their tests are under `/tests/scripts/lib`; some error cases are also checked by the mustfail tests in `/test/scripts/mustfail`.

Finally, GitHub CI has access to the option `make test-sanitized` which attempts to use memory sanitation. I don't know enough about this to offer much comentary, only that several invisible issues are monitored this way.

