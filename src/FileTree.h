#ifndef _FILETREE_H_
#define _FILETREE_H_

#include "FileTable.h"

class CFileTree
{
	public:
		FILE_ITEM AddItem(char *absolutePath, unsigned char *handle);
		void RemoveItem(char *absolutePath);
		void RenameItem(char *absolutePathFrom, char *absolutePathTo);
		
	protected:
		tree_node_<FILE_ITEM>* findNodeFromRootWithPath(char *path);
		tree_node_<FILE_ITEM>* findNodeWithPathFromNode(std::string path, tree_node_<FILE_ITEM>* node);
		tree_node_<FILE_ITEM>* findParentNodeFromRootForPath(char *path);
};
extern void DisplayTree(tree_node_<FILE_ITEM>* node, int level);

#endif