# Unit-aware values and `settle`

## Unit-aware values

Supported suffixes in this first implementation are `m`, `km`, `s`, `min`, and `h`.
Values are converted to base units at compile time/code generation: metres and seconds.
Thus `500.m + 2.km` evaluates to `2500`, and `30.s + 2.min` evaluates to `150`.

Addition/subtraction and comparisons require matching dimensions. Multiplication and
integer division combine dimensions; the runtime representation is an integer scalar,
so division truncates as in the existing integer arithmetic. Unit dimensions are checked
at compile time. Unit-aware floating-point arithmetic, compound unit suffixes such as
`cm`, and display formatting with a unit label are not part of this initial version.

## `settle`

```mead
settle max 100 {
    if (dist[a] + cost < dist[b]) {
        dist[b] = dist[a] + cost;
    }
} else {
    panic("Did not settle");
}
```

The initial implementation supports ordinary variable assignments and control flow in
the block. Each assignment compares the old and new scalar value; assigning the same
value does not mark the pass as changed. The block repeats until a full pass makes no
changes. `max` limits passes; if the limit is exhausted, the `else` block runs. `panic`
currently accepts one string literal, prints it, and exits with status 1. Array indexing
is not implemented by this compiler snapshot, so the example above illustrates intended
syntax rather than a currently compilable array program.
