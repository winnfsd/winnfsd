#include "NFSProg.h"
#include "FileTable.h"
#include <string.h>
#include <io.h>
#include <direct.h>
#include <sys/stat.h>
#include <windows.h>
#include <share.h>

enum
{
    NFSPROC_NULL = 0,
    NFSPROC_GETATTR = 1,
    NFSPROC_SETATTR = 2,
    NFSPROC_ROOT = 3,
    NFSPROC_LOOKUP = 4,
    NFSPROC_READLINK = 5,
    NFSPROC_READ = 6,
    NFSPROC_WRITECACHE = 7,
    NFSPROC_WRITE = 8,
    NFSPROC_CREATE = 9,
    NFSPROC_REMOVE = 10,
    NFSPROC_RENAME = 11,
    NFSPROC_LINK = 12,
    NFSPROC_SYMLINK = 13,
    NFSPROC_MKDIR = 14,
    NFSPROC_RMDIR = 15,
    NFSPROC_READDIR = 16,
    NFSPROC_STATFS = 17
};

enum
{
    NFS_OK = 0,
    NFSERR_PERM = 1,
    NFSERR_NOENT = 2,
    NFSERR_IO = 5,
    NFSERR_NXIO = 6,
    NFSERR_ACCES = 13,
    NFSERR_EXIST = 17,
    NFSERR_NODEV = 19,
    NFSERR_NOTDIR = 20,
    NFSERR_ISDIR = 21,
    NFSERR_FBIG = 27,
    NFSERR_NOSPC = 28,
    NFSERR_ROFS = 30,
    NFSERR_NAMETOOLONG = 63,
    NFSERR_NOTEMPTY = 66,
    NFSERR_DQUOT = 69,
    NFSERR_STALE = 70,
    NFSERR_WFLUSH = 99,
};

enum
{
    NFNON = 0,
    NFREG = 1,
    NFDIR = 2,
    NFBLK = 3,
    NFCHR = 4,
    NFLNK = 5,
};

typedef void(CNFS2Prog::*PPROC)(void);

CNFS2Prog::CNFS2Prog() : CRPCProg()
{
    m_nUID = m_nGID = 0;
}

CNFS2Prog::~CNFS2Prog()
{
}

void CNFS2Prog::SetUserID(unsigned int nUID, unsigned int nGID)
{
    m_nUID = nUID;
    m_nGID = nGID;
}

int CNFS2Prog::Process(IInputStream *pInStream, IOutputStream *pOutStream, ProcessParam *pParam)
{
    static PPROC pf[] = { &CNFS2Prog::ProcedureNULL, &CNFS2Prog::ProcedureGETATTR, &CNFS2Prog::ProcedureSETATTR, &CNFS2Prog::ProcedureNOTIMP, &CNFS2Prog::ProcedureLOOKUP, &CNFS2Prog::ProcedureNOTIMP, &CNFS2Prog::ProcedureREAD, &CNFS2Prog::ProcedureNOTIMP, &CNFS2Prog::ProcedureWRITE, &CNFS2Prog::ProcedureCREATE, &CNFS2Prog::ProcedureREMOVE, &CNFS2Prog::ProcedureRENAME, &CNFS2Prog::ProcedureNOTIMP, &CNFS2Prog::ProcedureNOTIMP, &CNFS2Prog::ProcedureMKDIR, &CNFS2Prog::ProcedureRMDIR, &CNFS2Prog::ProcedureREADDIR, &CNFS2Prog::ProcedureSTATFS };

    PrintLog("NFS ");

    if (pParam->nProc >= sizeof(pf) / sizeof(PPROC)) {
        ProcedureNOTIMP();
        PrintLog("\n");
        return PRC_NOTIMP;
    }

    m_pInStream = pInStream;
    m_pOutStream = pOutStream;
    m_nResult = PRC_OK;

    (this->*pf[pParam->nProc])();

    PrintLog("\n");

    return m_nResult;
}

void CNFS2Prog::ProcedureNULL(void)
{
    PrintLog("NULL");
}

void CNFS2Prog::ProcedureGETATTR(void)
{
    char *path;

    PrintLog("GETATTR");
    path = GetPath();
    
    if (!CheckFile(path)) {
        return;
    }

    m_pOutStream->Write(NFS_OK);
    WriteFileAttributes(path);
}

void CNFS2Prog::ProcedureSETATTR(void)
{
    char *path;
    unsigned long nMode, nAttr;

    PrintLog("SETATTR");
    path = GetPath();
    if (!CheckFile(path))
        return;

    m_pInStream->Read(&nMode);
    nAttr = 0;

    if ((nMode & 0x100) != 0) {
        nAttr |= S_IREAD;
    }
        
    if ((nMode & 0x80) != 0) {
        nAttr |= S_IWRITE;
    }
        
    _chmod(path, nAttr);
    m_pOutStream->Write(NFS_OK);
    WriteFileAttributes(path);
}

