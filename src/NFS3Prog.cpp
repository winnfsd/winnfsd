#pragma comment(lib, "Shlwapi.lib")
#include "NFSProg.h"
#include "FileTable.h"
#include <string.h>
#include <io.h>
#include <direct.h>
#include <sys/stat.h>
#include <assert.h>
#include <string>
#include <windows.h>
#include <time.h>
#include <share.h>
#include "common.hh"
#include "shlwapi.h"
#define BUFFER_SIZE 1000

enum
{
    NFSPROC3_NULL = 0,
    NFSPROC3_GETATTR = 1,
    NFSPROC3_SETATTR = 2,
    NFSPROC3_LOOKUP = 3,
    NFSPROC3_ACCESS = 4,
    NFSPROC3_READLINK = 5,
    NFSPROC3_READ = 6,
    NFSPROC3_WRITE = 7,
    NFSPROC3_CREATE = 8,
    NFSPROC3_MKDIR = 9,
    NFSPROC3_SYMLINK = 10,
    NFSPROC3_MKNOD = 11,
    NFSPROC3_REMOVE = 12,
    NFSPROC3_RMDIR = 13,
    NFSPROC3_RENAME = 14,
    NFSPROC3_LINK = 15,
    NFSPROC3_READDIR = 16,
    NFSPROC3_READDIRPLUS = 17,
    NFSPROC3_FSSTAT = 18,
    NFSPROC3_FSINFO = 19,
    NFSPROC3_PATHCONF = 20,
    NFSPROC3_COMMIT = 21
};

enum
{
    NFS3_OK = 0,
    NFS3ERR_PERM = 1,
    NFS3ERR_NOENT = 2,
    NFS3ERR_IO = 5,
    NFS3ERR_NXIO = 6,
    NFS3ERR_ACCES = 13,
    NFS3ERR_EXIST = 17,
    NFS3ERR_XDEV = 18,
    NFS3ERR_NODEV = 19,
    NFS3ERR_NOTDIR = 20,
    NFS3ERR_ISDIR = 21,
    NFS3ERR_INVAL = 22,
    NFS3ERR_FBIG = 27,
    NFS3ERR_NOSPC = 28,
    NFS3ERR_ROFS = 30,
    NFS3ERR_MLINK = 31,
    NFS3ERR_NAMETOOLONG = 63,
    NFS3ERR_NOTEMPTY = 66,
    NFS3ERR_DQUOT = 69,
    NFS3ERR_STALE = 70,
    NFS3ERR_REMOTE = 71,
    NFS3ERR_BADHANDLE = 10001,
    NFS3ERR_NOT_SYNC = 10002,
    NFS3ERR_BAD_COOKIE = 10003,
    NFS3ERR_NOTSUPP = 10004,
    NFS3ERR_TOOSMALL = 10005,
    NFS3ERR_SERVERFAULT = 10006,
    NFS3ERR_BADTYPE = 10007,
    NFS3ERR_JUKEBOX = 10008
};

enum
{
    NF3REG = 1,
    NF3DIR = 2,
    NF3BLK = 3,
    NF3CHR = 4,
    NF3LNK = 5,
    NF3SOCK = 6,
    NF3FIFO = 7
};

enum
{
    ACCESS3_READ = 0x0001,
    ACCESS3_LOOKUP = 0x0002,
    ACCESS3_MODIFY = 0x0004,
    ACCESS3_EXTEND = 0x0008,
    ACCESS3_DELETE = 0x0010,
    ACCESS3_EXECUTE = 0x0020
};

enum
{
    FSF3_LINK = 0x0001,
    FSF3_SYMLINK = 0x0002,
    FSF3_HOMOGENEOUS = 0x0008,
    FSF3_CANSETTIME = 0x0010
};

enum
{
    UNSTABLE = 0,
    DATA_SYNC = 1,
    FILE_SYNC = 2
};

enum
{
    DONT_CHANGE = 0,
    SET_TO_SERVER_TIME = 1,
    SET_TO_CLIENT_TIME = 2
};

enum
{
    UNCHECKED = 0,
    GUARDED = 1,
    EXCLUSIVE = 2
};

opaque::opaque()
{
    length = 0;
    contents = NULL;
}

opaque::opaque(uint32 len)
{
    contents = NULL;
    SetSize(len);
}

opaque::~opaque()
{
    delete[] contents;
}

void opaque::SetSize(uint32 len)
{
    delete[] contents;
    length = len;
    contents = new unsigned char[length];
    memset(contents, 0, length);
}

nfs_fh3::nfs_fh3() : opaque(g_NFS3_FHSIZE)
{
}

nfs_fh3::~nfs_fh3()
{
}

filename3::filename3() : opaque()
{
    name = NULL;
}

filename3::~filename3()
{
}

void filename3::SetSize(uint32 len)
{
    opaque::SetSize(len + 1);
    length = len;
    name = (char *)contents;
}

void filename3::Set(char *str)
{
    SetSize(strlen(str));
    strcpy_s(name, (strlen(str) + 1), str);
}

nfspath3::nfspath3() : opaque()
{
    path = NULL;
}

nfspath3::~nfspath3()
{
}

void nfspath3::SetSize(uint32 len)
{
    opaque::SetSize(len + 1);
    length = len;
    path = (char *)contents;
}

void nfspath3::Set(char *str)
{
    SetSize(strlen(str));
    strcpy_s(path, (strlen(str) + 1), str);
}

typedef nfsstat3(CNFS3Prog::*PPROC)(void);

CNFS3Prog::CNFS3Prog() : CRPCProg()
{
    m_nUID = m_nGID = 0;
}

CNFS3Prog::~CNFS3Prog()
{
}

void CNFS3Prog::SetUserID(unsigned int nUID, unsigned int nGID)
{
    m_nUID = nUID;
    m_nGID = nGID;
}

