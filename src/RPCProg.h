#ifndef _RPCPROG_H_
#define _RPCPROG_H_

#include "InputStream.h"
#include "OutputStream.h"

/* The maximum number of bytes in a pathname argument. */
#define MAXPATHLEN 1024

/* The maximum number of bytes in a file name argument. */
#define MAXNAMELEN 255

/* The size in bytes of the opaque file handle. */
#define FHSIZE 32
#define NFS3_FHSIZE 64

enum
{
    PRC_OK,
    PRC_FAIL,
    PRC_NOTIMP
};

typedef struct
{
    unsigned int nVersion;
    unsigned int nProc;
    char *pRemoteAddr;
} ProcessParam;

class CRPCProg
{
    public:
    CRPCProg();
    virtual ~CRPCProg();
    virtual int Process(IInputStream *pInStream, IOutputStream *pOutStream, ProcessParam *pParam) = 0;
    virtual void SetLogOn(bool bLogOn);

    protected:
    bool m_bLogOn;
    virtual int PrintLog(const char *format, ...);
};

#endif