void CNFS2Prog::ProcedureLOOKUP(void)
{
    char *path;

    PrintLog("LOOKUP");
    path = GetFullPath();

    if (!CheckFile(path)) {
        return;
    }

    m_pOutStream->Write(NFS_OK);
    m_pOutStream->Write(GetFileHandle(path), FHSIZE);
    WriteFileAttributes(path);
}

void CNFS2Prog::ProcedureREAD(void)
{
    char *path;
    unsigned long nOffset, nCount, nTotalCount;
    FILE *file;
    char *pBuffer;
    unsigned char opaque[3] = { 0, 0, 0 };

    PrintLog("READ");
    path = GetPath();

    if (!CheckFile(path)) {
        return;
    }

    m_pInStream->Read(&nOffset);
    m_pInStream->Read(&nCount);
    m_pInStream->Read(&nTotalCount);

    file = _fsopen(path, "rb", _SH_DENYWR);

    if (file != NULL) {
        fseek(file, nOffset, SEEK_SET);
        pBuffer = new char[nCount];
        nCount = fread(pBuffer, sizeof(char), nCount, file);
        fclose(file);
    } else {
        return;
    }

    m_pOutStream->Write(NFS_OK);
    WriteFileAttributes(path);
    m_pOutStream->Write(nCount); //length
    m_pOutStream->Write(pBuffer, nCount); //contents
    nCount &= 3;

    if (nCount != 0) {
        m_pOutStream->Write(opaque, 4 - nCount); //opaque bytes
    }
        
    delete[] pBuffer;
}

void CNFS2Prog::ProcedureWRITE(void)
{
    char *path;
    unsigned long nBeginOffset, nOffset, nTotalCount, nCount;
    FILE *file;
    char *pBuffer;

    PrintLog("WRITE");
    path = GetPath();

    if (!CheckFile(path)) {
        return;
    }

    m_pInStream->Read(&nBeginOffset);
    m_pInStream->Read(&nOffset);
    m_pInStream->Read(&nTotalCount);
    m_pInStream->Read(&nCount);
    pBuffer = new char[nCount];
    m_pInStream->Read(pBuffer, nCount);

    file = _fsopen(path, "r+b", _SH_DENYWR);

    if (file != NULL) {
        fseek(file, nOffset, SEEK_SET);
        nCount = fwrite(pBuffer, sizeof(char), nCount, file);
        fclose(file);
    } else {
        return;
    }
    
    delete[] pBuffer;

    m_pOutStream->Write(NFS_OK);
    WriteFileAttributes(path);
}

void CNFS2Prog::ProcedureCREATE(void)
{
    char *path;
    FILE *file;

    PrintLog("CREATE");
    path = GetFullPath();

    if (path == NULL) {
        return;
    }

    file = _fsopen(path, "wb", _SH_DENYWR);

    if (file != NULL) {
        fclose(file);
    } else {
        return;
    }

    m_pOutStream->Write(NFS_OK);
    m_pOutStream->Write(GetFileHandle(path), FHSIZE);
    WriteFileAttributes(path);
}

void CNFS2Prog::ProcedureREMOVE(void)
{
    char *path;

    PrintLog("REMOVE");
    path = GetFullPath();

    if (!CheckFile(path)) {
        return;
    }

    remove(path);
    m_pOutStream->Write(NFS_OK);
}

void CNFS2Prog::ProcedureRENAME(void)
{
    char *path;
    char pathFrom[MAXPATHLEN], *pathTo;

    PrintLog("RENAME");
    path = GetFullPath();

    if (!CheckFile(path)) {
        return;
    }

    strcpy_s(pathFrom, path);
    pathTo = GetFullPath();

    RenameFile(pathFrom, pathTo);
    m_pOutStream->Write(NFS_OK);
}

void CNFS2Prog::ProcedureMKDIR(void)
{
    char *path;

    PrintLog("MKDIR");
    path = GetFullPath();

    if (path == NULL) {
        return;
    }

    _mkdir(path);
    m_pOutStream->Write(NFS_OK);
    m_pOutStream->Write(GetFileHandle(path), FHSIZE);
    WriteFileAttributes(path);
}

void CNFS2Prog::ProcedureRMDIR(void)
{
    char *path;

    PrintLog("RMDIR");
    path = GetFullPath();

    if (!CheckFile(path)) {
        return;
    }

    _rmdir(path);
    m_pOutStream->Write(NFS_OK);
}

void CNFS2Prog::ProcedureREADDIR(void)
{
    unsigned char opaque[3] = { 0, 0, 0 };
    char *path, filePath[MAXPATHLEN + 1];
    int handle;
    struct _finddata_t fileinfo;
    unsigned long count;
    unsigned int nLen;

    PrintLog("READDIR");
    path = GetPath();

    if (!CheckFile(path)) {
        return;
    }

    m_pOutStream->Write(NFS_OK);
    sprintf_s(filePath, "%s\\*", path);
    count = 0;
    handle = _findfirst(filePath, &fileinfo);

    if (handle) {
        do {
            m_pOutStream->Write(1); //value follows
            sprintf_s(filePath, "%s\\%s", path, fileinfo.name);
            m_pOutStream->Write(GetFileID(filePath)); //file id
            m_pOutStream->Write(nLen = strlen(fileinfo.name));
            m_pOutStream->Write(fileinfo.name, nLen);
            nLen &= 3;
            if (nLen != 0) {
                m_pOutStream->Write(opaque, 4 - nLen); //opaque bytes
            }
            
            m_pOutStream->Write(++count); //cookie
        } while (_findnext(handle, &fileinfo) == 0);

        _findclose(handle);
    }

    m_pOutStream->Write(0); //no value follows
    m_pOutStream->Write(1); //EOF
}