int CNFS3Prog::Process(IInputStream *pInStream, IOutputStream *pOutStream, ProcessParam *pParam)
{
    static PPROC pf[] = { 
        &CNFS3Prog::ProcedureNULL, &CNFS3Prog::ProcedureGETATTR, &CNFS3Prog::ProcedureSETATTR, 
        &CNFS3Prog::ProcedureLOOKUP, &CNFS3Prog::ProcedureACCESS, &CNFS3Prog::ProcedureREADLINK, 
        &CNFS3Prog::ProcedureREAD, &CNFS3Prog::ProcedureWRITE, &CNFS3Prog::ProcedureCREATE, 
        &CNFS3Prog::ProcedureMKDIR, &CNFS3Prog::ProcedureSYMLINK, &CNFS3Prog::ProcedureMKNOD, 
        &CNFS3Prog::ProcedureREMOVE, &CNFS3Prog::ProcedureRMDIR, &CNFS3Prog::ProcedureRENAME, 
        &CNFS3Prog::ProcedureLINK, &CNFS3Prog::ProcedureREADDIR, &CNFS3Prog::ProcedureREADDIRPLUS,
        &CNFS3Prog::ProcedureFSSTAT, &CNFS3Prog::ProcedureFSINFO, &CNFS3Prog::ProcedurePATHCONF,
        &CNFS3Prog::ProcedureCOMMIT
    };

    nfsstat3 stat;

	struct tm current;
	time_t now;

	time(&now);
	localtime_s(&current, &now);

	PrintLog("[%02d:%02d:%02d] NFS ", current.tm_hour, current.tm_min, current.tm_sec);

    if (pParam->nProc >= sizeof(pf) / sizeof(PPROC)) {
        ProcedureNOIMP();
        PrintLog("\n");

        return PRC_NOTIMP;
    }

    m_pInStream = pInStream;
    m_pOutStream = pOutStream;
    m_pParam = pParam;
    m_nResult = PRC_OK;

    try {
        stat = (this->*pf[pParam->nProc])();
    } catch (...) {
        m_nResult = PRC_FAIL;
    }

    PrintLog(" ");

    if (m_nResult == PRC_FAIL) { //input data is truncated
        PrintLog("fail");
    } else {
        switch (stat) {
            case NFS3_OK:
                PrintLog("OK");
                break;
            case NFS3ERR_PERM:
                PrintLog("PERM");
                break;
            case NFS3ERR_NOENT:
                PrintLog("NOENT");
                break;
            case NFS3ERR_IO:
                PrintLog("IO");
                break;
            case NFS3ERR_NXIO:
                PrintLog("NXIO");
                break;
            case NFS3ERR_ACCES:
                PrintLog("ACCESS");
                break;
            case NFS3ERR_EXIST:
                PrintLog("EXIST");
                break;
            case NFS3ERR_XDEV:
                PrintLog("XDEV");
                break;
            case NFS3ERR_NODEV:
                PrintLog("NODEV");
                break;
            case NFS3ERR_NOTDIR:
                PrintLog("NOTDIR");
                break;
            case NFS3ERR_ISDIR:
                PrintLog("ISDIR");
                break;
            case NFS3ERR_INVAL:
                PrintLog("INVAL");
                break;
            case NFS3ERR_FBIG:
                PrintLog("FBIG");
                break;
            case NFS3ERR_NOSPC:
                PrintLog("NOSPC");
                break;
            case NFS3ERR_ROFS:
                PrintLog("ROFS");
                break;
            case NFS3ERR_MLINK:
                PrintLog("MLINK");
                break;
            case NFS3ERR_NAMETOOLONG:
                PrintLog("NAMETOOLONG");
                break;
            case NFS3ERR_NOTEMPTY:
                PrintLog("NOTEMPTY");
                break;
            case NFS3ERR_DQUOT:
                PrintLog("DQUOT");
                break;
            case NFS3ERR_STALE:
                PrintLog("STALE");
                break;
            case NFS3ERR_REMOTE:
                PrintLog("REMOTE");
                break;
            case NFS3ERR_BADHANDLE:
                PrintLog("BADHANDLE");
                break;
            case NFS3ERR_NOT_SYNC:
                PrintLog("NOT_SYNC");
                break;
            case NFS3ERR_BAD_COOKIE:
                PrintLog("BAD_COOKIE");
                break;
            case NFS3ERR_NOTSUPP:
                PrintLog("NOTSUPP");
                break;
            case NFS3ERR_TOOSMALL:
                PrintLog("TOOSMALL");
                break;
            case NFS3ERR_SERVERFAULT:
                PrintLog("SERVERFAULT");
                break;
            case NFS3ERR_BADTYPE:
                PrintLog("BADTYPE");
                break;
            case NFS3ERR_JUKEBOX:
                PrintLog("JUKEBOX");
                break;
            default:
                assert(false);
                break;
        }
    }

    PrintLog("\n");

    return m_nResult;
}

nfsstat3 CNFS3Prog::ProcedureNULL(void)
{
    PrintLog("NULL");
    return NFS3_OK;
}

nfsstat3 CNFS3Prog::ProcedureGETATTR(void)
{
	std::string path;
    fattr3 obj_attributes;
    nfsstat3 stat;
	char canPath[MAX_PATH];
	BOOL r;

    PrintLog("GETATTR");
    GetPath(path);

	memset(canPath, 0, sizeof(canPath));
	if (!path.empty()) {
		r = PathCanonicalize(canPath, path.c_str());
		if (!r)
			strncpy_s(canPath, path.c_str(), MAX_PATH);
	}

    stat = CheckFile(canPath);
	//printf("\nscanned file %s\n", path);
    if (stat == NFS3ERR_NOENT) {
        stat = NFS3ERR_STALE;
    } else if (stat == NFS3_OK) {
        if (!GetFileAttributesForNFS(canPath, &obj_attributes)) {
            stat = NFS3ERR_IO;
        }
    }

    Write(&stat);

    if (stat == NFS3_OK) {
        Write(&obj_attributes);
    }

    return stat;
}

