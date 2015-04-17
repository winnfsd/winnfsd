#ifndef _FILETREE_H_
#define _FILETREE_H_

#include "FileTable.h"

class CFileTree
{
	public:
		FILE_ITEM AddItem(char *absolutePath, unsigned char *handle);
		
	protected:
		tree<FILE_ITEM>::iterator findNodeFromRootWithPath(char *path);
		tree<FILE_ITEM>::iterator findNodeWithPathFromNode(std::string path, tree<FILE_ITEM>::iterator node);
		tree<FILE_ITEM>::iterator findParentNodeFromRootForPath(char *path);
};
extern void DisplayTree(tree<FILE_ITEM>::iterator node, int level);

#endif