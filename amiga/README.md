# mg-amiga

mg (Micro Emacs, [troglobit/mg](https://github.com/troglobit/mg)) for
AmigaOS 3.x on any 68000 or better. Runs in a Shell window; tested on
AmigaOS 3.2 on an A500 (68000) and an A1200. Users: see
[README.txt.in](README.txt.in), which goes into the release archive.

Upstream is UNIX-only by choice
([troglobit/mg#45](https://github.com/troglobit/mg/issues/45)), so the
port lives in this fork and follows upstream releases.

## Layout

The Amiga code is in its own files, so that merging upstream rarely
conflicts:

| Where | What |
|---|---|
| `src/amiga/amiga.h` | included first in every file of the Amiga build (`-include`): POSIX names clib2 lacks, a `stat()` with POSIX semantics, `ioctl(TIOCGWINSZ)` from the console, stubs for `fork`/`pipe`/`exec`/ptys that fail with ENOSYS, ISO-8859-1 `isprint` |
| `src/amiga/include/` | headers clib2 has not (`err.h`, `poll.h`, `langinfo.h`) |
| `src/amiga/ttyio.c` | the console as mg's terminal, in place of `src/ttyio.c`: raw mode, CSI keys, wrap/scroll, stack size, ^C |
| `src/amiga/console.c` | console control sequences and keys, window size, Latin-1 word characters |
| `src/amiga/path.c` | Amiga path names (`Volume:dir/file`), `.` and `..` |
| `src/amiga/dired.c` | dired's directory listing, in `ls -al`'s layout, from `Examine()`/`ExNext()` instead of running `ls` |
| `src/amiga/compat.c` | the stubs and wrappers declared in `amiga.h`; `open()`, `fchmod()` and `rename()` that keep protection bits and comments and replace an existing file as POSIX does |
| `src/amiga/version.c` | the `$VER:` string |
| `amiga/` | build, packaging, docs, Aminet readme |

Changes in upstream's files are insertions marked `__amigaos__` (or
`mg-amiga` in the build files), plus one line in `src/Makefile.am`:
`configure.ac` (`AMIGA` conditional, libm), `src/Makefile.am` (sources and
flags), `src/ansi.c` and `src/ansi.h` (console setup, no alternate screen
or colours), `src/file.c`, `src/fileio.c`, `src/dir.c`, `src/echo.c`
(path rules), `src/dired.c` (the listing; no `/` after `Work:`, also
when copy and rename put a name after a directory). `git diff master -- src configure.ac` shows them all.

## Building

With the AmigaPorts toolchain (`m68k-amigaos-gcc` with clib2) on the PATH,
for instance the prebuilt Linux release the CI uses (see
`.github/workflows/amiga.yml`, unpacked anywhere):

    amiga/build.sh                          # amiga/out/mg
    MG_AMIGA_VERSION=1.0.0 amiga/package.sh # amiga/out/mg-amiga-1.0.0-mg4.2.lha + .readme

`build.sh` runs `autogen.sh` when there is no `configure`. Configure
options and the reasons for what is left out are in `build.sh`.

## Releases and versions

The version is our own [SemVer](https://semver.org), released by
[semantic-release](https://semantic-release.gitbook.io) from
[conventional commits](https://www.conventionalcommits.org) on the `amiga`
branch: `fix:` gives a patch release, `feat:` a minor one, `!` a major one.
Tags are `amiga-vX.Y.Z` (upstream's own `vX.Y` tags are in the fork too).
The upstream release a version is built on shows in the release title
("mg-amiga 1.2.0 (mg 4.3)"), the archive name
(`mg-amiga-1.2.0-mg4.3.lha`), the `$VER:` string (`Version mg`) and the
Aminet readme. Configuration: `.releaserc.json`.

Aminet: the workflow "Aminet upload" (Actions, run by hand) takes a
release, renames its files to `mg-amiga.lha` and `mg-amiga.readme` (an
upload with the same name replaces the old one), checks them with
`amiga/aminet-check.sh` against Aminet's rules and, with "upload" ticked,
sends them to `ftp://main.aminet.net/new`. Without the tick it only checks
and lists the FTP directory. Aminet wants an e-mail address in the
readme's `Uploader:` field; it stays out of the repository and the GitHub
release: the workflow adds the secret `AMINET_EMAIL` to that line just
before the upload (and uses it as the FTP password). Aminet asks for at
most one update a week.

## Following upstream

Branches: `master` mirrors troglobit/mg and is never changed here;
`amiga` (the default branch) is upstream plus the port. For a new upstream
release:

    git fetch upstream --tags
    git checkout amiga
    git merge v4.3                  # resolve; the hooks are small
    amiga/build.sh                  # fix what the new code needs from amiga.h
    # test in FS-UAE (below), then:
    git commit --amend -m "feat: update to upstream mg 4.3"

Merge, never rebase: the release tags must stay on the branch.

## Tests

The binary is tested in FS-UAE with AmigaOS 3.2 by a harness that types
keys and checks the files mg saves (scratch buffer, new files on a volume
root, Amiga keys, ^C, Latin-1, restarts, the Shell after quitting). It
needs a Kickstart ROM and AmigaOS, which cannot be in a public CI, so it
runs locally before releases.

## Licences

mg is public domain, and so is the port (`UNLICENSE`). The binary also
contains a few ISC-licensed files from `lib/` and clib2 (BSD 3-clause);
`amiga/package.sh` collects their notices into `LICENSES.txt`.