nfsstat3 CNFS3Prog::ProcedureSETATTR(void)
{
    std::string path;
    sattr3 new_attributes;
    sattrguard3 guard;
    wcc_data obj_wcc;
    nfsstat3 stat;
    int nMode;
    FILE *pFile;
    HANDLE hFile;
    FILETIME fileTime;
    SYSTEMTIME systemTime;

    PrintLog("SETATTR");
    GetPath(path);
    Read(&new_attributes);
    Read(&guard);
    stat = CheckFile(path.c_str());
    obj_wcc.before.attributes_follow = GetFileAttributesForNFS(path.c_str(), &obj_wcc.before.attributes);

    if (stat == NFS3_OK) {
        if (new_attributes.mode.set_it) {
            nMode = 0;

            if ((new_attributes.mode.mode & 0x100) != 0) {
                nMode |= S_IREAD;
            }

            // Always set read and write permissions (deliberately implemented this way)
            // if ((new_attributes.mode.mode & 0x80) != 0) {
            nMode |= S_IWRITE;
            // }

            // S_IEXEC is not availabile on windows
            // if ((new_attributes.mode.mode & 0x40) != 0) {
            //     nMode |= S_IEXEC;
            // }

            if (_chmod(path.c_str(), nMode) != 0) {
                stat = NFS3ERR_INVAL;
            } else {

            }
        }   

        // deliberately not implemented because we cannot reflect uid/gid on windows (easliy)
        if (new_attributes.uid.set_it){}
        if (new_attributes.gid.set_it){}

        // deliberately not implemented
        if (new_attributes.mtime.set_it == SET_TO_CLIENT_TIME){}
        if (new_attributes.atime.set_it == SET_TO_CLIENT_TIME){}

        if (new_attributes.mtime.set_it == SET_TO_SERVER_TIME || new_attributes.atime.set_it == SET_TO_SERVER_TIME){
            hFile = CreateFile(path.c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
            if (hFile != INVALID_HANDLE_VALUE) {
                GetSystemTime(&systemTime);
                SystemTimeToFileTime(&systemTime, &fileTime);
                if (new_attributes.mtime.set_it == SET_TO_SERVER_TIME){
                    SetFileTime(hFile, NULL, NULL, &fileTime);
                }
                if (new_attributes.atime.set_it == SET_TO_SERVER_TIME){
                    SetFileTime(hFile, NULL, &fileTime, NULL);
                }
            }
            CloseHandle(hFile);
        }

        if (new_attributes.size.set_it){
            pFile = _fsopen(path.c_str(), "r+b", _SH_DENYWR);
            if (pFile != NULL) {
                int filedes = _fileno(pFile);
                _chsize_s(filedes, new_attributes.size.size);
                fclose(pFile);
            }
        }
    }

    obj_wcc.after.attributes_follow = GetFileAttributesForNFS(path.c_str(), &obj_wcc.after.attributes);

    Write(&stat);
    Write(&obj_wcc);

    return stat;
}

nfsstat3 CNFS3Prog::ProcedureLOOKUP(void)
{
    char *path;
    nfs_fh3 object;
    post_op_attr obj_attributes;
    post_op_attr dir_attributes;
    nfsstat3 stat;
	char canPath[MAX_PATH];
	BOOL r;

    PrintLog("LOOKUP");

    std::string dirName;
    std::string fileName;
    ReadDirectory(dirName, fileName);

    path = GetFullPath(dirName, fileName);

	r = PathCanonicalize(canPath, path);
	if (!r)
		strncpy_s(canPath, path, MAX_PATH);

    stat = CheckFile((char*)dirName.c_str(), canPath);

    if (stat == NFS3_OK) {
        GetFileHandle(canPath, &object);
        obj_attributes.attributes_follow = GetFileAttributesForNFS(canPath, &obj_attributes.attributes);
    }

    dir_attributes.attributes_follow = GetFileAttributesForNFS(dirName.c_str(), &dir_attributes.attributes);

	printf(" canPath = %s dirName = %s", canPath, dirName.c_str());
    Write(&stat);

    if (stat == NFS3_OK) {
        Write(&object);
        Write(&obj_attributes);
    }

    Write(&dir_attributes);

    return stat;
}

nfsstat3 CNFS3Prog::ProcedureACCESS(void)
{
	std::string path;
    uint32 access;
    post_op_attr obj_attributes;
    nfsstat3 stat;

    PrintLog("ACCESS");
    GetPath(path);
    Read(&access);
    stat = CheckFile(path.c_str());

    if (stat == NFS3ERR_NOENT) {
        stat = NFS3ERR_STALE;
    }

    obj_attributes.attributes_follow = GetFileAttributesForNFS(path.c_str(), &obj_attributes.attributes);

    Write(&stat);
    Write(&obj_attributes);

    if (stat == NFS3_OK) {
        Write(&access);
    }

    return stat;
}

nfsstat3 CNFS3Prog::ProcedureREADLINK(void)
{
    PrintLog("READLINK");
	std::string path;
    char *pMBBuffer = 0;

    post_op_attr symlink_attributes;
    nfspath3 data = nfspath3();

    //opaque data;
    nfsstat3 stat;

    HANDLE hFile;
    REPARSE_DATA_BUFFER *lpOutBuffer;
    lpOutBuffer = (REPARSE_DATA_BUFFER*)malloc(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
    DWORD bytesReturned;

    GetPath(path);
    stat = CheckFile(path.c_str());
    if (stat == NFS3_OK) {

        hFile = CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_REPARSE_POINT | FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);

        if (hFile == INVALID_HANDLE_VALUE) {
            stat = NFS3ERR_IO;
        }
        else
        {
            lpOutBuffer = (REPARSE_DATA_BUFFER*)malloc(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
            if (!lpOutBuffer) {
                stat = NFS3ERR_IO;
            }
            else {
                DeviceIoControl(hFile, FSCTL_GET_REPARSE_POINT, NULL, 0, lpOutBuffer, MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &bytesReturned, NULL);
                std::string finalSymlinkPath;
                if (lpOutBuffer->ReparseTag == IO_REPARSE_TAG_SYMLINK || lpOutBuffer->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT)
                {
                    if (lpOutBuffer->ReparseTag == IO_REPARSE_TAG_SYMLINK)
                    {
                        size_t plen = lpOutBuffer->SymbolicLinkReparseBuffer.PrintNameLength / sizeof(WCHAR);
                        WCHAR *szPrintName = new WCHAR[plen + 1];
                        wcsncpy_s(szPrintName, plen + 1, &lpOutBuffer->SymbolicLinkReparseBuffer.PathBuffer[lpOutBuffer->SymbolicLinkReparseBuffer.PrintNameOffset / sizeof(WCHAR)], plen);
                        szPrintName[plen] = 0;
                        std::wstring wStringTemp(szPrintName);
                        delete[] szPrintName;
                        std::string cPrintName(wStringTemp.begin(), wStringTemp.end());
                        finalSymlinkPath.assign(cPrintName);
                        // TODO: Revisit with cleaner solution
                        if (!PathIsRelative(cPrintName.c_str()))
                        {
                            std::string strFromChar;
                            strFromChar.append("\\\\?\\");
                            strFromChar.append(cPrintName);
                            char *target = _strdup(strFromChar.c_str());
                            // remove last folder
							path = path.substr(0, path.rfind('\\'));

                            char szOut[MAX_PATH] = "";
                            PathRelativePathTo(szOut, path.c_str(), FILE_ATTRIBUTE_DIRECTORY, target, FILE_ATTRIBUTE_DIRECTORY);
                            std::string symlinkPath(szOut);
                            finalSymlinkPath.assign(symlinkPath);
                        }
                    }

                    // TODO: Revisit with cleaner solution
                    if (lpOutBuffer->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT)
                    {
                        size_t slen = lpOutBuffer->MountPointReparseBuffer.SubstituteNameLength / sizeof(WCHAR);
                        WCHAR *szSubName = new WCHAR[slen + 1];
                        wcsncpy_s(szSubName, slen + 1, &lpOutBuffer->MountPointReparseBuffer.PathBuffer[lpOutBuffer->MountPointReparseBuffer.SubstituteNameOffset / sizeof(WCHAR)], slen);
                        szSubName[slen] = 0;
                        std::wstring wStringTemp(szSubName);
                        delete[] szSubName;
                        std::string target(wStringTemp.begin(), wStringTemp.end());
                        target.erase(0, 2);
                        target.insert(0, 2, '\\');
                        // remove last folder, see above
						path = path.substr(0, path.rfind('\\'));

                        char szOut[MAX_PATH] = "";
                        PathRelativePathTo(szOut, path.c_str(), FILE_ATTRIBUTE_DIRECTORY, target.c_str(), FILE_ATTRIBUTE_DIRECTORY);
                        std::string symlinkPath = szOut;
                        finalSymlinkPath.assign(symlinkPath);
                    }

                    // write path always with / separator, so windows created symlinks work too
                    std::replace(finalSymlinkPath.begin(), finalSymlinkPath.end(), '\\', '/');
                    char *result = _strdup(finalSymlinkPath.c_str());
                    data.Set(result);
                }
                free(lpOutBuffer);
            }
        }
        CloseHandle(hFile);
    }

    symlink_attributes.attributes_follow = GetFileAttributesForNFS(path.c_str(), &symlink_attributes.attributes);

    Write(&stat);
    Write(&symlink_attributes);
    if (stat == NFS3_OK) {
        Write(&data);
    }

    return stat;
}

nfsstat3 CNFS3Prog::ProcedureREAD(void)
{
	std::string path;
    offset3 offset;
    count3 count;
    post_op_attr file_attributes;
    bool eof;
    opaque data;
    nfsstat3 stat;
    FILE *pFile;

    PrintLog("READ");
    GetPath(path);
    Read(&offset);
    Read(&count);
    stat = CheckFile(path.c_str());

    if (stat == NFS3_OK) {
        data.SetSize(count);
        pFile = _fsopen(path.c_str(), "rb", _SH_DENYWR);

        if (pFile != NULL) {
            _fseeki64(pFile, offset, SEEK_SET) ;
            count = fread(data.contents, sizeof(char), count, pFile);
            eof = fgetc(pFile) == EOF;
            fclose(pFile);
        } else {
            char buffer[BUFFER_SIZE];
            errno_t errorNumber = errno;
            strerror_s(buffer, BUFFER_SIZE, errorNumber);
            PrintLog(buffer);

            if (errorNumber == 13) {
                stat = NFS3ERR_ACCES;
            } else {
                stat = NFS3ERR_IO;
            }
        }
    }

    file_attributes.attributes_follow = GetFileAttributesForNFS(path.c_str(), &file_attributes.attributes);

    Write(&stat);
    Write(&file_attributes);

    if (stat == NFS3_OK) {
        Write(&count);
        Write(&eof);
        Write(&data);
    }

    return stat;
}

nfsstat3 CNFS3Prog::ProcedureWRITE(void)
{
    std::string path;
    offset3 offset;
    count3 count;
    stable_how stable;
    opaque data;
    wcc_data file_wcc;
    writeverf3 verf;
    nfsstat3 stat;
    FILE *pFile;

    PrintLog("WRITE");
    GetPath(path);
    Read(&offset);
    Read(&count);
    Read(&stable);
    Read(&data);
    stat = CheckFile(path.c_str());

    file_wcc.before.attributes_follow = GetFileAttributesForNFS(path.c_str(), &file_wcc.before.attributes);

    if (stat == NFS3_OK) {       

        if (stable == UNSTABLE) {
            nfs_fh3 handle;
            GetFileHandle(path.c_str(), &handle);
            int handleId = *(unsigned int *)handle.contents;
            if (unstableStorageFile.count(handleId) == 0){
                unstableStorageFile.insert(std::make_pair(handleId, _fsopen(path.c_str(), "r+b", _SH_DENYWR)));
            }
            pFile = unstableStorageFile[handleId];
            if (pFile != NULL) {
                _fseeki64(pFile, offset, SEEK_SET);
                count = fwrite(data.contents, sizeof(char), data.length, pFile);
            } else {
                char buffer[BUFFER_SIZE];
                errno_t errorNumber = errno;
                strerror_s(buffer, BUFFER_SIZE, errorNumber);
                PrintLog(buffer);

                if (errorNumber == 13) {
                    stat = NFS3ERR_ACCES;
                }
                else {
                    stat = NFS3ERR_IO;
                }
            }
            // this should not be zero but a timestamp (process start time) instead
            verf = 0;
            // we can reuse this, because no physical write has happend
            file_wcc.after.attributes_follow = file_wcc.before.attributes_follow;
        } else {

            pFile = _fsopen(path.c_str(), "r+b", _SH_DENYWR);

            if (pFile != NULL) {
                _fseeki64(pFile, offset, SEEK_SET) ;
                count = fwrite(data.contents, sizeof(char), data.length, pFile);
                fclose(pFile);
            } else {
                char buffer[BUFFER_SIZE];
                errno_t errorNumber = errno;
                strerror_s(buffer, BUFFER_SIZE, errorNumber);
                PrintLog(buffer);

                if (errorNumber == 13) {
                    stat = NFS3ERR_ACCES;
                } else {
                    stat = NFS3ERR_IO;
                }
            }

            stable = FILE_SYNC;
            verf = 0;

            file_wcc.after.attributes_follow = GetFileAttributesForNFS(path.c_str(), &file_wcc.after.attributes);
        }
    }

    Write(&stat);
    Write(&file_wcc);

    if (stat == NFS3_OK) {
        Write(&count);
        Write(&stable);
        Write(&verf);
    }

    return stat;
}

nfsstat3 CNFS3Prog::ProcedureCREATE(void)
{
    char *path;
    createhow3 how;
    post_op_fh3 obj;
    post_op_attr obj_attributes;
    wcc_data dir_wcc;
    nfsstat3 stat;
    FILE *pFile;

    PrintLog("CREATE");
    std::string dirName;
    std::string fileName;
    ReadDirectory(dirName, fileName);
    path = GetFullPath(dirName, fileName);
    Read(&how);

    dir_wcc.before.attributes_follow = GetFileAttributesForNFS((char*)dirName.c_str(), &dir_wcc.before.attributes);

    pFile = _fsopen(path, "wb", _SH_DENYWR);
       
    if (pFile != NULL) {
        fclose(pFile);
        stat = NFS3_OK;
    } else {
        char buffer[BUFFER_SIZE];
        errno_t errorNumber = errno;
        strerror_s(buffer, BUFFER_SIZE, errorNumber);
        PrintLog(buffer);

        if (errorNumber == 2) {
            stat = NFS3ERR_STALE;
        } else if (errorNumber == 13) {
            stat = NFS3ERR_ACCES;
        } else {
            stat = NFS3ERR_IO;
        }
    }

    if (stat == NFS3_OK) {
        obj.handle_follows = GetFileHandle(path, &obj.handle);
        obj_attributes.attributes_follow = GetFileAttributesForNFS(path, &obj_attributes.attributes);
    }
    
    dir_wcc.after.attributes_follow = GetFileAttributesForNFS((char*)dirName.c_str(), &dir_wcc.after.attributes);

    Write(&stat);

    if (stat == NFS3_OK) {
        Write(&obj);
        Write(&obj_attributes);
    }

    Write(&dir_wcc);

    return stat;
}

nfsstat3 CNFS3Prog::ProcedureMKDIR(void)
{
    char *path;
    sattr3 attributes;
    post_op_fh3 obj;
    post_op_attr obj_attributes;
    wcc_data dir_wcc;
    nfsstat3 stat;

    PrintLog("MKDIR");

    std::string dirName;
    std::string fileName;
    ReadDirectory(dirName, fileName);
    path = GetFullPath(dirName, fileName);
    Read(&attributes);

    dir_wcc.before.attributes_follow = GetFileAttributesForNFS((char*)dirName.c_str(), &dir_wcc.before.attributes);

    int result = _mkdir(path);

    if (result == 0) {
        stat = NFS3_OK;
        obj.handle_follows = GetFileHandle(path, &obj.handle);
        obj_attributes.attributes_follow = GetFileAttributesForNFS(path, &obj_attributes.attributes);
    } else if (errno == EEXIST) {
        PrintLog("Directory already exists.");
        stat = NFS3ERR_EXIST;
    } else if (errno == ENOENT) {
        stat = NFS3ERR_NOENT;
    } else {
        stat = CheckFile(path);

        if (stat != NFS3_OK) {
            stat = NFS3ERR_IO;
        }
    }

    dir_wcc.after.attributes_follow = GetFileAttributesForNFS((char*)dirName.c_str(), &dir_wcc.after.attributes);

    Write(&stat);

    if (stat == NFS3_OK) {
        Write(&obj);
        Write(&obj_attributes);
    }

    Write(&dir_wcc);

    return stat;
}

nfsstat3 CNFS3Prog::ProcedureSYMLINK(void)
{
    PrintLog("SYMLINK");

    char* path;
    post_op_fh3 obj;
    post_op_attr obj_attributes;
    wcc_data dir_wcc;
    nfsstat3 stat;

    diropargs3 where;
    symlinkdata3 symlink;

    DWORD targetFileAttr;
    DWORD dwFlags;

    std::string dirName;
    std::string fileName;
    ReadDirectory(dirName, fileName);
    path = GetFullPath(dirName, fileName);

    Read(&symlink);

    _In_ LPTSTR lpSymlinkFileName = path; // symlink (full path)

    // TODO: Maybe revisit this later for a cleaner solution
    // Convert target path to windows path format, maybe this could also be done
    // in a safer way by a combination of PathRelativePathTo and GetFullPathName.
    // Without this conversion nested folder symlinks do not work cross platform.
    std::string strFromChar;
    strFromChar.append(symlink.symlink_data.path); // target (should be relative path));
    std::replace(strFromChar.begin(), strFromChar.end(), '/', '\\');
    _In_ LPTSTR lpTargetFileName = const_cast<LPSTR>(strFromChar.c_str());

    std::string fullTargetPath = dirName + std::string("\\") + std::string(lpTargetFileName);

    // Relative path do not work with GetFileAttributes (directory are not recognized)
    // so we normalize the path before calling GetFileAttributes
    TCHAR fullTargetPathNormalized[MAX_PATH];
    _In_ LPTSTR fullTargetPathString = const_cast<LPSTR>(fullTargetPath.c_str());;
    GetFullPathName(fullTargetPathString, MAX_PATH, fullTargetPathNormalized, NULL);
    targetFileAttr = GetFileAttributes(fullTargetPathNormalized);

    dwFlags = 0x0;
	if (targetFileAttr & FILE_ATTRIBUTE_DIRECTORY) {
        dwFlags = SYMBOLIC_LINK_FLAG_DIRECTORY;
    }

    BOOLEAN failed = CreateSymbolicLink(lpSymlinkFileName, lpTargetFileName, dwFlags);

    if (failed != 0) {
        stat = NFS3_OK;
        obj.handle_follows = GetFileHandle(path, &obj.handle);
        obj_attributes.attributes_follow = GetFileAttributesForNFS(path, &obj_attributes.attributes);
    }
    else {
        stat = NFS3ERR_IO;
        PrintLog("An error occurs or file already exists.");
        stat = CheckFile(path);
        if (stat != NFS3_OK) {
            stat = NFS3ERR_IO;
        }
    }

    dir_wcc.after.attributes_follow = GetFileAttributesForNFS((char*)dirName.c_str(), &dir_wcc.after.attributes);

    Write(&stat);

    if (stat == NFS3_OK) {
        Write(&obj);
        Write(&obj_attributes);
    }

    Write(&dir_wcc);

    return stat;
}

nfsstat3 CNFS3Prog::ProcedureMKNOD(void)
{
    //TODO
    PrintLog("MKNOD");

    return NFS3ERR_NOTSUPP;
}

nfsstat3 CNFS3Prog::ProcedureREMOVE(void)
{
    char *path;
    wcc_data dir_wcc;
    nfsstat3 stat;

    PrintLog("REMOVE");

    std::string dirName;
    std::string fileName;
    ReadDirectory(dirName, fileName);
    path = GetFullPath(dirName, fileName);
    stat = CheckFile((char*)dirName.c_str(), path);

    dir_wcc.before.attributes_follow = GetFileAttributesForNFS((char*)dirName.c_str(), &dir_wcc.before.attributes);

    if (stat == NFS3_OK) {
        DWORD fileAttr = GetFileAttributes(path);
        if ((fileAttr & FILE_ATTRIBUTE_DIRECTORY) && (fileAttr & FILE_ATTRIBUTE_REPARSE_POINT)) {
			if (RemoveFolder(path) == 0) {
                stat = NFS3ERR_IO;
            }
        } else {
            if (!RemoveFile(path)) {
                stat = NFS3ERR_IO;
            }
        }
    }

    dir_wcc.after.attributes_follow = GetFileAttributesForNFS((char*)dirName.c_str(), &dir_wcc.after.attributes);

    Write(&stat);
    Write(&dir_wcc);

    return stat;
}

nfsstat3 CNFS3Prog::ProcedureRMDIR(void)
{
    char *path;
    wcc_data dir_wcc;
    nfsstat3 stat;

    PrintLog("RMDIR");

    std::string dirName;
    std::string fileName;
    ReadDirectory(dirName, fileName);
    path = GetFullPath(dirName, fileName);
    stat = CheckFile((char*)dirName.c_str(), path);

    dir_wcc.before.attributes_follow = GetFileAttributesForNFS((char*)dirName.c_str(), &dir_wcc.before.attributes);

    if (stat == NFS3_OK) {
        if (!RemoveFolder(path)) {
            stat = NFS3ERR_IO;
        }         
    }
    
    dir_wcc.after.attributes_follow = GetFileAttributesForNFS((char*)dirName.c_str(), &dir_wcc.after.attributes);

    Write(&stat);
    Write(&dir_wcc);

    return stat;
}

nfsstat3 CNFS3Prog::ProcedureRENAME(void)
{
    char pathFrom[MAXPATHLEN], *pathTo;
    wcc_data fromdir_wcc, todir_wcc;
    nfsstat3 stat;

    PrintLog("RENAME");

    std::string dirFromName;
    std::string fileFromName;
    ReadDirectory(dirFromName, fileFromName);
    strcpy_s(pathFrom, GetFullPath(dirFromName, fileFromName));

    std::string dirToName;
    std::string fileToName;
    ReadDirectory(dirToName, fileToName);
    pathTo = GetFullPath(dirToName, fileToName);

    stat = CheckFile((char*)dirFromName.c_str(), pathFrom);

    fromdir_wcc.before.attributes_follow = GetFileAttributesForNFS((char*)dirFromName.c_str(), &fromdir_wcc.before.attributes);
    todir_wcc.before.attributes_follow = GetFileAttributesForNFS((char*)dirToName.c_str(), &todir_wcc.before.attributes);
    
    if (FileExists(pathTo)) {
		DWORD fileAttr = GetFileAttributes(pathTo);
		if ((fileAttr & FILE_ATTRIBUTE_DIRECTORY) && (fileAttr & FILE_ATTRIBUTE_REPARSE_POINT)) {
			if (RemoveFolder(pathTo) == 0) {
				stat = NFS3ERR_IO;
			}
		}
		else {
			if (!RemoveFile(pathTo)) {
				stat = NFS3ERR_IO;
			}
		}
    } 
    
    if (stat == NFS3_OK) {
        errno_t errorNumber = RenameDirectory(pathFrom, pathTo);

        if (errorNumber != 0) {
            char buffer[BUFFER_SIZE];
            strerror_s(buffer, BUFFER_SIZE, errorNumber);
            PrintLog(buffer);

            if (errorNumber == 13) {
                stat = NFS3ERR_ACCES;
            } else {
                stat = NFS3ERR_IO;
            }
        }
    }

    fromdir_wcc.after.attributes_follow = GetFileAttributesForNFS((char*)dirFromName.c_str(), &fromdir_wcc.after.attributes);
    todir_wcc.after.attributes_follow = GetFileAttributesForNFS((char*)dirToName.c_str(), &todir_wcc.after.attributes);

    Write(&stat);
    Write(&fromdir_wcc);
    Write(&todir_wcc);

    return stat;
}

nfsstat3 CNFS3Prog::ProcedureLINK(void)
{
    PrintLog("LINK");
    std::string filePath;
    diropargs3 link;
    std::string dirName;
    std::string fileName;
    nfsstat3 stat;
    post_op_attr obj_attributes;
    wcc_data dir_wcc;

    GetPath(filePath);
    ReadDirectory(dirName, fileName);

    char *linkFullPath = GetFullPath(dirName, fileName);

    if (CreateHardLink(linkFullPath, filePath.c_str(), NULL) == 0) {
        stat = NFS3ERR_IO;
    }
    stat = CheckFile(linkFullPath);
    if (stat == NFS3_OK) {
        obj_attributes.attributes_follow = GetFileAttributesForNFS(filePath.c_str(), &obj_attributes.attributes);

        if (!obj_attributes.attributes_follow) {
            stat = NFS3ERR_IO;
        }
    }

    dir_wcc.after.attributes_follow = GetFileAttributesForNFS((char*)dirName.c_str(), &dir_wcc.after.attributes);

    Write(&stat);
    Write(&obj_attributes);
    Write(&dir_wcc);

    return stat;
}

nfsstat3 CNFS3Prog::ProcedureREADDIR(void)
{
    std::string path;
    cookie3 cookie;
    cookieverf3 cookieverf;
    count3 count;
    post_op_attr dir_attributes;
    fileid3 fileid;
    filename3 name;
    bool eof;
    bool bFollows;
    nfsstat3 stat;
    char filePath[MAXPATHLEN];
    int handle, nFound;
    struct _finddata_t fileinfo;
    unsigned int i, j;

    PrintLog("READDIR");
    GetPath(path);
    Read(&cookie);
    Read(&cookieverf);
    Read(&count);
    stat = CheckFile(path.c_str());

    if (stat == NFS3_OK) {
        dir_attributes.attributes_follow = GetFileAttributesForNFS(path.c_str(), &dir_attributes.attributes);

        if (!dir_attributes.attributes_follow) {
            stat = NFS3ERR_IO;
        }    
    }

    Write(&stat);
    Write(&dir_attributes);

    if (stat == NFS3_OK) {
        Write(&cookieverf);
        sprintf_s(filePath, "%s\\*", path.c_str());
        eof = true;
        handle = _findfirst(filePath, &fileinfo);
        bFollows = true;

        if (handle) {
            nFound = 0;

            for (i = (unsigned int)cookie; i > 0; i--) {
                nFound = _findnext(handle, &fileinfo);
            }

            // TODO: Implement this workaround correctly with the
            // count variable and not a fixed threshold of 10
            if (nFound == 0) {
                j = 10;

                do {
                    Write(&bFollows); //value follows
                    sprintf_s(filePath, "%s\\%s", path.c_str(), fileinfo.name);
                    fileid = GetFileID(filePath);
                    Write(&fileid); //file id
                    name.Set(fileinfo.name);
                    Write(&name); //name
                    ++cookie;
                    Write(&cookie); //cookie
                    if (--j == 0) {
                        eof = false;
                        break;
                    }
                } while (_findnext(handle, &fileinfo) == 0);
            }

            _findclose(handle);
        }

        bFollows = false;
        Write(&bFollows);
        Write(&eof); //eof
    }

    return stat;
}

nfsstat3 CNFS3Prog::ProcedureREADDIRPLUS(void)
{
    std::string path;
    cookie3 cookie;
    cookieverf3 cookieverf;
    count3 dircount, maxcount;
    post_op_attr dir_attributes;
    fileid3 fileid;
    filename3 name;
    post_op_attr name_attributes;
    post_op_fh3 name_handle;
    bool eof;
    nfsstat3 stat;
    char filePath[MAXPATHLEN];
    int handle, nFound;
    struct _finddata_t fileinfo;
    unsigned int i, j;
    bool bFollows;
	char canPath[MAX_PATH];
	BOOL r;


    PrintLog("READDIRPLUS");
    GetPath(path);

	memset(canPath, 0, sizeof(canPath));
	if (path.c_str()) {
		r = PathCanonicalize(canPath, path.c_str());
		if (!r)
			strncpy_s(canPath, path.c_str(), MAX_PATH);
	}

    Read(&cookie);
    Read(&cookieverf);
    Read(&dircount);
    Read(&maxcount);
    stat = CheckFile(canPath);

    if (stat == NFS3_OK) {
        dir_attributes.attributes_follow = GetFileAttributesForNFS(canPath, &dir_attributes.attributes);
        
        if (!dir_attributes.attributes_follow) {
            stat = NFS3ERR_IO;
        }
    }

    Write(&stat);
    Write(&dir_attributes);

	PrintLog("READDIRPLUS");

    if (stat == NFS3_OK) {
        Write(&cookieverf);
        sprintf_s(filePath, "%s\\*", canPath);
        handle = _findfirst(filePath, &fileinfo);
        eof = true;

        if (handle > -1) {
            nFound = 0;

            for (i = (unsigned int)cookie; i > 0; i--) {
                nFound = _findnext(handle, &fileinfo);
            }              

            if (nFound == 0) {
                bFollows = true;
                j = 10;

                do {
                    Write(&bFollows); //value follows
                    sprintf_s(filePath, "%s\\%s", canPath, fileinfo.name);					

                    fileid = GetFileID(filePath);
                    Write(&fileid); //file id
                    name.Set(fileinfo.name);
                    Write(&name); //name
                    ++cookie;
                    Write(&cookie); //cookie
                    name_attributes.attributes_follow = GetFileAttributesForNFS(filePath, &name_attributes.attributes);
                    Write(&name_attributes);
                    name_handle.handle_follows = GetFileHandle(filePath, &name_handle.handle);
                    Write(&name_handle);

                    if (--j == 0) {
                        eof = false;
                        break;
                    }
                } while (_findnext(handle, &fileinfo) == 0);

            }

            _findclose(handle);
        }

        bFollows = false;
        Write(&bFollows); //value follows
        Write(&eof); //eof
    }

    return stat;
}

nfsstat3 CNFS3Prog::ProcedureFSSTAT(void)
{
    std::string path;
    post_op_attr obj_attributes;
    size3 tbytes, fbytes, abytes, tfiles, ffiles, afiles;
    uint32 invarsec;

    nfsstat3 stat;

    PrintLog("FSSTAT");
    GetPath(path);
    stat = CheckFile(path.c_str());

    if (stat == NFS3_OK) {
        obj_attributes.attributes_follow = GetFileAttributesForNFS(path.c_str(), &obj_attributes.attributes);

        if (obj_attributes.attributes_follow
            && GetDiskFreeSpaceEx(path.c_str(), (PULARGE_INTEGER)&fbytes, (PULARGE_INTEGER)&tbytes, (PULARGE_INTEGER)&abytes)
            ) {
            //tfiles = 99999999999;
            //ffiles = 99999999999;
            //afiles = 99999999999;
            invarsec = 0;
        } else {
            stat = NFS3ERR_IO;
        }
    }

    Write(&stat);
    Write(&obj_attributes);

    if (stat == NFS3_OK) {
        Write(&tbytes);
        Write(&fbytes);
        Write(&abytes);
        Write(&tfiles);
        Write(&ffiles);
        Write(&afiles);
        Write(&invarsec);
    }

    return stat;
}

nfsstat3 CNFS3Prog::ProcedureFSINFO(void)
{
    std::string path;
    post_op_attr obj_attributes;
    uint32 rtmax, rtpref, rtmult, wtmax, wtpref, wtmult, dtpref;
    size3 maxfilesize;
    nfstime3 time_delta;
    uint32 properties;
    nfsstat3 stat;

    PrintLog("FSINFO");
    GetPath(path);
    stat = CheckFile(path.c_str());

    if (stat == NFS3_OK) {
        obj_attributes.attributes_follow = GetFileAttributesForNFS(path.c_str(), &obj_attributes.attributes);

        if (obj_attributes.attributes_follow) {
            rtmax = 65536;
            rtpref = 32768;
            rtmult = 4096;
            wtmax = 65536;
            wtpref = 32768;
            wtmult = 4096;
            dtpref = 8192;
            maxfilesize = 0x7FFFFFFFFFFFFFFF;
            time_delta.seconds = 1;
            time_delta.nseconds = 0;
            properties = FSF3_LINK | FSF3_SYMLINK | FSF3_CANSETTIME;
        } else {
            stat = NFS3ERR_SERVERFAULT;
        }         
    }

    Write(&stat);
    Write(&obj_attributes);

    if (stat == NFS3_OK) {
        Write(&rtmax);
        Write(&rtpref);
        Write(&rtmult);
        Write(&wtmax);
        Write(&wtpref);
        Write(&wtmult);
        Write(&dtpref);
        Write(&maxfilesize);
        Write(&time_delta);
        Write(&properties);
    }

    return stat;
}

nfsstat3 CNFS3Prog::ProcedurePATHCONF(void)
{
    std::string path;
    post_op_attr obj_attributes;
    nfsstat3 stat;
    uint32 linkmax, name_max;
    bool no_trunc, chown_restricted, case_insensitive, case_preserving;

    PrintLog("PATHCONF");
    GetPath(path);
    stat = CheckFile(path.c_str());

    if (stat == NFS3_OK) {
        obj_attributes.attributes_follow = GetFileAttributesForNFS(path.c_str(), &obj_attributes.attributes);

        if (obj_attributes.attributes_follow) {
            linkmax = 1023;
            name_max = 255;
            no_trunc = true;
            chown_restricted = true;
            case_insensitive = true;
            case_preserving = true;
        } else {
            stat = NFS3ERR_SERVERFAULT;
        }
    }

    Write(&stat);
    Write(&obj_attributes);

    if (stat == NFS3_OK) {
        Write(&linkmax);
        Write(&name_max);
        Write(&no_trunc);
        Write(&chown_restricted);
        Write(&case_insensitive);
        Write(&case_preserving);
    }

    return stat;
}

nfsstat3 CNFS3Prog::ProcedureCOMMIT(void)
{
	std::string path;
    int handleId;
    offset3 offset;
    count3 count;
    wcc_data file_wcc;
    nfsstat3 stat;
    nfs_fh3 file;
    writeverf3 verf;

    PrintLog("COMMIT");
    Read(&file);
    GetFilePath(file.contents, path);

    // offset and count are unused
    // offset never was anything else than 0 in my tests
    // count does not matter in the way COMMIT is implemented here
    // to fulfill the spec this should be improved
    Read(&offset);
    Read(&count);

    file_wcc.before.attributes_follow = GetFileAttributesForNFS(path.c_str(), &file_wcc.before.attributes);

    handleId = *(unsigned int*)file.contents;

    if (unstableStorageFile[handleId] != NULL) {
        fclose(unstableStorageFile[handleId]);
        unstableStorageFile.erase(handleId);
        stat = NFS3_OK;
    } else {
        stat = NFS3ERR_IO;
    }

    file_wcc.after.attributes_follow = GetFileAttributesForNFS(path.c_str(), &file_wcc.after.attributes);

    Write(&stat);
    Write(&file_wcc);
    // verf should be the timestamp the server startet to notice reboots
    verf = 0;
    Write(&verf);

    return stat;
}

nfsstat3 CNFS3Prog::ProcedureNOIMP(void)
{
    PrintLog("NOIMP");
    m_nResult = PRC_NOTIMP;

    return NFS3_OK;
}

void CNFS3Prog::Read(bool *pBool)
{
    uint32 b;

    if (m_pInStream->Read(&b) < sizeof(uint32)) {
        throw __LINE__;
    }
        
    *pBool = b == 1;
}

void CNFS3Prog::Read(uint32 *pUint32)
{
    if (m_pInStream->Read(pUint32) < sizeof(uint32)) {
        throw __LINE__;
    }       
}

void CNFS3Prog::Read(uint64 *pUint64)
{
    if (m_pInStream->Read8(pUint64) < sizeof(uint64)) {
        throw __LINE__;
    }        
}

void CNFS3Prog::Read(sattr3 *pAttr)
{
    Read(&pAttr->mode.set_it);

    if (pAttr->mode.set_it) {
        Read(&pAttr->mode.mode);
    }
        
    Read(&pAttr->uid.set_it);

    if (pAttr->uid.set_it) {
        Read(&pAttr->uid.uid);
    }  

    Read(&pAttr->gid.set_it);

    if (pAttr->gid.set_it) {
        Read(&pAttr->gid.gid);
    }       

    Read(&pAttr->size.set_it);

    if (pAttr->size.set_it) {
        Read(&pAttr->size.size);
    }       

    Read(&pAttr->atime.set_it);

    if (pAttr->atime.set_it == SET_TO_CLIENT_TIME) {
        Read(&pAttr->atime.atime);
    }
        
    Read(&pAttr->mtime.set_it);

    if (pAttr->mtime.set_it == SET_TO_CLIENT_TIME) {
        Read(&pAttr->mtime.mtime);
    }        
}

void CNFS3Prog::Read(sattrguard3 *pGuard)
{
    Read(&pGuard->check);

    if (pGuard->check) {
        Read(&pGuard->obj_ctime);
    }        
}

void CNFS3Prog::Read(diropargs3 *pDir)
{
    Read(&pDir->dir);
    Read(&pDir->name);
}

void CNFS3Prog::Read(opaque *pOpaque)
{
    uint32 len, byte;

    Read(&len);
    pOpaque->SetSize(len);

    if (m_pInStream->Read(pOpaque->contents, len) < len) {
        throw __LINE__;
    }        

    len = 4 - (len & 3);

    if (len != 4) {
        if (m_pInStream->Read(&byte, len) < len) {
            throw __LINE__;
        }            
    }
}

void CNFS3Prog::Read(nfstime3 *pTime)
{
    Read(&pTime->seconds);
    Read(&pTime->nseconds);
}

void CNFS3Prog::Read(createhow3 *pHow)
{
    Read(&pHow->mode);

    if (pHow->mode == UNCHECKED || pHow->mode == GUARDED) {
        Read(&pHow->obj_attributes);
    } else {
        Read(&pHow->verf);
    }       
}

void CNFS3Prog::Read(symlinkdata3 *pSymlink)
{
	Read(&pSymlink->symlink_attributes);
	Read(&pSymlink->symlink_data);
}

void CNFS3Prog::Write(bool *pBool)
{
    m_pOutStream->Write(*pBool ? 1 : 0);
}

void CNFS3Prog::Write(uint32 *pUint32)
{
    m_pOutStream->Write(*pUint32);
}

void CNFS3Prog::Write(uint64 *pUint64)
{
    m_pOutStream->Write8(*pUint64);
}

void CNFS3Prog::Write(fattr3 *pAttr)
{
    Write(&pAttr->type);
    Write(&pAttr->mode);
    Write(&pAttr->nlink);
    Write(&pAttr->uid);
    Write(&pAttr->gid);
    Write(&pAttr->size);
    Write(&pAttr->used);
    Write(&pAttr->rdev);
    Write(&pAttr->fsid);
    Write(&pAttr->fileid);
    Write(&pAttr->atime);
    Write(&pAttr->mtime);
    Write(&pAttr->ctime);
}

void CNFS3Prog::Write(opaque *pOpaque)
{
    uint32 len, byte;

    Write(&pOpaque->length);
    m_pOutStream->Write(pOpaque->contents, pOpaque->length);
    len = pOpaque->length & 3;

    if (len != 0) {
        byte = 0;
        m_pOutStream->Write(&byte, 4 - len);
    }
}

void CNFS3Prog::Write(wcc_data *pWcc)
{
    Write(&pWcc->before);
    Write(&pWcc->after);
}

void CNFS3Prog::Write(post_op_attr *pAttr)
{
    Write(&pAttr->attributes_follow);

    if (pAttr->attributes_follow) {
        Write(&pAttr->attributes);
    }      
}

void CNFS3Prog::Write(pre_op_attr *pAttr)
{
    Write(&pAttr->attributes_follow);

    if (pAttr->attributes_follow) {
        Write(&pAttr->attributes);
    }    
}

void CNFS3Prog::Write(post_op_fh3 *pObj)
{
    Write(&pObj->handle_follows);

    if (pObj->handle_follows) {
        Write(&pObj->handle);
    }     
}

void CNFS3Prog::Write(nfstime3 *pTime)
{
    Write(&pTime->seconds);
    Write(&pTime->nseconds);
}

void CNFS3Prog::Write(specdata3 *pSpec)
{
    Write(&pSpec->specdata1);
    Write(&pSpec->specdata2);
}

void CNFS3Prog::Write(wcc_attr *pAttr)
{
    Write(&pAttr->size);
    Write(&pAttr->mtime);
    Write(&pAttr->ctime);
}

void CNFS3Prog::GetPath(std::string& path)
{
    nfs_fh3 object;

    Read(&object);
    GetFilePath(object.contents, path);
    PrintLog(" %s ", path.c_str());
}

void CNFS3Prog::ReadDirectory(std::string &dirName, std::string &fileName)
{
    diropargs3 fileRequest;
    Read(&fileRequest);

    GetFilePath(fileRequest.dir.contents, dirName);
    fileName = std::string(fileRequest.name.name);

    PrintLog("(%d) %s | %s ", (int)*fileRequest.dir.contents, dirName.c_str(), fileName.c_str());
}

char *CNFS3Prog::GetFullPath(void)
{
    std::string dirName;
    std::string fileName;

    ReadDirectory(dirName, fileName);
    return GetFullPath(dirName, fileName);
}

char *CNFS3Prog::GetFullPath(std::string &dirName, std::string &fileName)
{
    static char fullPath[MAXPATHLEN + 1];

    if (dirName.size() + 1 + fileName.size() > MAXPATHLEN) {
        return NULL;
    }

    sprintf_s(fullPath, "%s\\%s", dirName.c_str(), fileName.c_str()); //concate path and filename
    PrintLog(" %s ", fullPath);

    return fullPath;
}

nfsstat3 CNFS3Prog::CheckFile(const char *fullPath)
{
    if (fullPath == NULL) {
        return NFS3ERR_STALE;
    }

	//if (!FileExists(fullPath)) {
	if (_access(fullPath, 0) != 0) {
        return NFS3ERR_NOENT;
    }

    return NFS3_OK;
}

nfsstat3 CNFS3Prog::CheckFile(char *directory, char *fullPath)
{
    if (directory == NULL || !FileExists(directory) || fullPath == NULL) {
        return NFS3ERR_STALE;
    }
        
    if (!FileExists(fullPath)) {
        return NFS3ERR_NOENT;
    }
        
    return NFS3_OK;
}

bool CNFS3Prog::GetFileHandle(const char *path, nfs_fh3 *pObject)
{
	unsigned char* handle;

	handle = ::GetFileHandle(path);
	if (!handle) {
		PrintLog("no filehandle(path %s)", path);
		return false;
	}
    auto err = memcpy_s(pObject->contents, g_NFS3_FHSIZE, handle, pObject->length);
	PrintLog("Get FH (path %s)", path);
	if (err != 0) {
		PrintLog(" err %d ", err);
		return false;
	}

    return true;
}

bool CNFS3Prog::GetFileAttributesForNFS(const char *path, wcc_attr *pAttr)
{
    struct stat data;

    if (path == NULL) {
        return false;
    }

    if (stat(path, &data) != 0) {
        return false;
    }

    pAttr->size = data.st_size;
    pAttr->mtime.seconds = (unsigned int)data.st_mtime;
    pAttr->mtime.nseconds = 0;
	// TODO: This needs to be tested (not called on my setup)
	// This seems to be the changed time, not creation time.
    //pAttr->ctime.seconds = data.st_ctime;
    pAttr->ctime.seconds = (unsigned int)data.st_mtime;
    pAttr->ctime.nseconds = 0;

    return true;
}

bool CNFS3Prog::GetFileAttributesForNFS(const char *path, fattr3 *pAttr)
{
    DWORD fileAttr;
    BY_HANDLE_FILE_INFORMATION lpFileInformation;
    HANDLE hFile;
    DWORD dwFlagsAndAttributes;
	char canPath[MAX_PATH];
	BOOL r;

	if (path == NULL)
		return false;

	r = PathCanonicalize(canPath, path);
	if (!r)
		strncpy_s(canPath, path, MAX_PATH);

    fileAttr = GetFileAttributes(canPath);

    if (fileAttr == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }

    dwFlagsAndAttributes = 0;
    if (fileAttr & FILE_ATTRIBUTE_DIRECTORY) {
        pAttr->type = NF3DIR;
        dwFlagsAndAttributes = FILE_ATTRIBUTE_DIRECTORY | FILE_FLAG_BACKUP_SEMANTICS;
    }
    else if (fileAttr & FILE_ATTRIBUTE_ARCHIVE) {
        pAttr->type = NF3REG;
        dwFlagsAndAttributes = FILE_ATTRIBUTE_ARCHIVE | FILE_FLAG_OVERLAPPED;
    }
    else if (fileAttr & FILE_ATTRIBUTE_NORMAL) {
        pAttr->type = NF3REG;
        dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED;
    }
    else {
        pAttr->type = 0;
    }

    if (fileAttr & FILE_ATTRIBUTE_REPARSE_POINT) {
        pAttr->type = NF3LNK;
        dwFlagsAndAttributes = FILE_ATTRIBUTE_REPARSE_POINT | FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS;
    }

    hFile = CreateFile(canPath, FILE_READ_EA, FILE_SHARE_READ, NULL, OPEN_EXISTING, dwFlagsAndAttributes, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    if ((fileAttr & FILE_ATTRIBUTE_REPARSE_POINT) && (fileAttr & FILE_ATTRIBUTE_DIRECTORY)) {
    	REPARSE_DATA_BUFFER *lpOutBuffer;
    	lpOutBuffer = (REPARSE_DATA_BUFFER*)malloc(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
    	DWORD bytesReturned;
    	BOOL bRet;
    	if (!lpOutBuffer)
    		return false;

    	bRet = DeviceIoControl(hFile, FSCTL_GET_REPARSE_POINT, NULL, 0, lpOutBuffer, MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &bytesReturned, NULL);
    	if (bRet && (lpOutBuffer->ReparseTag == IO_REPARSE_TAG_SYMLINK))
    		pAttr->type = NF3LNK;
    	else
    		pAttr->type = NF3DIR;

    	free(lpOutBuffer);
    }

    GetFileInformationByHandle(hFile, &lpFileInformation);
    CloseHandle(hFile);
    pAttr->mode = 0;

	// Set execution right for all
    pAttr->mode |= 0x49;

    // Set read right for all
    pAttr->mode |= 0x124;

    //if ((lpFileInformation.dwFileAttributes & FILE_ATTRIBUTE_READONLY) == 0) {
        pAttr->mode |= 0x92;
    //}

    ULONGLONG fileSize = lpFileInformation.nFileSizeHigh;
    fileSize <<= sizeof(lpFileInformation.nFileSizeHigh) * 8;
    fileSize |= lpFileInformation.nFileSizeLow;

    pAttr->nlink = lpFileInformation.nNumberOfLinks;
    pAttr->uid = m_nUID;
    pAttr->gid = m_nGID;
    pAttr->size = fileSize;
    pAttr->used = pAttr->size;
    pAttr->rdev.specdata1 = 0;
    pAttr->rdev.specdata2 = 0;
    pAttr->fsid = 7; //NTFS //4; 
    pAttr->fileid = GetFileID(canPath);
    pAttr->atime.seconds = FileTimeToPOSIX(lpFileInformation.ftLastAccessTime);
    pAttr->atime.nseconds = 0;
    pAttr->mtime.seconds = FileTimeToPOSIX(lpFileInformation.ftLastWriteTime);
    pAttr->mtime.nseconds = 0;
	// This seems to be the changed time, not creation time
	pAttr->ctime.seconds = FileTimeToPOSIX(lpFileInformation.ftLastWriteTime);
    pAttr->ctime.nseconds = 0;

    return true;
}

UINT32 CNFS3Prog::FileTimeToPOSIX(FILETIME ft)
{
    // takes the last modified date
    LARGE_INTEGER date, adjust;
    date.HighPart = ft.dwHighDateTime;
    date.LowPart = ft.dwLowDateTime;

    // 100-nanoseconds = milliseconds * 10000
    adjust.QuadPart = 11644473600000 * 10000;

    // removes the diff between 1970 and 1601
    date.QuadPart -= adjust.QuadPart;

    // converts back from 100-nanoseconds to seconds
    return (unsigned int)(date.QuadPart / 10000000);
}
