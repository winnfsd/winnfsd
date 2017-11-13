#include "FileTree.h"
#include <string>
#include <io.h>
#include <stdio.h>
#include <windows.h>
#include <sys/stat.h>
#include "tree.hh"
#include "conv.h"

static tree<FILE_ITEM> filesTree;
static tree<FILE_ITEM>::iterator topNode;

std::string _first_dirname(std::string path) {
	auto wcs = _conv_from_932(path.c_str());
	if (wcs == NULL) {
		return path.substr(0, path.find('\\'));
	}
	auto wpath = std::basic_string<wchar_t>(wcs);
	delete wcs;
	auto dest = _conv_to_932(wpath.substr(0, wpath.find(L'\\')).c_str());
	if (dest == NULL) {
		return path.substr(0, path.find('\\'));
	}
	auto result = std::string(dest);
	delete dest;
	return result;
}

std::string _dirname_932(std::string path) {
	auto wcs = _conv_from_932(path.c_str());
	auto result = path.find('\\') != std::string::npos ? path.substr(0, path.find_last_of('\\')) : std::string("");
	if (wcs == NULL) {
		return result;
	}
	auto wpath = std::basic_string<wchar_t>(wcs);
	delete wcs;
	if (wpath.find(L'\\') == std::basic_string<wchar_t>::npos) {
		return result;
	}
	auto dest = _conv_to_932(wpath.substr(0, wpath.find_last_of(L'\\')).c_str());
	if (dest == NULL) {
		return result;
	}
	result = std::string(dest);
	delete dest;
	return result;
}

std::string _following_path(std::string path) {
	auto wcs = _conv_from_932(path.c_str());
	if (wcs == NULL) {
		return path.find('\\') != std::string::npos ? path.substr(path.find('\\') + 1) : std::string("");
	}
	auto wpath = std::basic_string<wchar_t>(wcs);
	delete wcs;
	if (wpath.find(L'\\') == std::basic_string<wchar_t>::npos) {
		return std::string("");
	}
	auto dest = _conv_to_932(wpath.substr(wpath.find(L'\\') + 1).c_str());
	if (dest == NULL) {
		return path.find('\\') != std::string::npos ? path.substr(path.find('\\') + 1) : std::string("");
	}
	auto result = std::string(dest);
	delete dest;
	return result;
}

std::string _basename_932(std::string path) {
	auto wcs = _conv_from_932(path.c_str());
	if (wcs == NULL) {
		return path.substr(path.find_last_of('\\') + 1);
	}
	auto wpath = std::basic_string<wchar_t>(wcs);
	delete wcs;
	auto dest = _conv_to_932(wpath.substr(wpath.find_last_of(L'\\') + 1).c_str());
	if (dest == NULL) {
		return path.substr(path.find_last_of('\\') + 1);
	}
	auto result = std::string(dest);
	delete dest;
	return result;
}

FILE_ITEM CFileTree::AddItem(const char *absolutePath, unsigned char* handle)
{
	FILE_ITEM item;
	item.handle = handle;
	item.bCached = false;

	// If the tree is empty just add the new path as node on the top level.
	if (filesTree.empty()) {
		item.path = new char[strlen(absolutePath) + 1];
		strcpy_s(item.path, (strlen(absolutePath) + 1), absolutePath);
		item.nPathLen = strlen(item.path);

		filesTree.set_head(item);
		topNode = filesTree.begin();
	}
	else {
		// Check if the requested path belongs to an already registered parent node.
		std::string sPath(absolutePath);
		tree_node_<FILE_ITEM>* parentNode = findParentNodeFromRootForPath(absolutePath);
		std::string splittedPath = _basename_932(sPath);
		//printf("spl %s %s\n", splittedPath.c_str(), absolutePath);
		item.path = new char[splittedPath.length() + 1];
		strcpy_s(item.path, (splittedPath.length() + 1), splittedPath.c_str());
		// If a parent was found use th parent.
		if (parentNode) {
			//printf("parent %s\n", parentNode->data.path);
			filesTree.append_child(tree<FILE_ITEM>::iterator_base(parentNode), item);
		} else {
			// Node wasn't found - most likely a new root - add it to the top level.
			//printf("No parent node found for %s. Adding new sibbling.", absolutePath);
			item.path = new char[strlen(absolutePath) + 1];
			strcpy_s(item.path, (strlen(absolutePath) + 1), absolutePath);
			item.nPathLen = strlen(item.path);
			
			filesTree.insert(tree<FILE_ITEM>::iterator_base(topNode), item);
			topNode = filesTree.begin();
		}
	}

	DisplayTree(topNode.node, 0);

	return item;
}

void CFileTree::RemoveItem(const char *absolutePath)
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

