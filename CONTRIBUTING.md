# Contributor Guidelines

## Advice for new contributors

Start small. The PRs most likely to be merged are the ones that make small,
easily reviewed changes with clear and specific intentions. See below for more
[guidelines on pull requests](#pull-requests).

It's a good idea to gauge interest in your intended work by finding the current issue
for it or creating a new one yourself. You can use also that issue as a place to signal
your intentions and get feedback from the users most likely to appreciate your changes.

You're most likely to have your pull request accepted easily if it addresses bugs already
in the [Next Steps project](https://github.com/despair86/loki-pager/projects/1),
especially if they are near the top of the Backlog column. Those are what we'll be looking
at next, so it would be great if you helped us out!

Once you've spent a little bit of time planning your solution, it's a good idea to go
back to the issue and talk about your approach. We'd be happy to provide feedback. [An
ounce of prevention, as they say!](https://www.goodreads.com/quotes/247269-an-ounce-of-prevention-is-worth-a-pound-of-cure)

## Developer Setup

First, you'll need a C89 compiler. App and library code is straight C89, while unit tests make extensive use of C99+.
We've tested the following C compilation systems:

- Microsoft C v15.00.30729+ **NOTE: A limited set of unit tests will be available when building with MSVC.**
- GNU CC v8.3+
- Apple LLVM C v8.0.1
- Oracle ProWorks (the x86 compiler) 12.1 or later
- Oracle SPARCWorks 12.1+
- IRIS Development Option v7.2 (or any version of Pro64 for any supported CPU and its descendants)

While these are the compilers currently tested, pretty much any conforming C89/99 implementation should work,
as we make no use of any vendor specific extensions, only platform-specific extensions when warranted.

Then you need `git`, if you don't have that yet: https://git-scm.com/

### macOS

1.  Install the [Xcode Command-Line Tools](http://osxdaily.com/2014/02/12/install-command-line-tools-mac-os-x/).

We can use the old NextSTEP implementation of BSD curses, as long as it has something resembling a standard-conforming API.

### Windows (using Microsoft C)

1.  **Windows 7 or earlier:**
    * Install Microsoft .NET Framework 3.5:
      (See Programs and Features/Optional Features)
    * Install Windows SDK version 7.0: https://developer.microsoft.com/en-us/windows/downloads/sdk-archive
2.  Install CMake from https://cmake.org/. 32-bit build is recommended even in 64-bit installs 
    if core memory is at a premium on your site.
3.  Install cURL from https://curl.haxx.se, we use it only to download the Netscape Navigator root 
    cert bundle, manually grab from: https://curl.haxx.se/ca/cacert.pem and save as `rootcerts.pem`
4.  Compile and load either of PDCurses or `ncurses`. You only need a separate curses library if you compile CDK yourself (see below).
    - For PDCurses, you may have to copy/symlink `pdcurses/win[con|gui]/pdcurses.lib` to 
      `lib/curses.lib`. Ncurses automatically installs its headers and libraries in the expected locations.
      - You may also need `<stdint.h>`, the header in-tree can be copied to the PDCurses root where it will be
        picked up by its own build system.
    - It is still possible to use the GNU Build System to configure CDK with Microsoft C,
      but you will need a POSIX-compatible `sh` and `make` at a minimum.
      - The canonical target name for this configuration is `i[3-7]86-pc-winnt3.51` or `x86_64-pc-winnt5.2` even for Windows 10.
      - You will also need an implementation of POSIX `opendir(3C)` and friends, one will be provided shortly.
      - In the meantime, a statically linked `libcdk.dll` and `cdk.lib` [will be provided](https://snowlight.net/files/cdk_win32.tar.xz)
        (requires nothing except `msvcrt.dll`, as `libgcc` and win32 native PDCurses are linked into the shared object.)
      - some environment variables:
        - `CC=cl`
        - `CXX=cl -TP`
        - `CPPFLAGS=-I[path to pdcurses root]`
        - `LIBS=-link -LIBPATH:[path to pdcurses/wincon or pdcurses/wingui] pdcurses.lib user32.lib gdi32.lib advapi32.lib kernel32.lib`
      - You may yet need GNU Binutils if for some reason, the build system cannot use 
        `LIB(1)` or `DUMPBIN(1)` (for `ar(1)` and `nm(1)`)

### Windows (using msys2 or mingw-w64)

1.  **Windows 7 or earlier:**
    * Install Microsoft .NET Framework 3.5:
      (See Programs and Features/Optional Features)
2.  Install CMake from https://cmake.org/. 32-bit build is recommended even in 64-bit installs 
    if core memory is at a premium on your site.
3.  Install cURL from https://curl.haxx.se, we use it only to download the Netscape Navigator root 
    cert bundle, manually grab from: https://curl.haxx.se/ca/cacert.pem and save as `rootcerts.pem`
4.  Compile and load either of PDCurses or `ncurses`. 
    - For PDCurses, you may have to copy/symlink `pdcurses/win[con|gui]/libpdcursesstatic.a` to 
      `lib/libcurses.a`. Ncurses automatically installs its headers and libraries in the expected locations.
    - If using the GNU build system, set the installation prefix to this folder, then rename the resulting library to `libcurses.a`.
5.  Compile and load the [Curses Development Kit (CDK)](https://github.com/despair86/cdk-win32). set the prefix to this folder.
    - This installs the library and headers to ./lib and ./include respectively.
    - It is technically possible to use the pre-built `libcdk.dll` above directly, if desired.
6.  Alternatively, you can use MSYS2 `pacman` to install these from the MSYS2 AUR
    (along with the base `mingw-w64-$ARCH-[gcc|clang]` package)

If you are developing on an older version of Windows, you should probably get the last version
of Microsoft C available for your system, plus the Windows SDK for Windows Server 2003 R2.

### Linux

1.  Pick your favorite package manager.
1.  Install `gcc`
1.  Install `g++`
1.  Install `make`
1.  Install `cmake`
1.  Install `curl`

`ncurses` is required on Linux.

### UCB UNIX (OpenBSD, NetBSD, FreeBSD, etc)

The C compilation system should already be available (try executing `cc`), optionally 
install Ninja, and make sure CMake and cURL are available.

We can use ancient BSD curses if that's all you have, so long as it implements something resembling X/Open curses.

### Solaris 2.x

Install Oracle Workshop from https://www.oracle.com/technetwork/server-storage/developerstudio/overview/index.html, and CMake, or
install GCC or Clang from IPS. For SVR4 packages, the [UNC SunSITE](http://ibiblio.org) may have 
GCC or Clang packages for Solaris 2.5-2.10 available.

A separate X/Open Group curses implementation is not required, we can use the regular System 5 Release 4 `libcurses`.

**NOTE: the system libcurses may print inconsistent colours if the terminal background *isn't* black.** This limitation stems from
[the assumption that all terminals have a black background](https://docs.oracle.com/cd/E88353_01/html/E37849/init-pair-3curses.html#scrolltoc).

## Development host setup (All platforms)

### Build dependencies

* [CMake](https://cmake.org/) 2.8.4 or higher
* [LCOV *2](http://ltp.sourceforge.net/coverage/lcov.php)

Most of these dependencies are required just for the unit test suite and
development of the library itself. When integrating into actual applications,
you should not need anything beyond CMake. Alternatively, you may integrate
the code using a build system of your choice.
Items marked with *1 are required for tests, with *2 are additionally required for code coverage.

```
git clone https://github.com/despair86/loki-pager.git
cd loki-pager
```

### Setting up a fresh source tree

    $ mkdir build
    $ cd build
    $ cmake -DCMAKE_BUILD_TYPE=Debug ..
    $ make

### Running the unit tests

    $ cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=1 ..
    $ cd tests
    $ make
    $ cd ..
    $ ctest

### Creating the code coverage report

    $ cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=1 -DCOVERAGE=1 ..
    $ make coverage

The generated code coverage report can be found in:
`/path/to/libsignal-protocol-c/build/coverage`

### Eclipse project setup

CMake provides a tutorial on Eclipse project setup here:
https://cmake.org/Wiki/CMake:Eclipse_UNIX_Tutorial

It is recommended to follow the more manual "Option 2," since the Eclipse
project generator built into CMake tends to be outdated and leads you toward
a very awkward and occasionally broken project configuration.

### Protocol Buffers compiler

This project uses serialization code based on [Protocol Buffers](https://github.com/google/protobuf).
Since the official library does not support C, the [protobuf-c](https://github.com/protobuf-c/protobuf-c)
generator is used instead. For the sake of convenience, the generated code and its dependencies are
included in the source tree. The generated code can be regenerated at any time by installing the two
mentioned packages and running "make" in the "protobuf/" subdirectory.

## Target platforms

CMake toolchain files have been included from the following sources:

* [iOS](https://code.google.com/archive/p/ios-cmake)
* [BlackBerry 10](https://github.com/blackberry/OGRE/blob/master/src/CMake/toolchain/blackberry.toolchain.cmake)

## Additional storage profiles

Since there is no registration for Loki Messenger, you can create as many accounts as you
can public keys. To test the P2P functionality on the same machine, however, requries
that each client binds their message server to a different port.

You can use the following command to start a client bound to a different port.

```
pager --bind-port=[port]
```
# Making changes

So you're in the process of preparing that pull request. Here's how to make that go
smoothly.

## Tests

Please write tests! Our testing framework is
[Check](https://libcheck.github.io/check).

A C99 compilation system is required to assemble unit tests.

The easiest way to run all tests at once is `make test`. Additionally, you can handwrite unit tests, by
making them return 0 for success, and anything else if it failed.

## Pull requests

So you wanna make a pull request? Please observe the following guidelines.

<!-- TODO:
* Please do not submit pull requests for translation fixes. Anyone can update
  the translations in
  [Transifex](https://www.transifex.com/projects/p/signal-desktop).
-->

* Never use plain strings right in the source code - pull them from `messages.json`!
  You **only** need to modify the default locale
  [`_locales/en/messages.json`](_locales/en/messages.json).
  * We should probably use `catgets` or `gettext`, since we're not shackled by the limitations of
    a JS interpreter environment.
  * On Windows NT, we can also write translation strings into the resource section. 
  * An internal API will be provided for platform-independent localisation, including a fallback to the JSON
    string tables on platforms where no other localisation facility exists.
  <!-- TODO:
    Other locales are generated automatically based on that file and then periodically
    uploaded to Transifex for translation. -->
* [Rebase](https://nathanleclaire.com/blog/2014/09/14/dont-be-scared-of-git-rebase/) your
  changes on the latest `development` branch, resolving any conflicts.
  This ensures that your changes will merge cleanly when you open your PR.
* Be sure to add and run tests!
* Make sure the diff between our master and your branch contains only the
  minimal set of changes needed to implement your feature or bugfix. This will
  make it easier for the person reviewing your code to approve the changes.
  Please do not submit a PR with commented out code or unfinished features.
* Avoid meaningless or too-granular commits. If your branch contains commits like
  the lines of "Oops, reverted this change" or "Just experimenting, will
  delete this later", please 
  [squash or rebase those changes away](https://robots.thoughtbot.com/git-interactive-rebase-squash-amend-rewriting-history).
* Don't have too few commits. If you have a complicated or long lived feature
  branch, it may make sense to break the changes up into logical atomic chunks
  to aid in the review process.
* Provide a well written and nicely formatted commit message. See [this
  link](http://chris.beams.io/posts/git-commit/)
  for some tips on formatting. As far as content, try to include in your
  summary
  1.  What you changed
  2.  Why this change was made (including git issue # if appropriate)
  3.  Any relevant technical details or motivations for your implementation
      choices that may be helpful to someone reviewing or auditing the commit
      history in the future. When in doubt, err on the side of a longer
      commit message.

Above all, spend some time with the repository. Follow the pull request template added to
your pull request description automatically. Take a look at recent approved pull requests,
see how they did things.

## Testing Production Builds

To test changes to the build system, build a release using `make` or `ninja`,
 which should kick off CMake if the build system changed.

Then, run the tests using the `test` target depending on your make app (`[make|ninja] test`)

<!-- TODO:
## Translations

To pull the latest translations, follow these steps:

1.  Download Transifex client:
    https://docs.transifex.com/client/installing-the-client
2.  Create Transifex account: https://transifex.com
3.  Generate API token: https://www.transifex.com/user/settings/api/
4.  Create `~/.transifexrc` configuration:
    https://docs.transifex.com/client/client-configuration#-transifexrc
5.  Run `yarn grunt tx`. -->