void CNFS2Prog::ProcedureSTATFS(void)
{
    char *path;
    int nDrive;
    struct _diskfree_t data;

    PrintLog("STATFS");
    path = GetPath();

    if (!CheckFile(path)) {
        return;
    }

    if (path[0] >= 'a' && path[0] <= 'z') {
        nDrive = path[0] - 'a' + 1;
    } else if (path[0] >= 'A' && path[0] <= 'Z') {
        nDrive = path[0] - 'A' + 1;
    } else {
        m_pOutStream->Write(NFSERR_NOENT);

        return;
    }

    _getdiskfree(nDrive, &data);
    m_pOutStream->Write(NFS_OK);
    m_pOutStream->Write(data.sectors_per_cluster * data.bytes_per_sector); //transfer size
    m_pOutStream->Write(data.sectors_per_cluster * data.bytes_per_sector); //block size
    m_pOutStream->Write(data.total_clusters); //total blocks
    m_pOutStream->Write(data.avail_clusters); //free blocks
    m_pOutStream->Write(data.avail_clusters); //available blocks
}

void CNFS2Prog::ProcedureNOTIMP(void)
{
    PrintLog("NOTIMP");
    m_nResult = PRC_NOTIMP;
}

char *CNFS2Prog::GetPath(void)
{
    unsigned char fhandle[FHSIZE];
    char *path;

    m_pInStream->Read(fhandle, FHSIZE);
    path = GetFilePath(fhandle);

    if (path == NULL) {
        return NULL;
    }
        
    PrintLog(" %s", path);
    return path;
}

char *CNFS2Prog::GetFullPath(void)
{
    char *path;
    static char filePath[MAXPATHLEN + 1];
    unsigned int nLen1, nBytes;
    unsigned long nLen2;

    path = GetPath();

    if (path == NULL) {
        return NULL;
    }        

    nLen1 = strlen(path);
    m_pInStream->Read(&nLen2);
    sprintf_s(filePath, "%s\\", path);
    m_pInStream->Read(filePath + nLen1 + 1, nLen2);
    filePath[nLen1 + 1 + nLen2] = '\0';
    PrintLog("%s", filePath + nLen1);

    if ((nLen2 & 3) != 0) {
        m_pInStream->Read(&nBytes, 4 - (nLen2 & 3));
    }
        
    return filePath;
}

bool CNFS2Prog::CheckFile(char *path)
{
    if (path == NULL) {
        m_pOutStream->Write(NFSERR_STALE);
        
        return false;
    }

    if (!FileExists(path)) {
        m_pOutStream->Write(NFSERR_NOENT);

        return false;
    }

    return true;
}

bool CNFS2Prog::WriteFileAttributes(char *path)
{
    struct stat data;
    unsigned long nValue;

    if (stat(path, &data) != 0) {
        return false;
    }      

    switch (data.st_mode & S_IFMT) {
        case S_IFREG:
            nValue = NFREG;
            break;
        case S_IFDIR:
            nValue = NFDIR;
            break;
        case S_IFCHR:
            nValue = NFCHR;
            break;
        default:
            nValue = NFNON;
            break;
    }

    m_pOutStream->Write(nValue); //type

    if (nValue == NFREG) {
        nValue = 0x8000;
    } else if (nValue == NFDIR) {
        nValue = 0x4000;
    } else {
        nValue = 0;
    }     

    if ((data.st_mode & S_IREAD) != 0) {
        nValue |= 0x124;
    }

    if ((data.st_mode & S_IWRITE) != 0) {
        nValue |= 0x92;
    }
        
    if ((data.st_mode & S_IEXEC) != 0) {
        nValue |= 0x49;
    }
        
    m_pOutStream->Write(nValue); //mode
    m_pOutStream->Write(data.st_nlink); //nlink	
    m_pOutStream->Write(m_nUID); //uid
    m_pOutStream->Write(m_nGID); //gid
    m_pOutStream->Write(data.st_size); //size
    m_pOutStream->Write(8192); //blocksize
    m_pOutStream->Write(0); //rdev
    m_pOutStream->Write((data.st_size + 8191) / 8192); //blocks
    m_pOutStream->Write(4); //fsid
    m_pOutStream->Write(GetFileID(path)); //fileid
    m_pOutStream->Write(data.st_atime); //atime
    m_pOutStream->Write(0); //atime
    m_pOutStream->Write(data.st_mtime); //mtime
    m_pOutStream->Write(0); //mtime
    m_pOutStream->Write(data.st_ctime); //ctime
    m_pOutStream->Write(0); //ctime

    return true;
}
