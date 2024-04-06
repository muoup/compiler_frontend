# Compiler Optimization Concepts

## Tail Calls

Tails calls allow for the assembly to not generate unnecessary stack frames for 
recursive functions, replacing a call with a jump under the circumstance that
the function returns either after the call or the call is the value which is
returned. It is not necessarily that the call must be at the end of the function
as tail call optimizations can exist in conditional blocks as well, among other
similar circumstances.

## Loop Unrolling

For loops under a certain size, it is better to unroll the loop into individual
repeating blocks of code. This increases the binary size, but avoids the inefficiency
of the conditional jumps (i.e. branch misses) and the overhead of the loop counter.

## Dead Code Elimination

When code is guaranteed to never run, it is better to remove it from the binary
to reduce the size of the binary. While this may seem silly, it is often the case
that during debugging, code is added to the binary that is never run in the final
product and is stuck inside dead code which is activated only during debugging.

## Variable Inlining

If a variable is trivially determinable, or is a method call which is only referenced
once, it is better to inline the variable or method call to avoid the additional overhead
of the store and load operations. This is especially true for trivial constant variables,
which are often used to avoid magical numbers in code.

## Function Inlining

Adequately sized functions or ones which are only called once or twice should be inlined
into the calling function to avoid the stack frame overhead of the method call.