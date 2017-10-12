#ifndef _NFS3PROG_H_
#define _NFS3PROG_H_

#include "RPCProg.h"
#include <string>
#include <windows.h>
#include <unordered_map>

typedef unsigned __int64 uint64;
typedef unsigned long uint32;
typedef long int32;
typedef uint64 fileid3;
typedef uint64 cookie3;
typedef uint32 uid3;
typedef uint32 gid3;
typedef uint64 size3;
typedef uint64 offset3;
typedef uint32 mode3;
typedef uint32 count3;
typedef uint32 nfsstat3;
typedef uint32 ftype3;
typedef uint32 stable_how;
typedef uint32 time_how;
typedef uint32 createmode3;
typedef uint64 cookieverf3;
typedef uint64 createverf3;
typedef uint64 writeverf3;

class opaque
{
    public:
    uint32 length;
    unsigned char *contents;

    opaque();
    opaque(uint32 len);
    virtual ~opaque();
    virtual void SetSize(uint32 len);
};

class nfs_fh3 : public opaque
{
    public:
    nfs_fh3();
    ~nfs_fh3();
};

class filename3 : public opaque
{
    public:
    char *name;

    filename3();
    ~filename3();
    void SetSize(uint32 len);
    void Set(char *str);
};

class nfspath3 : public opaque
{
	public:
	char *path;

	nfspath3();
	~nfspath3();
	void SetSize(uint32 len);
    void Set(char *str);
};

typedef struct
{
    uint32 specdata1;
    uint32 specdata2;
} specdata3;

typedef struct
{
    uint32 seconds;
    uint32 nseconds;
} nfstime3;

typedef struct
{
    bool check;
    nfstime3 obj_ctime;
} sattrguard3;

typedef struct
{
    ftype3 type;
    mode3 mode;
    uint32 nlink;
    uid3 uid;
    gid3 gid;
    size3 size;
    size3 used;
    specdata3 rdev;
    uint64 fsid;
    fileid3 fileid;
    nfstime3 atime;
    nfstime3 mtime;
    nfstime3 ctime;
} fattr3;

typedef struct
{
    bool attributes_follow;
    fattr3 attributes;
} post_op_attr;

typedef struct
{
    size3 size;
    nfstime3 mtime;
    nfstime3 ctime;
} wcc_attr;

typedef struct
{
    bool attributes_follow;
    wcc_attr attributes;
} pre_op_attr;

typedef struct
{
    pre_op_attr before;
    post_op_attr after;
} wcc_data;

typedef struct
{
    bool handle_follows;
    nfs_fh3 handle;
} post_op_fh3;

typedef struct
{
    bool set_it;
    mode3 mode;
} set_mode3;

typedef struct
{
    bool set_it;
    uid3 uid;
} set_uid3;

typedef struct
{
    bool set_it;
    gid3 gid;
} set_gid3;

typedef struct
{
    bool set_it;
    size3 size;
} set_size3;

typedef struct
{
    time_how set_it;
    nfstime3 atime;
} set_atime;

typedef struct
{
    time_how set_it;
    nfstime3 mtime;
} set_mtime;

typedef struct
{
    set_mode3 mode;
    set_uid3 uid;
    set_gid3 gid;
    set_size3 size;
    set_atime atime;
    set_mtime mtime;
} sattr3;

typedef struct
{
    nfs_fh3 dir;
    filename3 name;
} diropargs3;

typedef struct
{
    createmode3 mode;
    sattr3 obj_attributes;
    createverf3 verf;
} createhow3;

typedef struct 
{
	sattr3 symlink_attributes;
	nfspath3 symlink_data;
} symlinkdata3;

