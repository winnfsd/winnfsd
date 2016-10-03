#include "MountProg.h"
#include "FileTable.h"
#include <string.h>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <direct.h>
#include "common.hh"

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

    for (i = 0; i < MOUNT_NUM_MAX; i++) {
        delete[] m_pClientAddr[i];
    }

}

bool CMountProg::SetPathFile(char *file)
{
    std::string formattedFile = FormatPath(file, FORMAT_PATH);

	if (!formattedFile.length()) {
		return false;
	}

	std::ifstream pathFile(formattedFile);

	if (pathFile.good()) {
		pathFile.close();
		m_PathFile = formattedFile;
		return true;
	}

	pathFile.close();
	return false;
}

void CMountProg::Export(char *path, char *pathAlias)
{
	std::string formattedPath = FormatPath(path, FORMAT_PATH);
	std::string r;
	r = FormatPath(pathAlias, FORMAT_PATHALIAS);

	pathAlias = const_cast<char *>(r.c_str());

	if (path != NULL && pathAlias != NULL) {
		if (m_PathMap.count(pathAlias) == 0) {
			m_PathMap[pathAlias] = formattedPath;
			printf("Path #%i is: %s, path alias is: %s\n", m_PathMap.size(), path, pathAlias);
		} else {
			printf("Path %s with path alias  %s already known\n", path, pathAlias);
		}
	}

}

bool CMountProg::Refresh()
{
	if (m_PathFile.size()) {
		ReadPathsFromFile(m_PathFile.c_str());
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
    std::string path;
	unsigned char* handle;
	int i;

	PrintLog("MNT");
	PrintLog(" from %s\n", m_pParam->pRemoteAddr);

	if (GetPath(path)) {
		m_pOutStream->Write(MNT_OK); //OK
		handle = GetFileHandle(path.c_str());
		if (m_pParam->nVersion == 1) {
			m_pOutStream->Write(&handle, g_FHSIZE);  //fhandle
		} else {
			m_pOutStream->Write(g_NFS3_FHSIZE);  //length
			m_pOutStream->Write(handle, g_NFS3_FHSIZE);  //fhandle
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
    int i;

    PrintLog("UMNT");
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

bool CMountProg::GetPath(std::string &returnPath)
{
	unsigned long i, nSize;
	static char path[MAXPATHLEN + 1];
	static char finalPath[MAXPATHLEN + 1];
	bool foundPath = false;

	memset(path, 0, sizeof(path));
	memset(finalPath, 0, sizeof(finalPath));

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

	returnPath = std::string(finalPath);
	return foundPath;
}


bool CMountProg::ReadPathsFromFile(const char* sFileName)
{
	std::ifstream pathFile(sFileName);

	if (pathFile.is_open()) {
		std::string line, path;
		std::vector<std::string> paths;
		std::istringstream ss;

		while (std::getline(pathFile, line)) {
			ss.clear();
			paths.clear();
			ss.str(line);

			// split path and alias separated by '>'
			while (std::getline(ss, path, '>')) {
				paths.push_back(path);
			}
			if (paths.size() < 1) {
				continue;
			}
			if (paths.size() < 2) {
				paths.push_back(paths[0]);
			}

			char *pCurPath = (char*)malloc(paths[0].size() + 1);
			pCurPath = (char*)paths[0].c_str();
			
			if (pCurPath != NULL) {
				char *pCurPathAlias = (char*)malloc(paths[1].size() + 1);
				pCurPathAlias = (char*)paths[1].c_str();
				Export(pCurPath, pCurPathAlias);
			}
		}
	} else {
		printf("Can't open file %s.\n", sFileName);
		return false;
	}

	return true;
}

std::string CMountProg::FormatPath(char *pPath, pathFormats format)
{
	std::string ret;
    size_t len = strlen(pPath);

	//Remove head spaces
	while (*pPath == ' ') {
		++pPath;
		len--;
	}

	//Remove tail spaces
	while (len > 0 && *(pPath + len - 1) == ' ') {
		len--;
	}

	//Is comment?
	if (*pPath == '#') {
		return ret;
	}

	//Remove head "
	if (*pPath == '"') {
		++pPath;
		len--;
	}

	//Remove tail "
	if (len > 0 && *(pPath + len - 1) == '"') {
		len--;
	}

	if (len < 1) {
		return ret;
	}

	char *result = (char *)malloc(len + 1);
	strncpy_s(result, len + 1, pPath, len);

	//Check for right path format
	if (format == FORMAT_PATH) {
		if (result[0] == '.') {
			static char path1[MAXPATHLEN];
			_getcwd(path1, MAXPATHLEN);

			if (result[1] == '\0') {
				len = strlen(path1);
				result = (char *)realloc(result, len + 1);
				strcpy_s(result, len + 1, path1);
			} else if (result[1] == '\\') {
				strcat_s(path1, result + 1);
				len = strlen(path1);
				result = (char *)realloc(result, len + 1);
				strcpy_s(result, len + 1, path1);
			}

		}
		if (len >= 2 && result[1] == ':' && ((result[0] >= 'A' && result[0] <= 'Z') || (result[0] >= 'a' && result[0] <= 'z'))) { //check path format
			char tempPath[MAXPATHLEN] = "\\\\?\\";
			strcat_s(tempPath, result);
			len = strlen(tempPath);
			result = (char *)realloc(result, len + 1);
			strcpy_s(result, len + 1, tempPath);
		}

		if (len < 6 || result[5] != ':' || !((result[4] >= 'A' && result[4] <= 'Z') || (result[4] >= 'a' && result[4] <= 'z'))) { //check path format
			printf("Path %s format is incorrect.\n", pPath);
			printf("Please use a full path such as C:\\work or \\\\?\\C:\\work\n");
			free(result);
			return ret;
		}

		for (size_t i = 0; i < len; i++) {
			if (result[i] == '/') {
				result[i] = '\\';
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
			strncpy_s(result, len + 1, pPath, len);
		} else if (pPath[0] != '/') { //check path alias format
			printf("Path alias format is incorrect.\n");
			printf("Please use a path like /exports\n");
			free(result);
			return ret;
		}
	}

	ret = std::string(result);
	free(result);
	return ret;
}
