# Testing Toy

Toy uses GitHub actions for automated testing - all of the tests are under `test/`, and can be executed by running `make test`.

The tests consist of a number of different situations and edge cases which have been discovered, and should probably be thoroughly tested one way or another. There are also several "-bugfix.toy" scripts which explicitly test a bug that has been encountered in one way or another.

Finally, the libs that are stored in `repl/` are also tested - their tests are under `/tests/scripts/lib`.

