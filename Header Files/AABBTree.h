#pragma once
#include "Core.h"
#include "AABB.h"
#include <stack>
#include <queue>

//Visualizing the node aabb box
#include "Model.h"
#include "Shader.h"

#define AABB_INTERNAL_NODE -1

struct AABBNode
{
	AABB box;
	int objectIndex; //This is the one you use to search your vector for the object, -1 means internal node
	AABBNode * parent;
	AABBNode * child1;
	AABBNode * child2;
	//bool isLeaf;

	bool isLeaf() { return child1 == nullptr; }

	AABBNode()
	{
		objectIndex = AABB_INTERNAL_NODE;
		parent = nullptr;
		child1 = nullptr;
		child2 = nullptr;
	}

	AABBNode(AABB objBox)
	{
		box = objBox;
		objectIndex = AABB_INTERNAL_NODE;
		parent = nullptr;
		child1 = nullptr;
		child2 = nullptr;
	}
};

class AABBTree
{
	public:
		AABBTree();
		~AABBTree();

		AABB getAABB();

		void insertLeaf(AABBNode & leaf);
		//void removeLeaf(AABBNode & leaf);
		int find(glm::vec2 searchCoordinate);

		//std::vector<unsigned int> getPotentialOverlaps(AABB box);

		//Get all the AABB nodes for testing purpose
		//std::vector<AABBNode> & getNodeTree();

		//Visualization
		void update();
		void draw(const glm::mat4 & viewProjMtx);

	private:

		AABBNode * root = nullptr;
		//unsigned int numleaf;

		//Helper methods
		AABBNode & getBestSiblingNode(const AABBNode & leaf);
		void fixTree(AABBNode & node);
		void rotateSubTree(AABBNode & node);

		//Visualizing the nodes
		Model AABBTreeDrawing;
		ShaderProgram * treeShader;


};


