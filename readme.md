# An exploration of C++11 for asynchronous programming

This set of experiments sets out to see how well we can handle asynchronous
(split-phase) style programming in C++.

There are a couple of goals/restrictions to make the game more fun:

### No dynamic memory allocation

The validity of this restriction is a separate debate, but for fairness with
what we have said Rust must do, I figure we need to have no dynamic memory.

### No explicit tagging of variables for closure.

This is a big one. If we *did* allow explicit tagging, then we could do all
this as a preprocessor step in C: simply move the "closure" to a global
function that takes an extra context struct. Copy all the tagged variables and
a pointer to the global func into the context struct and use it as the
"closure" that gets passed around.

Automatic closure seems like it is nice, but we could possibly go back on this
restriction.

### The ability to queue closures in a task queue

This is like posting a task in TinyOS. We need the ability to have a list of
closures to be executed by a scheduler later. Note that we preserve the
"only once" invariant of TinyOS, each closure has its own slot in the queue
with statically allocated memory for it. Multiple postings will either be
a noop or overwrite the previous slot.

### The ability to pass closures to functions with clean syntax

The biggest use of closures is as parameters to functions as an OnDone callback.
It would be a pity if this common case had terrible syntax or otherwise required
a lot of mental effort.

# Conclusion

There appear to be two approaches (or a hybrid as a third). One is using lambdas
directly. These cannot be passed as parameters to functions except as templated
arguments, and they cannot be stored in variables except as auto variables. They
cannot be returned from functions at all. This aside, they are fast and minimal.
The second option is to convert a lambda to a std::function and use that. That
allows the result to be returned, accepted as a parameter and stored in a variable.

Example 1 demonstrates a task queue application using pure lambdas. Example 2
does the same but using std::function. I suspect that the latter will be far
more usable.
