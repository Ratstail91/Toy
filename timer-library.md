# Timer Library

The timer library offers a series of utility functions for generating and manipulating timers, which can be used for recording or initiating timed events. Please be aware that "timers" are opaque objects, which must be destroyed when you're done with them, otherwise you'll have a memory leak in your program.

These functions are implemented by the host program, so they may not all be provided by default.

The timer library can usually be accessed with the `import` keyword:

```
import timer;
```

## fn startTimer(): opaque

This function returns an `opaque` timer object, representing the point in time when the function was called.

## fn _stopTimer(self: opaque): opaque

This function returns an `opaque` timer object, representing the duration between when the `self` argument was created, and when this function was called.

## fn createTimer(seconds: int, microseconds: int): opaque

This function returns an `opaque` timer object, with seconds and microseconds set to the given values. To give a negative value between 0 and -1, give a negative value for microseconds (negative 0 is not a thing in Toy).

Please note that `microseconds` has upper and lower bounds of -1,000,000 to 1,000,000. Also, if seconds has a non-zero value, then microseconds has a lower bounds of 0 instead.

## fn _getTimerSeconds(self: opaque): int

This function returns an integer value, representing the seconds value of the timer object.

## fn _getTimerMicroseconds(self: opaque): int

This function returns an integer value, representing the seconds value of the timer object.

## fn _compareTimer(self: opaque, other: opaque): opaque

This function returns an `opaque` timer object, representing the duration between the `self` and `other` arguments. The return value may be negative.

## fn _timerToString(self: opaque): string

This function returns a string representation of the `self` argument, which can be casted into other basic [types](types).

## fn _destroyTimer(self: opaque): null

This function cleans up the memory of the `self` timer object.

