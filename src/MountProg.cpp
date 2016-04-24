#include "MountProg.h"
#include "FileTable.h"
#include <string.h>
#include <map>
#include <fstream>
#include <iostream>
#include <string>
#include <direct.h>

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
    m_nMountNum = 0;
    memset(m_pClientAddr, 0, sizeof(m_pClientAddr));
}

CMountProg::~CMountProg()
{
    int i;
	m_pPathFile = NULL;

    for (i = 0; i < MOUNT_NUM_MAX; i++) {
        delete[] m_pClientAddr[i];
    }

}

bool CMountProg::SetPathFile(char *file)
{
	std::ifstream pathFile(file);

	if (pathFile.good()) {
		pathFile.close();
		m_pPathFile = file;
		return true;
	}

	pathFile.close();
	return false;
}

void CMountProg::Export(char *path, char *pathAlias)
{
	path = FormatPath(path, FORMAT_PATH);
	pathAlias = FormatPath(pathAlias, FORMAT_PATHALIAS);

	if (path != NULL && pathAlias != NULL) {
		if (m_PathMap.count(pathAlias) == 0) {
			m_PathMap[pathAlias] = path;
			printf("Path #%i is: %s, path alias is: %s\n", m_PathMap.size(), path, pathAlias);
		} else {
			printf("Path %s with path alias  %s already known\n", path, pathAlias);
		}
	}

}

