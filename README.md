# deldir
## Recursively delete a directory (supports huge numbers of inodes per directory)

### Introduction

`deldir` is a lightweight C program to recurisvely delete a directory. It's not
terribly smart, as it deletes all files and folders in the given directory (it
has no support for filtering of any kind including globbing), but it has one
advantage over `rm`: it works with a theoretically unlimited number of inodes.
For what that means, see the Advantages section. If you have a folder with so
many files and folders that `rm` or other programs refuse to delete, `deldir` is
the answer.

### License

`deldir` is licensed under the [GNU GPLv3](https://www.gnu.org/licenses/gpl.html)
**only** (**not** any later version). A copy of the GPLv3 is available as
`COPYING` in both the source and binary distributions.

### Requirements

To build `deldir` you need a C compiler that supports at least C90. C99 and C11
are also fully supported. At least C99-compliant compiler is strongly suggested
as additional optimizations can be used with C99. `deldir` can also be compiled
in C11 mode. `deldir` has been compiled with [Clang](http://clang.llvm.org/),
[GCC](http://gcc.gnu.org/), the
[Intel C++ Compiler](http://software.intel.com/en-us/c-compilers/), and
[TCC](http://bellard.org/tcc/) successfully.

A Bourne-compatible shell is also needed to run the optional `build.sh` script
(`bash` works fine as does `dash`). If you don't have `sh` for some reason, you
can manually compile `deldir` fairly easily as it is only one C file.

To run `deldir` you need a UNIX-ish system. `deldir` is only officially tested
on Linux and FreeBSD, but I'd love to hear if it works elsewhere! It (should)
only use standard ISO C and POSIX functions. `deldir` uses `lstat` to safely
handle symbolic links (symlinks) which was added to POSIX in POSIX.1-2001.

Windows is not supported at all in the absence of a POSIX emulation environment
(Cygwin should work, MinGW almost certainly won't due to lack of `lstat`). It
should be possible in the future to fall back to `stat` on Windows as Windows
does not support symbolic links. If you want `deldir` to support Windows please
contact me. If there is demand for it, I will investigate porting it with MinGW.

### Building

Running `make` from the `deldir` source folder will build `deldir` for you. If
you cannot run `make` for whatever reason, you should be able to manually
compile `deldir` with something similar to:

`cc -o deldir deldir.c`

Replace `cc` with your compiler's name if it's not usable as `cc`.

Note that `make` will run `strip` on the compiled executable by default. If you
don't want `strip` to be run (perhaps because `strip` removes debug symbols),
just run `make deldir` instead.

### Running

**WARNING**: In case you missed it, `deldir` deletes *ALL* of the files and
folders in the target directory. If a folder contains *ANY* files you want to
keep **DO NOT** run `deldir` on it! Also, `deldir` has no warranty whatsoever,
so don't yell at me if Something Bad(tm) happens! Keep your data backed up
(which you should always do in general anyway).

Before `deldir` gets to work deleting everything in the folder, it will first
confirm it has the right path. You'll see a prompt like this:

    WARNING: *ALL* files in `example' will be deleted! Continue?
    Type "yes" and press ENTER to confirm, or anything else to cancel.
    > 

Make sure that the path on the first line is correct, and *please* make sure
that the target folder doesn't contain anything you want to keep. If you are
sure you want to run `deldir`, type `yes` and press enter.

You should run the self-test that is included with the source and binary
distributions before using it, especially if you compiled `deldir` yourself! To
run the self-test, make sure `test.sh` and `deldir` are in the same folder, then
run `test.sh`. Make sure the target is in `/tmp` (it should be
`/tmp/deldir-test.XXXXXXXXXX`, replacing `X` with random characters). At the end
of the self-test you should see the line `Self-test PASSED`. If you instead see
`Self-test FAILED`, or *anything else* at the end of the output, **DO NOT** use
that copy of `deldir`! Please contact me as soon as possible and send me a copy
of the self-test output. It's possible that `deldir` is working but something in
your environment caused the failure, but with a program for mass deletion it's
best to err on the side of caution.

`deldir` requires only one argument on the command line: the path to the folder
to delete. The path may be absolute or relative; `deldir` will show the full
canonical path in the warning message in either case. `deldir` also has several
switches that change how `deldir` works:

* `-h`: Prints command-line usage help and exits (no path required)
* `-q`: Quiet mode; do not print files and folders as they are deleted
* `-s`: Simulate mode; do not actually delete files and folders, only print what
  would be deleted
* `-v`: Prints the version of `deldir` and exits (no path required)
* `-y`: Skip the confirmation to delete; assume it is answered with `yes`
    * `deldir` will instead print the folder that will be deleted then pause for
      five seconds to give you time to cancel before it continues

### Advantages

`deldir` has one big advantage (which is the entire reason it exists): it works
with a theoretically infinite number of inodes (at least as many as the kernel
supports, which there should be no way to exceed in the first place). `deldir`
was created when a friend of mine had a folder with an extremely large number of
inodes, and found that `rm` was unable to delete it, even without globbing!
Instead of digging through the [coreutils](https://www.gnu.org/software/coreutils/)
source code to try to figure out why `rm` failed, I wrote a much simpler program
to just wipe out the folder.

### Disadvantages

Because `deldir` uses recursion to handle nested folders it does not work with
very deeply nested folders. If you have a folder with thousands of folders in
one folder `deldir` will work fine. The problematic case is when one folder
contains another folder that contains yet another folder, and so on for an
extremely large number of levels. `deldir` recursively calls a function for each
directory, so a large amount of nesting causes `deldir` to overflow the stack.
You may be able to work around this by increasing the size of the stack. With
`bash`, for example, you can use `ulimit -s` to get or set the stack size.
Search `bash`'s manpage for `ulimit` to get more details.

However, you shouldn't run into the limit in practice. With stack size limit of
8192 on an x86_64 machine, `deldir` can recurse 174470 times.

