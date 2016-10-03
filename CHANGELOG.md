## 2.4.0 (September 7,2016)

**BUG FIXES & IMPROVEMENTS:**
- Many memory leaks fixed (char * replaced with std::string) also memory was not freed
- Added nfs3fhsize32 option. Some NFS clients do not like 64 bit handles
- When path is received it can be in form <path>/../<folder>. This introduced some problems. Path is now converted to canonical path,
- Not every reparse point is considered a link. Reparse point which mounts another volume is not a link,

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