bool CMountProg::Refresh()
{
	if (m_pPathFile != NULL) {
		ReadPathsFromFile(m_pPathFile);
		return true;
	}

	return false;
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
	Refresh();
    char *path = new char[MAXPATHLEN + 1];
	int i;

	PrintLog("MNT");
	PrintLog(" from %s\n", m_pParam->pRemoteAddr);

	if (GetPath(&path)) {
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
	char *path = new char[MAXPATHLEN + 1];
    int i;

    PrintLog("UMNT");
    GetPath(&path);
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

bool CMountProg::GetPath(char **returnPath)
{
	unsigned long i, nSize;
	static char path[MAXPATHLEN + 1];
	static char finalPath[MAXPATHLEN + 1];
	bool foundPath = false;

	m_pInStream->Read(&nSize);

	if (nSize > MAXPATHLEN) {
		nSize = MAXPATHLEN;
	}

	typedef std::map<std::string, std::string>::iterator it_type;
	m_pInStream->Read(path, nSize);

	for (it_type iterator = m_PathMap.begin(); iterator != m_PathMap.end(); iterator++) {
		char* pathAlias = const_cast<char*>(iterator->first.c_str());
		char* windowsPath = const_cast<char*>(iterator->second.c_str());

		size_t aliasPathSize = strlen(pathAlias);
		size_t windowsPathSize = strlen(windowsPath);
		size_t requestedPathSize = nSize;

		if ((requestedPathSize > aliasPathSize) && (strncmp(path, pathAlias, aliasPathSize) == 0)) {
			foundPath = true;
			//The requested path starts with the alias. Let's replace the alias with the real path
			strncpy_s(finalPath, MAXPATHLEN, windowsPath, windowsPathSize);
			strncpy_s(finalPath + windowsPathSize, MAXPATHLEN - windowsPathSize, (path + aliasPathSize), requestedPathSize - aliasPathSize);
			finalPath[windowsPathSize + requestedPathSize - aliasPathSize] = '\0';

			for (i = 0; i < requestedPathSize - aliasPathSize; i++) {
				//transform path to Windows format
				if (finalPath[windowsPathSize + i] == '/') {
					finalPath[windowsPathSize + i] = '\\';
				}
			}
		} else if ((requestedPathSize == aliasPathSize) && (strncmp(path, pathAlias, aliasPathSize) == 0)) {
			foundPath = true;
			//The requested path IS the alias
			strncpy_s(finalPath, MAXPATHLEN, windowsPath, windowsPathSize);
			finalPath[windowsPathSize] = '\0';
		}

		if (foundPath == true) {
			break;
		}
	}

	if (foundPath != true) {
		//The requested path does not start with the alias, let's treat it normally.
		strncpy_s(finalPath, MAXPATHLEN, path, nSize);
		//transform mount path to Windows format. /d/work => d:\work
		finalPath[0] = finalPath[1];
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

	*returnPath = finalPath;
	return foundPath;
}


bool CMountProg::ReadPathsFromFile(char* sFileName)
{
	sFileName = FormatPath(sFileName, FORMAT_PATH);
	std::ifstream pathFile(sFileName);

	if (pathFile.is_open()) {
		std::string line;

		while (std::getline(pathFile, line)) {
			char *pCurPath = (char*)malloc(line.size() + 1);
			pCurPath = (char*)line.c_str();

			if (pCurPath != NULL) {
				char curPathAlias[MAXPATHLEN];
				strcpy_s(curPathAlias, pCurPath);
				char *pCurPathAlias = (char*)malloc(strlen(curPathAlias));
				pCurPathAlias = curPathAlias;

				Export(pCurPath, pCurPathAlias);
			}
		}
	} else {
		printf("Can't open file %s.\n", sFileName);
		return false;
	}

	return true;
}

char *CMountProg::FormatPath(char *pPath, pathFormats format)
{
	//Remove head spaces
	while (*pPath == ' ') {
		++pPath;
	}

	//Remove tail spaces
	while (*(pPath + strlen(pPath) - 1) == ' ') {
		*(pPath + strlen(pPath) - 1) = '\0';
	}

	//Is comment?
	if (*pPath == '#') {
		return NULL;
	}

	//Remove head "
	if (*pPath == '"') {
		++pPath;
	}

	//Remove tail "
	if (*(pPath + strlen(pPath) - 1) == '"') {
		*(pPath + strlen(pPath) - 1) = '\0';
	}

	//Check for right path format
	if (format == FORMAT_PATH) {
		if (pPath[0] == '.') {
			static char path1[MAXPATHLEN];
			_getcwd(path1, MAXPATHLEN);

			if (pPath[1] == '\0') {
				pPath = path1;
			} else if (pPath[1] == '\\') {
				strcat_s(path1, pPath + 1);
				pPath = path1;
			}

		}
		if (pPath[1] == ':' && ((pPath[0] >= 'A' && pPath[0] <= 'Z') || (pPath[0] >= 'a' && pPath[0] <= 'z'))) { //check path format
			char tempPath[MAXPATHLEN] = "\\\\?\\";
			strcat_s(tempPath, pPath);
			strcpy_s(pPath, MAXPATHLEN, tempPath);
		}

		if (pPath[5] != ':' || !((pPath[4] >= 'A' && pPath[4] <= 'Z') || (pPath[4] >= 'a' && pPath[4] <= 'z'))) { //check path format
			printf("Path %s format is incorrect.\n", pPath);
			printf("Please use a full path such as C:\\work or \\\\?\\C:\\work\n");

			return NULL;
		}

		for (size_t i = 0; i < strlen(pPath); i++) {
			if (pPath[i] == '/') {
				pPath[i] = '\\';
			}
		}
	} else if (format == FORMAT_PATHALIAS) {
		if (pPath[1] == ':' && ((pPath[0] >= 'A' && pPath[0] <= 'Z') || (pPath[0] >= 'a' && pPath[0] <= 'z'))) {
			//transform Windows format to mount path d:\work => /d/work
			pPath[1] = pPath[0];
			pPath[0] = '/';
			for (size_t i = 2; i < strlen(pPath); i++) {
				if (pPath[i] == '\\') {
					pPath[i] = '/';
				}
			}
		} else if (pPath[0] != '/') { //check path alias format
			printf("Path alias format is incorrect.\n");
			printf("Please use a path like /exports\n");

			return NULL;
		}
	}

	return pPath;
}
