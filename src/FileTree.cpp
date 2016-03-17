#include "FileTree.h"
#include <string.h>
#include <io.h>
#include <stdio.h>
#include <windows.h>
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
		} else {
			//printf("Parent node found for %s", absolutePath);
		}
	}

	DisplayTree(topNode.node, 0);

	return item;
}

void CFileTree::RemoveItem(char *absolutePath)
{
	tree_node_<FILE_ITEM>* node = findNodeFromRootWithPath(absolutePath);
	if (node != NULL) {
		filesTree.erase(tree<FILE_ITEM>::iterator(node));
	}
	else {
		//printf("Do not find node for path : %s\n", absolutePath);
	}

	DisplayTree(topNode.node, 0);
}

void CFileTree::RenameItem(char *absolutePathFrom, char *absolutePathTo)
{
	tree_node_<FILE_ITEM>* node = findNodeFromRootWithPath(absolutePathFrom);
	tree_node_<FILE_ITEM>* parentNode = findParentNodeFromRootForPath(absolutePathTo);

	if (parentNode != NULL && node != NULL) {
		if (filesTree.number_of_children(parentNode) < 1) {
			FILE_ITEM emptyItem;
			emptyItem.nPathLen = 0;
			emptyItem.path = "";
			filesTree.append_child(tree<FILE_ITEM>::iterator_base(parentNode), emptyItem);
		}
		tree<FILE_ITEM>::iterator firstChild = filesTree.begin(parentNode);
		filesTree.move_after(firstChild, tree<FILE_ITEM>::iterator(node));

		std::string sPath(absolutePathTo);
		std::string splittedPath = sPath.substr(sPath.find_last_of('\\') + 1);
		node->data.path = new char[splittedPath.length() + 1];
		strcpy_s(node->data.path, (splittedPath.length() + 1), splittedPath.c_str());

	}
	DisplayTree(topNode.node, 0);
}

tree_node_<FILE_ITEM>* CFileTree::FindFileItemForPath(char *absolutePath)
{
	tree_node_<FILE_ITEM>* node = findNodeFromRootWithPath(absolutePath);
	if (node == NULL) {
		return NULL;
	}
	return node;
}

tree_node_<FILE_ITEM>* CFileTree::findNodeFromRootWithPath(char *path)
{
	if (topNode.node == NULL){
		return NULL;
	}
	std::string sPath(path);
	if (sPath == std::string(topNode->path)) {
		return topNode.node;
	}
	std::string splittedString = sPath.substr(strlen(topNode->path) + 1);
	return findNodeWithPathFromNode(splittedString, topNode.node);
}

tree_node_<FILE_ITEM>* CFileTree::findNodeWithPathFromNode(std::string path, tree_node_<FILE_ITEM>* node)
{
	tree<FILE_ITEM>::sibling_iterator sib = filesTree.begin(node);
	tree<FILE_ITEM>::sibling_iterator end = filesTree.end(node);
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

char* CFileTree::GetNodeFullPath(tree_node_<FILE_ITEM>* node)
{
	std::string path;
	path.append(node->data.path);
	node = node->parent;
	while (node != NULL)
	{
		path.insert(0, "\\");
		path.insert(0, node->data.path);
		node = node->parent;
	}

	// TODO : Memory leak
	char *cstr = new char[path.length() + 1];
	strcpy_s(cstr, path.length() + 1, path.c_str());
	return cstr;
}

void DisplayTree(tree_node_<FILE_ITEM>* node, int level)
{
	if (CFileTree::debug) {
		printf("\n\n\n<<<<<<<<<<<<<<<<<<<<<DISPLAY tree \n\n\n");
		tree<FILE_ITEM>::sibling_iterator  sib2 = filesTree.begin(node);
		tree<FILE_ITEM>::sibling_iterator  end2 = filesTree.end(node);
		while (sib2 != end2) {
			for (int i = 0; i < level; i++) {
				printf("  ");
			}
			if (tree<FILE_ITEM>::number_of_children(sib2) > 1) {
				DisplayTree(sib2.node, (level + 1));
			}
			++sib2;
		}
		printf("\n\n\n<<<<<<<<<<<<<<<<<<<<<End tree \n\n\n");
	}
}
