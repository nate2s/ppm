Taffy is an object-oriented programming language that supports symbolic computation, matrices, calculus, complex numbers, and much more.

## Examples ##

Print the first 21 Fibonacci numbers:

```
f(n) = f(n - 1) + f(n - 2)
f(0) = 0
f(1) = 1

f[0, 20]
==> [0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181, 6765]
```

Simplify the function **(x^2 + 2x + 1) / (x + 1)**:

```
(x^2 + 2x + 1) / (x + 1) simplify
==> F(x) = x + 1
```

Integrate the function **sin(x)**:

```
integrate sin(x)
==> F(x) = -cos(x)
```

Derive the function **cos(x^2) * x**:

```
derive cos(x^2) * x
==> F(x) = cos(x^2) - 2 * x^2 * sin(x^2)
```

[More Examples](http://www.arithmagic.com/taffy/Examples.html)

[API](http://www.arithmagic.com/taffy/Object.html)

Taffy is covered by the [GNU Lesser General Public License, version 3](https://www.gnu.org/licenses/lgpl-3.0.txt)

## Running Taffy and Pie ##

Taffy comes with two executables: **taffy** and **pie**. **taffy** executes an input file(s) and then exits, and **pie** executues
intructions interactively via the console (think Ruby's **irb**).

The **taffy** process requires at least one file as input:

```
taffy inputFile.ty
```

**pie** is executed with no arguments:

```
pie
```

and gives the user a prompt. The user then enters taffy code:

```
pie.1> putLine("Hello, Taffy")
Hello, Taffy
==> #IO
```

the **==>** symbol indicates the return value from the previous process. In this case, the **put()** method returned the **io** object,
which has type **IO**.

## Building Taffy on Linux, Mac and Cygwin

To build Taffy and Pie (the interactive command-line interpreter), use the **configure** script:

```
./configure
```

then build:

```
make
```

Then (optionally) install:

```
(sudo) make install
```

Taffy has no external library dependencies, besides [decNumber](http://download.icu-project.org/files/decNumber/) (a modified version is included in this distribution). Pie requires either [readline](http://tiswww.case.edu/php/chet/readline/rltop.html) or [editline](http://thrysoee.dk/editline/).
