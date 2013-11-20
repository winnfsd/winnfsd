#include "MountProg.h"
#include "FileTable.h"
#include <string.h>

enum
{
    MOUNTPROC_NULL = 0,
    MOUNTPROC_MNT = 1,
    MOUNTPROC_DUMP = 2,
    MOUNTPROC_UMNT = 3,
    MOUNTPROC_UMNTALL = 4,
    MOUNTPROC_EXPORT = 5
};

enum
{
    MNT_OK = 0,
    MNTERR_PERM = 1,
    MNTERR_NOENT = 2,
    MNTERR_IO = 5,
    MNTERR_ACCESS = 13,
    MNTERR_NOTDIR = 20,
    MNTERR_INVAL = 22
};

typedef void (CMountProg::*PPROC)(void);

CMountProg::CMountProg() : CRPCProg()
{
    m_pExportPath[0] = '\0';
    m_pPathAlias[0] = '\0';
    m_nMountNum = 0;
    memset(m_pClientAddr, 0, sizeof(m_pClientAddr));
}

CMountProg::~CMountProg()
{
    int i;

    for (i = 0; i < MOUNT_NUM_MAX; i++) {
        delete[] m_pClientAddr[i];
    }

}

void CMountProg::Export(char *path, char *pathAlias)
{
    strncpy_s(m_pExportPath, path, sizeof(m_pExportPath)-1);
    m_pExportPath[sizeof(m_pExportPath)-1] = '\0';
    strncpy_s(m_pPathAlias, pathAlias, sizeof(m_pPathAlias)-1);
    m_pPathAlias[sizeof(m_pPathAlias)-1] = '\0';
}

int CMountProg::GetMountNumber(void)
{
    return m_nMountNum;  //the number of clients mounted
}

char *CMountProg::GetClientAddr(int nIndex)
{
    int i;

    if (nIndex < 0 || nIndex >= m_nMountNum) {
        return NULL;
    }

    for (i = 0; i < MOUNT_NUM_MAX; i++) {
        if (m_pClientAddr[i] != NULL) {
            if (nIndex == 0) {
                return m_pClientAddr[i];  //client address
            } else {
                --nIndex;
            }
        }

    }
    return NULL;
}

int CMountProg::Process(IInputStream *pInStream, IOutputStream *pOutStream, ProcessParam *pParam)
{
    static PPROC pf[] = { &CMountProg::ProcedureNULL, &CMountProg::ProcedureMNT, &CMountProg::ProcedureNOIMP, &CMountProg::ProcedureUMNT };

    PrintLog("MOUNT ");

    if (pParam->nProc >= sizeof(pf) / sizeof(PPROC)) {
        ProcedureNOIMP();
        PrintLog("\n");
        return PRC_NOTIMP;
    }

    m_pInStream = pInStream;
    m_pOutStream = pOutStream;
    m_pParam = pParam;
    m_nResult = PRC_OK;
    (this->*pf[pParam->nProc])();
    PrintLog("\n");

    return m_nResult;
}

void CMountProg::ProcedureNULL(void)
{
    PrintLog("NULL");
}

void CMountProg::ProcedureMNT(void)
{
    char *path;
    int i;

    PrintLog("MNT");
    path = GetPath();
    PrintLog(" from %s", m_pParam->pRemoteAddr);

    if (m_nMountNum < MOUNT_NUM_MAX && _stricmp(path, m_pExportPath) == 0) { //path match
        m_pOutStream->Write(MNT_OK); //OK

        if (m_pParam->nVersion == 1) {
            m_pOutStream->Write(GetFileHandle(path), FHSIZE);  //fhandle
        } else {
            m_pOutStream->Write(NFS3_FHSIZE);  //length
            m_pOutStream->Write(GetFileHandle(path), NFS3_FHSIZE);  //fhandle
            m_pOutStream->Write(0);  //flavor
        }

        ++m_nMountNum;

        for (i = 0; i < MOUNT_NUM_MAX; i++) {
            if (m_pClientAddr[i] == NULL) { //search an empty space
                m_pClientAddr[i] = new char[strlen(m_pParam->pRemoteAddr) + 1];
                strcpy_s(m_pClientAddr[i], (strlen(m_pParam->pRemoteAddr) + 1), m_pParam->pRemoteAddr);  //remember the client address
                break;
            }
        }
    } else {
        m_pOutStream->Write(MNTERR_ACCESS);  //permission denied
    }

}

void CMountProg::ProcedureUMNT(void)
{
    char *path;
    int i;

    PrintLog("UMNT");
    path = GetPath();
    PrintLog(" from %s", m_pParam->pRemoteAddr);

    for (i = 0; i < MOUNT_NUM_MAX; i++) {
        if (m_pClientAddr[i] != NULL) {
            if (strcmp(m_pParam->pRemoteAddr, m_pClientAddr[i]) == 0) { //address match
                delete[] m_pClientAddr[i];  //remove this address
                m_pClientAddr[i] = NULL;
                --m_nMountNum;
                break;
            }
        }
    }
}

void CMountProg::ProcedureNOIMP(void)
{
    PrintLog("NOIMP");
    m_nResult = PRC_NOTIMP;
}

char *CMountProg::GetPath(void)
{
    unsigned long i, nSize;
    static char path[MAXPATHLEN + 1];
    static char finalPath[MAXPATHLEN + 1];

    m_pInStream->Read(&nSize);

    if (nSize > MAXPATHLEN) {
        nSize = MAXPATHLEN;
    }

    m_pInStream->Read(path, nSize);

    //We have the requested path, the local path and its alias
    //Let's cache the various string sizes
    size_t windowsPathSize = strlen(m_pExportPath);
    size_t aliasPathSize = strlen(m_pPathAlias);
    size_t requestedPathSize = nSize;

    if ((requestedPathSize < windowsPathSize) && (strncmp(path, m_pPathAlias, aliasPathSize) == 0)) {
        //The requested path starts with the alias. Let's replace the alias with the real path
        strncpy_s(finalPath, m_pExportPath, sizeof(finalPath));
        //strncpy_s(finalPath + windowsPathSize, (path + aliasPathSize), (sizeof(finalPath)-windowsPathSize));
        finalPath[windowsPathSize + requestedPathSize - aliasPathSize] = '\0';

        for (i = 0; i < requestedPathSize; i++) { //transform path to Windows format
            if (finalPath[windowsPathSize + i] == '/') {
                finalPath[windowsPathSize + i] = '\\';
            }
        }
    } else if ((strlen(path) == strlen(m_pPathAlias)) && (strncmp(path, m_pPathAlias, strlen(m_pPathAlias)) == 0)) {
        //The requested path IS the alias
        strncpy_s(finalPath, m_pExportPath, sizeof(finalPath));
        finalPath[windowsPathSize] = '\0';
    } else {
        //The requested path does not start with the alias, let's treat it normally
        strncpy_s(finalPath, path, sizeof(finalPath));
        finalPath[0] = finalPath[1];  //transform mount path to Windows format
        finalPath[1] = ':';

        for (i = 2; i < nSize; i++) {
            if (finalPath[i] == '/') {
                finalPath[i] = '\\';
            }
        }

        finalPath[nSize] = '\0';
    }

    PrintLog("Final local requested path: %s\n", finalPath);

    if ((nSize & 3) != 0) {
        m_pInStream->Read(&i, 4 - (nSize & 3));  //skip opaque bytes
    }


    return finalPath;
}