void CFileTree::RenameItem(const char *absolutePathFrom, const char *absolutePathTo)
{
	tree_node_<FILE_ITEM>* node = findNodeFromRootWithPath(absolutePathFrom);
	tree_node_<FILE_ITEM>* parentNode = findParentNodeFromRootForPath(absolutePathTo);

	if (parentNode != NULL && node != NULL) {
		if (filesTree.number_of_children(parentNode) < 1) {
			FILE_ITEM emptyItem;
			emptyItem.nPathLen = 0;
			emptyItem.path = const_cast<char*>("");
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

tree_node_<FILE_ITEM>* CFileTree::FindFileItemForPath(const char *absolutePath)
{
	tree_node_<FILE_ITEM>* node = findNodeFromRootWithPath(absolutePath);
	if (node == NULL) {
		return NULL;
	}
	return node;
}

tree_node_<FILE_ITEM>* CFileTree::findNodeFromRootWithPath(const char *path)
{
	// No topNode - bail out.
	if (topNode.node == NULL){
		return NULL;
	}
	std::string sPath(path);
	std::string nPath(topNode->path);
	// topNode path and requested path are the same? Use the node.
	if (sPath == nPath) {
		return topNode.node;
	}
	// printf("Did not find node for path : %s\n", path);

	// If the topNode path is part of the requested path this is a subpath.
	// Use the node.
	if (sPath.find(nPath) != std::string::npos) {
		// printf("Found %s is part of %s  \n", sPath.c_str(), topNode->path);
		std::string splittedString = sPath.substr(strlen(topNode->path) + 1);
		return findNodeWithPathFromNode(splittedString.c_str(), topNode.node);
	}
	else {
		// If the current topNode isn't related to the requested path
		// iterate over all _top_ level elements in the tree to look for
		// a matching item and register it as current top node.

		// printf("NOT found %s is NOT part of %s  \n", sPath.c_str(), topNode->path);
		tree<FILE_ITEM>::sibling_iterator it;
		for (it = filesTree.begin(); it != filesTree.end(); it++)
		{
			std::string itPath(it.node->data.path);
			// Current item path matches the requested path - use the item as topNode.
			if (sPath == itPath) {
				// printf("Found parent node %s \n", it.node->data.path);
				topNode = it;
				return it.node;
			}
			else if (sPath.find(itPath) != std::string::npos) {
				// If the item path is part of the requested path this is a subpath.
				// Use the the item as topNode and continue analyzing.
				// printf("Found root node %s \n", it.node->data.path);
				topNode = it;
				std::string splittedString = sPath.substr(itPath.length() + 1);
				return findNodeWithPathFromNode(splittedString.c_str(), it.node);
			}
		}
	}
	// Nothing found return NULL.
	return NULL;
}

tree_node_<FILE_ITEM>* CFileTree::findNodeWithPathFromNode(const char *path, tree_node_<FILE_ITEM>* node)
{
	tree<FILE_ITEM>::sibling_iterator sib = filesTree.begin(node);
	tree<FILE_ITEM>::sibling_iterator end = filesTree.end(node);
	bool currentLevel = true;

	std::string currentPath = _first_dirname(path);

	size_t position = currentPath.size();
	std::string followingPath = _following_path(path);
	currentLevel = followingPath.empty();

	while (sib != end) {
		// printf("sib->path '%s' lv %d curpath '%s' follow '%s'\n", sib->path, currentLevel, currentPath.c_str(), followingPath.c_str());
		if (strcmp(sib->path, currentPath.c_str()) == 0) {
			if (currentLevel) {
				return sib.node;
			}
			else {
				return findNodeWithPathFromNode(followingPath.c_str(), sib.node);
			}
		}
		++sib;
	}
	return NULL;
}

tree_node_<FILE_ITEM>* CFileTree::findParentNodeFromRootForPath(const char *path) {
	std::string sPath(path);
	std::string nPath(topNode->path);

	// If the topNode path is not part of the requested path bail out.
	// This avoids also issues with taking substrings of incompatible
	// paths below.
	if (sPath.find(nPath) == std::string::npos) {
		// printf("Path %s doesn't belong to current topNode %s Found %s is part of %s  \n", sPath.c_str(), topNode->path);
		return NULL;
	}
	std::string currentPath = sPath.substr(strlen(topNode->path) + 1);
	std::string followingPath = _dirname_932(currentPath);
	if (followingPath.empty()) {
		return topNode.node;
	} else {
		return findNodeWithPathFromNode(followingPath.c_str(), topNode.node);
	}
}

void CFileTree::GetNodeFullPath(tree_node_<FILE_ITEM>* node, std::string &path)
{
	path.append(node->data.path);
	tree_node_<FILE_ITEM>* parentNode = node->parent;
	while (parentNode != NULL)
	{
		path.insert(0, "\\");
		path.insert(0, parentNode->data.path);
		parentNode = parentNode->parent;
	}
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
