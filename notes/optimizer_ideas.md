# Notes

## Not All Optimizations Are Necessary in the Frontend

It is important to recognize that this piece of software is merely a compiler frontend,
and as such a lot of specific forms of optimization are not necessary to be implemented,
however the general concepts of the language will allow for optimizations that the LLVM
IR compiler may not be able to perform.

## Optimization Can Still be Useful Even If Implemented in the Backend

While it is true that the LLVM IR compiler will perform many of the optimizations that
are necessary for a compiler, it is still useful to implement some optimizations in the
frontend to enable previously mentioned language specific optimizations.

# Compiler Optimization Concepts

## Tail Calls

Tails calls allow for the assembly to not generate unnecessary stack frames for 
recursive functions, replacing a call with a jump under the circumstance that
the function returns either after the call or the call is the value which is
returned. It is not necessarily that the call must be at the end of the function
as tail call optimizations can exist in loop blocks as well, among other
similar circumstances.

## ~~Loop Unrolling~~

~~For loops under a certain size, it is better to unroll the loop into individual
repeating blocks of code. This increases the binary size, but avoids the inefficiency
of the loop jumps (i.e. branch misses) and the overhead of the loop counter.~~

Loop unrolling realistically would not change much in the scope of what optimization
can be done in the frontend, so seeing as LLVM already performs this optimization in
the current build, this will not be a priority.

## Dead Code Elimination

When code is guaranteed to never run, it is better to remove it from the binary
to reduce the size of the binary. While this may seem silly, it is often the case
that during debugging, code is added to the binary that is never run in the final
product and is stuck inside dead code which is activated only during debugging.

## Variable Inlining

If a instance is trivially determinable, or is a method call which is only referenced
once, it is better to inline the instance or method call to avoid the additional overhead
of the store and load operations. This is especially true for trivial constant variables,
which are often used to avoid magical numbers in code.

## Function Inlining

Adequately sized functions or ones which are only called once or twice should be inlined
into the calling function to avoid the stack frame overhead of the method call.

## Domain Analysis

I'm not quite sure what the technical term for this is, but for instance I know the Rust
compiler will recognize the domain of a modulus operation and evaluate that certain match
cases are impossible. There are certainly other applications of this, but I will need to do
more research to understand the full extent of this optimization.