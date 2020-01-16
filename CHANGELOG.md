## 2.4.0 (November 13, 2017)

**BUG FIXES & IMPROVEMENTS:**

- Fixed memory leads (many thanks to Toilal!)
- Fix file writing issue when first write cause access denied error (thanks to Toilal)
- Minor stuff

## 2.3.1 (October 13, 2016)

**BUG FIXES & IMPROVEMENTS:**

- Fix correct error code (NFS3ERR_NOTEMPTY) when deleting a directory which is not empty

## 2.3.0 (September 28, 2016)

**BUG FIXES & IMPROVEMENTS:**

- Implemented EXPORT to PROG_MOUNT, so showmount -e ip now works ([Kodi](https://kodi.tv) now works fine)
- Fixed some bugs with path aliases (see #30)
- Added assembly infos (see #31)
- Whole drives can now be shared

## 2.2.0 (August 6, 2016)

**BUG FIXES & IMPROVEMENTS:**

- Faster write speed (between 10 to 100 times faster) due to increased buffer size
- Added async write procedure for even faster writes for big files
- Fixed TCP implementation
- Added DUMP to PROG_MOUNT command for boot2docker and osx compatibility
- Added an alias option to the pathfile format (thanks to eidng8)
- Added an command line option to only listen on one interface (thanks to eidng8)

## 2.1.1 (July 10, 2016)

**BUG FIXES & IMPROVEMENTS:**

- Fix for slow READDIR
- Fixed odd log timestamp formatting

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
