## 2.1.0 (June 23, 2016)

Many things have been fixed and WinNFSd works much better in everyday situations now.
This update is highly recommended for every user.

**BUG FIXES & IMPROVEMENTS:**

- Multiple, concurrent mounts (thanks to das-peter and Yasushi)
- SETATTR set size implemented. This fixes many, if not all problems with with git, node etc.
- SETATTR set mtime/atime implemented. This fixes touch calls [https://github.com/winnfsd/vagrant-winnfsd/issues/39]
- Symlinks now work across os [https://github.com/winnfsd/winnfsd/issues/10]
- Junctions and absolute symlinks are also supported (if their target resides inside the mount dir)
- Fix READDIR implementation [https://github.com/winnfsd/vagrant-winnfsd/issues/78]
- Files larger than 2GB are now supported (thanks to lepsiloon)
- miscellaneous fixes and memory leak fixes (thanks to Yasushi and philr)
- added a basic tests based on the Connectathon test suite

## Previous

The changelog starts with version 2.1.0. Any prior changes can be
seen by checking out the tagged releases and git commit messages.