typedef struct _REPARSE_DATA_BUFFER {
	ULONG  ReparseTag;
	USHORT ReparseDataLength;
	USHORT Reserved;
	union {
		struct {
			USHORT SubstituteNameOffset;
			USHORT SubstituteNameLength;
			USHORT PrintNameOffset;
			USHORT PrintNameLength;
			ULONG  Flags;
			WCHAR  PathBuffer[1];
		} SymbolicLinkReparseBuffer;
		struct {
			USHORT SubstituteNameOffset;
			USHORT SubstituteNameLength;
			USHORT PrintNameOffset;
			USHORT PrintNameLength;
			WCHAR  PathBuffer[1];
		} MountPointReparseBuffer;
		struct {
			UCHAR DataBuffer[1];
		} GenericReparseBuffer;
	};
} REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

class CNFS3Prog : public CRPCProg
{
    public:
    CNFS3Prog();
    ~CNFS3Prog();
    void SetUserID(unsigned int nUID, unsigned int nGID);
    int Process(IInputStream *pInStream, IOutputStream *pOutStream, ProcessParam *pParam);

    protected:
    unsigned long m_nUID, m_nGID;
    IInputStream *m_pInStream;
    IOutputStream *m_pOutStream;
    ProcessParam *m_pParam;

    nfsstat3 ProcedureNULL(void);
    nfsstat3 ProcedureGETATTR(void);
    nfsstat3 ProcedureSETATTR(void);
    nfsstat3 ProcedureLOOKUP(void);
    nfsstat3 ProcedureACCESS(void);
    nfsstat3 ProcedureREADLINK(void);
    nfsstat3 ProcedureREAD(void);
    nfsstat3 ProcedureWRITE(void);
    nfsstat3 ProcedureCREATE(void);
    nfsstat3 ProcedureMKDIR(void);
    nfsstat3 ProcedureSYMLINK(void);
    nfsstat3 ProcedureMKNOD(void);
    nfsstat3 ProcedureREMOVE(void);
    nfsstat3 ProcedureRMDIR(void);
    nfsstat3 ProcedureRENAME(void);
    nfsstat3 ProcedureLINK(void);
    nfsstat3 ProcedureREADDIR(void);
    nfsstat3 ProcedureREADDIRPLUS(void);
    nfsstat3 ProcedureFSSTAT(void);
    nfsstat3 ProcedureFSINFO(void);
    nfsstat3 ProcedurePATHCONF(void);
    nfsstat3 ProcedureCOMMIT(void);
    nfsstat3 ProcedureNOIMP(void);

    void Read(bool *pBool);
    void Read(uint32 *pUint32);
    void Read(uint64 *pUint64);
    void Read(sattr3 *pAttr);
    void Read(sattrguard3 *pGuard);
    void Read(diropargs3 *pDir);
    void Read(opaque *pOpaque);
    void Read(nfstime3 *pTime);
    void Read(createhow3 *pHow);
	void Read(symlinkdata3 *pSymlink);
    void Write(bool *pBool);
    void Write(uint32 *pUint32);
    void Write(uint64 *pUint64);
    void Write(fattr3 *pAttr);
    void Write(opaque *pOpaque);
    void Write(wcc_data *pWcc);
    void Write(post_op_attr *pAttr);
    void Write(pre_op_attr *pAttr);
    void Write(post_op_fh3 *pObj);
    void Write(nfstime3 *pTime);
    void Write(specdata3 *pSpec);
    void Write(wcc_attr *pAttr);

    private:
    int m_nResult;

    bool GetPath(std::string &path);
    bool ReadDirectory(std::string &dirName, std::string &fileName);
    char *GetFullPath(std::string &dirName, std::string &fileName);
    nfsstat3 CheckFile(const char *fullPath);
    nfsstat3 CheckFile(const char *directory, const char *fullPath);
    bool GetFileHandle(const char *path, nfs_fh3 *pObject);
    bool GetFileAttributesForNFS(const char *path, wcc_attr *pAttr);
    bool GetFileAttributesForNFS(const char *path, fattr3 *pAttr);
    UINT32 FileTimeToPOSIX(FILETIME ft);
    std::unordered_map<int, FILE*> unstableStorageFile;
};

#endif
