#include "FileTree.h"
#include <string.h>
#include <io.h>
#include <stdio.h>
#include <Windows.h>
#include <sys/stat.h>
#include "tree.hh"

static tree<FILE_ITEM> filesTree;
static tree<FILE_ITEM>::iterator topNode;


FILE_ITEM CFileTree::AddItem(char *absolutePath, unsigned char* handle)
{
	FILE_ITEM item;
	item.handle = handle;
	item.bCached = false;

	if (filesTree.empty()) {
		item.path = new char[strlen(absolutePath) + 1];
		strcpy_s(item.path, (strlen(absolutePath) + 1), absolutePath);
		item.nPathLen = strlen(item.path);

		filesTree.set_head(item);
		topNode = filesTree.begin();
	}
	else {
		std::string sPath(absolutePath);
		tree_node_<FILE_ITEM>* parentNode = findParentNodeFromRootForPath(absolutePath);
		std::string splittedPath = sPath.substr(sPath.find_last_of('\\') + 1);
		item.path = new char[splittedPath.length() + 1];
		strcpy_s(item.path, (splittedPath.length() + 1), splittedPath.c_str());
		if (parentNode) {
			filesTree.append_child(tree<FILE_ITEM>::iterator_base(parentNode), item);
		}
	}

	printf("\n\n\n<<<<<<<<<<<<<<<<<<<<<DISPLAY tree \n\n\n");
	DisplayTree(topNode.node, 0);
	printf("\n\n\n<<<<<<<<<<<<<<<<<<<<<End tree \n\n\n");

	return item;
}

void CFileTree::RemoveItem(char *absolutePath)
{
	tree<FILE_ITEM>::iterator node = findNodeFromRootWithPath(absolutePath);
	if (node != NULL) {
		filesTree.erase(node);
	}
	else {
		printf("do not find node to delete : %s", absolutePath);
	}
	printf("\n\n\n<<<<<<<<<<<<<<<<<<<<<DISPLAY tree \n\n\n");
	DisplayTree(topNode.node, 0);
	printf("\n\n\n<<<<<<<<<<<<<<<<<<<<<End tree \n\n\n");
}

void CFileTree::RenameItem(char *absolutePathFrom, char *absolutePathTo)
{
	tree_node_<FILE_ITEM>* node = findNodeFromRootWithPath(absolutePathFrom);
	tree_node_<FILE_ITEM>* parentNode = findParentNodeFromRootForPath(absolutePathTo);

	if (parentNode != NULL && node != NULL) {
		tree<FILE_ITEM>::iterator firstChild = filesTree.begin(parentNode);
		tree<FILE_ITEM>::iterator endChild = filesTree.end(parentNode);
		// must be tested
		//filesTree.move_after(firstChild, node);
		//filesTree.reparent(parentNode, node, node);

		std::string sPath(absolutePathTo);
		std::string splittedPath = sPath.substr(sPath.find_last_of('\\') + 1);
		node->data.path = new char[splittedPath.length() + 1];
		strcpy_s(node->data.path, (splittedPath.length() + 1), splittedPath.c_str());

	}

	printf("\n\n\n<<<<<<<<<<<<<<<<<<<<<DISPLAY tree \n\n\n");
	DisplayTree(topNode.node, 0);
	printf("\n\n\n<<<<<<<<<<<<<<<<<<<<<End tree \n\n\n");
	
}

tree_node_<FILE_ITEM>* CFileTree::findNodeFromRootWithPath(char *path)
{
	std::string sPath(path);
	std::string splittedString = sPath.substr(strlen(topNode->path) + 1);
	return findNodeWithPathFromNode(splittedString, topNode.node);
}

tree_node_<FILE_ITEM>* CFileTree::findNodeWithPathFromNode(std::string path, tree_node_<FILE_ITEM>* node)
{
	tree<FILE_ITEM>::iterator sib = filesTree.begin(node);
	tree<FILE_ITEM>::iterator end = filesTree.end(node);
	bool currentLevel = true;

	std::string currentPath = path.substr(0, path.find('\\'));
	size_t position = path.find('\\');
	std::string followingPath("");
	if (position != std::string::npos) {
		followingPath = path.substr(path.find('\\') + 1);
		currentLevel = false;
	}

	while (sib != end) {
		if (strcmp(sib->path, currentPath.c_str()) == 0) {
			if (currentLevel) {
				return sib.node;
			}
			else {
				return findNodeWithPathFromNode(followingPath, sib.node);
			}
		}
		++sib;
	}
	return NULL;
}

tree_node_<FILE_ITEM>* CFileTree::findParentNodeFromRootForPath(char *path) {
	std::string sPath(path);
	std::string currentPath = sPath.substr(strlen(topNode->path) + 1);
	size_t position = currentPath.find('\\');
	std::string followingPath("");
	if (position == std::string::npos) {
		return topNode.node;
	}
	else {
		followingPath = currentPath.substr(0, currentPath.find_last_of('\\'));
		return findNodeWithPathFromNode(followingPath, topNode.node);
	}
}


void DisplayTree(tree_node_<FILE_ITEM>* node, int level)
{
	tree<FILE_ITEM>::sibling_iterator  sib2 = filesTree.begin(node);
	tree<FILE_ITEM>::sibling_iterator  end2 = filesTree.end(node);
	while (sib2 != end2) {
		for (int i = 0; i < level; i++) {
			printf("  ");
		}
		printf("(l.%i) %s\n", level, sib2->path);
		if (tree<FILE_ITEM>::number_of_children(sib2) > 1) {
			DisplayTree(sib2.node, (level + 1));
		}
		++sib2;
	}
}