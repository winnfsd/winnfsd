#include "NFSProg.h"
#include "FileTable.h"
#include <string.h>
#include <io.h>
#include <direct.h>
#include <sys/stat.h>
#include <assert.h>
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

nfs_fh3::nfs_fh3() : opaque(NFS3_FHSIZE)
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
        &CNFS3Prog::ProcedureLOOKUP, &CNFS3Prog::ProcedureACCESS, &CNFS3Prog::ProcedureNOIMP, 
        &CNFS3Prog::ProcedureREAD, &CNFS3Prog::ProcedureWRITE, &CNFS3Prog::ProcedureCREATE, 
        &CNFS3Prog::ProcedureMKDIR, &CNFS3Prog::ProcedureNOIMP, &CNFS3Prog::ProcedureNOIMP, 
        &CNFS3Prog::ProcedureREMOVE, &CNFS3Prog::ProcedureRMDIR, &CNFS3Prog::ProcedureRENAME, 
        &CNFS3Prog::ProcedureNOIMP, &CNFS3Prog::ProcedureREADDIR, &CNFS3Prog::ProcedureREADDIRPLUS, 
        &CNFS3Prog::ProcedureNOIMP, &CNFS3Prog::ProcedureFSINFO 
    };

    nfsstat3 stat;

    PrintLog("NFS ");

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
    char *path;
    fattr3 obj_attributes;
    nfsstat3 stat;

    PrintLog("GETATTR");
    path = GetPath();
    stat = CheckFile(path);

    if (stat == NFS3_OK) {
        if (!GetFileAttributes(path, &obj_attributes)) {
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
    char *path;
    sattr3 new_attributes;
    sattrguard3 guard;
    wcc_data obj_wcc;
    nfsstat3 stat;
    int nMode;

    PrintLog("SETATTR");
    path = GetPath();
    Read(&new_attributes);
    Read(&guard);
    stat = CheckFile(path);

    if (stat == NFS3_OK) {
        if (new_attributes.mode.set_it) {
            nMode = 0;

            if ((new_attributes.mode.mode & 0x100) != 0) {
                nMode |= S_IREAD;
            }

            if ((new_attributes.mode.mode & 0x80) != 0) {
                nMode |= S_IWRITE;
            }

            if (_chmod(path, nMode) != 0) {
                stat = NFS3ERR_INVAL;
            }
        }

        obj_wcc.before.attributes_follow = false;
        obj_wcc.after.attributes_follow = GetFileAttributes(path, &obj_wcc.after.attributes);
    } else {
        obj_wcc.before.attributes_follow = false;
        obj_wcc.after.attributes_follow = false;
    }

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

    PrintLog("LOOKUP");
    path = GetFullPath();

    stat = CheckFile(path);

    if (stat == NFS3_OK) {
        GetFileHandle(path, &object);
        obj_attributes.attributes_follow = GetFileAttributes(path, &obj_attributes.attributes);
    }

    dir_attributes.attributes_follow = false;

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
    char *path;
    uint32 access;
    post_op_attr obj_attributes;
    nfsstat3 stat;

    PrintLog("ACCESS");
    path = GetPath();
    Read(&access);
    stat = CheckFile(path);

    if (stat == NFS3_OK) {
        obj_attributes.attributes_follow = GetFileAttributes(path, &obj_attributes.attributes);
    } else {
        obj_attributes.attributes_follow = false;
    }

    Write(&stat);
    Write(&obj_attributes);

    if (stat == NFS3_OK) {
        Write(&access);
    }

    return stat;
}

nfsstat3 CNFS3Prog::ProcedureREAD(void)
{
    char *path;
    offset3 offset;
    count3 count;
    post_op_attr file_attributes;
    bool eof;
    opaque data;
    nfsstat3 stat;
    FILE *pFile;

    PrintLog("READ");
    path = GetPath();
    Read(&offset);
    Read(&count);
    stat = CheckFile(path);

    if (stat == NFS3_OK) {
        data.SetSize(count);

        errno_t errorNumber = fopen_s(&pFile, path, "rb");

        if (pFile != NULL) {
            fseek(pFile, (long)offset, SEEK_SET);
            count = fread(data.contents, sizeof(char), count, pFile);
            eof = fgetc(pFile) == EOF;
            fclose(pFile);
        } else {
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

    file_attributes.attributes_follow = false;

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
    char *path;
    offset3 offset;
    count3 count;
    stable_how stable;
    opaque data;
    wcc_data file_wcc;
    writeverf3 verf;
    nfsstat3 stat;
    FILE *pFile;

    PrintLog("WRITE");
    path = GetPath();
    Read(&offset);
    Read(&count);
    Read(&stable);
    Read(&data);
    stat = CheckFile(path);

    if (stat == NFS3_OK) {       
        errno_t errorNumber = fopen_s(&pFile, path, "r+b");

        if (pFile != NULL) {
            fseek(pFile, (long)offset, SEEK_SET);
            count = fwrite(data.contents, sizeof(char), data.length, pFile);
            fclose(pFile);
        } else {
            char buffer[BUFFER_SIZE];
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
    }

    file_wcc.before.attributes_follow = false;
    file_wcc.after.attributes_follow = false;

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
    path = GetFullPath();
    Read(&how);

    errno_t errorNumber = fopen_s(&pFile, path, "wb");

    if (pFile != NULL) {
        char buffer[BUFFER_SIZE];
        strerror_s(buffer, BUFFER_SIZE, errorNumber);
        PrintLog(buffer);

        if (errorNumber == 13) {
            stat = NFS3ERR_ACCES;
        } else {
            stat = NFS3ERR_IO;
        }

        fclose(pFile);
    }

    stat = pFile != NULL ? NFS3_OK : NFS3ERR_IO;

    if (stat == NFS3_OK) {
        obj.handle_follows = GetFileHandle(path, &obj.handle);
        obj_attributes.attributes_follow = GetFileAttributes(path, &obj_attributes.attributes);
    }

    dir_wcc.before.attributes_follow = false;
    dir_wcc.after.attributes_follow = false;

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
    path = GetFullPath();
    Read(&attributes);
    stat = _mkdir(path) == 0 ? NFS3_OK : NFS3ERR_IO;

    if (stat == NFS3_OK) {
        obj.handle_follows = GetFileHandle(path, &obj.handle);
        obj_attributes.attributes_follow = GetFileAttributes(path, &obj_attributes.attributes);
        dir_wcc.before.attributes_follow = false;
        dir_wcc.after.attributes_follow = GetFileAttributes(path, &dir_wcc.after.attributes);
    } else {
        dir_wcc.before.attributes_follow = false;
        dir_wcc.after.attributes_follow = false;
    }

    Write(&stat);

    if (stat == NFS3_OK) {
        Write(&obj);
        Write(&obj_attributes);
    }

    Write(&dir_wcc);

    return stat;
}

nfsstat3 CNFS3Prog::ProcedureREMOVE(void)
{
    char *path;
    wcc_data dir_wcc;
    nfsstat3 stat;

    PrintLog("REMOVE");
    path = GetFullPath();
    stat = CheckFile(path);

    if (stat == NFS3_OK) {
        if (!RemoveFile(path)) {
            stat = NFS3ERR_IO;
        }
            
    }

    dir_wcc.before.attributes_follow = false;
    dir_wcc.after.attributes_follow = false;

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
    path = GetFullPath();
    stat = CheckFile(path);

    if (stat == NFS3_OK) {
        if (_rmdir(path) != 0) {
            stat = NFS3ERR_IO;
        }         
    }

    dir_wcc.before.attributes_follow = false;
    dir_wcc.after.attributes_follow = false;

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
    strcpy_s(pathFrom, GetFullPath());
    pathTo = GetFullPath();
    stat = CheckFile(pathFrom);

    if (stat == NFS3_OK) {
        if (!RenameFile(pathFrom, pathTo)) {
            stat = NFS3ERR_IO;
        }            
    }

    fromdir_wcc.before.attributes_follow = false;
    fromdir_wcc.after.attributes_follow = false;
    todir_wcc.before.attributes_follow = false;
    todir_wcc.after.attributes_follow = false;

    Write(&stat);
    Write(&fromdir_wcc);
    Write(&todir_wcc);

    return stat;
}

nfsstat3 CNFS3Prog::ProcedureREADDIR(void)
{
    char *path;
    cookie3 cookie;
    cookieverf3 cookieverf;
    count3 count;
    post_op_attr dir_attributes;
    fileid3 fileid;
    filename3 name;
    bool eof;
    nfsstat3 stat;
    char filePath[MAXPATHLEN];
    int handle;
    struct _finddata_t fileinfo;

    PrintLog("READDIR");
    path = GetPath();
    Read(&cookie);
    Read(&cookieverf);
    Read(&count);
    stat = CheckFile(path);

    if (stat == NFS3_OK) {
        dir_attributes.attributes_follow = GetFileAttributes(path, &dir_attributes.attributes);

        if (!dir_attributes.attributes_follow) {
            stat = NFS3ERR_IO;
        }    
    }

    Write(&stat);
    Write(&dir_attributes);

    if (stat == NFS3_OK) {
        Write(&cookieverf);
        sprintf_s(filePath, "%s\\*", path);
        cookie = 0;
        eof = false;
        handle = _findfirst(filePath, &fileinfo);

        if (handle) {
            do {
                if (cookie > 0) {
                    Write(&eof); //eof
                }
                    
                sprintf_s(filePath, "%s\\%s", path, fileinfo.name);
                fileid = GetFileID(filePath);
                Write(&fileid); //file id
                name.Set(fileinfo.name);
                Write(&name); //name
                ++cookie;
                Write(&cookie); //cookie
            } while (_findnext(handle, &fileinfo) == 0);

            _findclose(handle);
        }

        eof = true;
        Write(&eof); //eof
    }

    return stat;
}

nfsstat3 CNFS3Prog::ProcedureREADDIRPLUS(void)
{
    char *path;
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

    PrintLog("READDIRPLUS");
    path = GetPath();
    Read(&cookie);
    Read(&cookieverf);
    Read(&dircount);
    Read(&maxcount);
    stat = CheckFile(path);

    if (stat == NFS3_OK) {
        dir_attributes.attributes_follow = GetFileAttributes(path, &dir_attributes.attributes);
        if (!dir_attributes.attributes_follow)
            stat = NFS3ERR_IO;
    }

    Write(&stat);
    Write(&dir_attributes);

    if (stat == NFS3_OK) {
        Write(&cookieverf);
        sprintf_s(filePath, "%s\\*", path);
        handle = _findfirst(filePath, &fileinfo);
        eof = true;

        if (handle) {
            nFound = 0;

            for (i = (unsigned int)cookie; i > 0; i--) {
                nFound = _findnext(handle, &fileinfo);
            }              

            if (nFound == 0) {
                bFollows = true;
                j = 10;

                do {
                    Write(&bFollows); //value follows
                    sprintf_s(filePath, "%s\\%s", path, fileinfo.name);
                    fileid = GetFileID(filePath);
                    Write(&fileid); //file id
                    name.Set(fileinfo.name);
                    Write(&name); //name
                    ++cookie;
                    Write(&cookie); //cookie
                    name_attributes.attributes_follow = GetFileAttributes(filePath, &name_attributes.attributes);
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

nfsstat3 CNFS3Prog::ProcedureFSINFO(void)
{
    char *path;
    post_op_attr obj_attributes;
    uint32 rtmax, rtpref, rtmult, wtmax, wtpref, wtmult, dtpref;
    size3 maxfilesize;
    nfstime3 time_delta;
    uint32 properties;
    nfsstat3 stat;

    PrintLog("FSINFO");
    path = GetPath();
    stat = CheckFile(path);

    if (stat == NFS3_OK) {
        obj_attributes.attributes_follow = GetFileAttributes(path, &obj_attributes.attributes);

        if (obj_attributes.attributes_follow) {
            rtmax = 32768;
            rtpref = 32768;
            rtmult = 512;
            wtmax = 4096;
            wtpref = 4096;
            wtmult = 512;
            dtpref = 8192;
            maxfilesize = 0x7FFFFFFF;
            time_delta.seconds = 1;
            time_delta.nseconds = 0;
            properties = FSF3_CANSETTIME;
        } else {
            stat = NFS3ERR_IO;
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

char *CNFS3Prog::GetPath(void)
{
    nfs_fh3 object;
    char *path;

    Read(&object);
    path = GetFilePath(object.contents);
    PrintLog(" %s ", path);

    return path;
}

char *CNFS3Prog::GetFullPath(void)
{
    diropargs3 dir;
    char *path;
    static char fullPath[MAXPATHLEN + 1];

    Read(&dir);
    path = GetFilePath(dir.dir.contents);

    if (path == NULL) {
        return NULL;
    }    

    if (strlen(path) + 1 + strlen(dir.name.name) > MAXPATHLEN) {
        return NULL;
    }
        
    sprintf_s(fullPath, "%s\\%s", path, dir.name.name); //concate path and filename
    PrintLog(" %s ", fullPath);

    return fullPath;
}

nfsstat3 CNFS3Prog::CheckFile(char *path)
{
    if (path == NULL) {
        return NFS3ERR_STALE;
    }
        
    if (!FileExists(path)) {
        return NFS3ERR_NOENT;
    }
        
    return NFS3_OK;
}

bool CNFS3Prog::GetFileHandle(char *path, nfs_fh3 *pObject)
{
    memcpy(pObject->contents, ::GetFileHandle(path), pObject->length);

    return true;
}

bool CNFS3Prog::GetFileAttributes(char *path, fattr3 *pAttr)
{
    struct stat data;

    if (stat(path, &data) != 0) {
        return false;
    }
        
    switch (data.st_mode & S_IFMT) {
        case S_IFREG:
            pAttr->type = NF3REG;
            break;
        case S_IFDIR:
            pAttr->type = NF3DIR;
            break;
        case S_IFCHR:
            pAttr->type = NF3CHR;
            break;
        default:
            pAttr->type = 0;
            break;
    }

    pAttr->mode = 0;
    
    if ((data.st_mode & S_IREAD) != 0) {
        pAttr->mode |= 0x124;
    }
        
    if ((data.st_mode & S_IWRITE) != 0) {
        pAttr->mode |= 0x92;
    }
        
    if ((data.st_mode & S_IEXEC) != 0) {
        pAttr->mode |= 0x49;
    }        

    pAttr->nlink = data.st_nlink;
    pAttr->uid = m_nUID;
    pAttr->gid = m_nGID;
    pAttr->size = data.st_size;
    pAttr->used = pAttr->size;
    pAttr->rdev.specdata1 = 0;
    pAttr->rdev.specdata2 = 0;
    pAttr->fsid = 4;
    pAttr->fileid = GetFileID(path);
    pAttr->atime.seconds = data.st_atime;
    pAttr->atime.nseconds = 0;
    pAttr->mtime.seconds = data.st_mtime;
    pAttr->mtime.nseconds = 0;
    pAttr->ctime.seconds = data.st_ctime;
    pAttr->ctime.nseconds = 0;

    return true;
}
