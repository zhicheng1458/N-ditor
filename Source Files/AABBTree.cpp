#include "AABBTree.h"

AABBTree::AABBTree()
{
	//numleaf = 0;
	treeShader = new ShaderProgram("./Resources/Shaders/Model.glsl", ShaderProgram::eRender); //TODO: custom shader
	root = nullptr;
}

AABBTree::~AABBTree()
{
	delete treeShader;
	if (root == nullptr) { return; }

	std::stack<AABBNode *> nodeStack;
	nodeStack.push(root);

	while (!nodeStack.empty())
	{
		AABBNode * node = nodeStack.top();
		nodeStack.pop();
		if (!node->isLeaf())
		{
			nodeStack.push(node->child1);
			nodeStack.push(node->child2);
		}
		delete node;
	}
}

AABB AABBTree::getAABB()
{
	return AABB();
}

/*
std::vector<unsigned int> AABBTree::getPotentialOverlaps(AABB box)
{
	//fprintf(std::err, "AABB tree search only support GameObject type.");
	std::vector<T *> overlaps;
	std::stack<unsigned int> stack;
	AABB objectAABB = obj->getTightAABB(); //Preferably the tight aabb box;

	stack.push(rootIndex);
	while (!stack.empty())
	{
		unsigned int nodeIndex = stack.top();
		stack.pop();

		if (nodeIndex == AABB_NULL_NODE) { continue; }

		AABBNode<T> & node = nodes[nodeIndex];

		if (node.box.overlaps(objectAABB))
		{
			if (node.isLeaf() && node.obj != obj)
			{
				overlaps.push_back(node.obj);
			}
			else
			{
				stack.push(node.child1);
				stack.push(node.child2);
			}
		}
	}

	return overlaps;
}
*/

/*
std::vector<AABBNode>& AABBTree::getNodeTree()
{
	return nodes;
}
*/

void AABBTree::update()
{
	if (root == nullptr) { return; }

	AABBTreeDrawing.clearBuffers();

	std::stack<AABBNode *> nodeStack;
	nodeStack.push(root);

	while (!nodeStack.empty())
	{
		AABBNode * node = nodeStack.top();
		nodeStack.pop();
		if (!node->isLeaf())
		{
			nodeStack.push(node->child1);
			nodeStack.push(node->child2);
		}
		glm::vec2 bottomLeft = node->box.lowerBound;
		glm::vec2 topRight = node->box.upperBound;
		glm::vec2 topLeft = glm::vec2(bottomLeft.x, topRight.y);
		glm::vec2 bottomRight = glm::vec2(topRight.x, bottomLeft.y);
		float width = topRight.x - topLeft.x;
		float height = topRight.y - bottomRight.y;
		glm::vec2 offset = glm::vec2(width * 0.1f, height * 0.1f);
		bottomLeft += glm::vec2(-offset.x, -offset.y);
		topRight += glm::vec2(offset.x, offset.y);
		topLeft += glm::vec2(-offset.x, offset.y);
		bottomRight += glm::vec2(offset.x, -offset.y);

		AABBTreeDrawing.addLine(bottomLeft, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), topLeft, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 2.0f);
		AABBTreeDrawing.addLine(topLeft, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), topRight, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 2.0f);
		AABBTreeDrawing.addLine(topRight, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), bottomRight, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 2.0f);
		AABBTreeDrawing.addLine(bottomRight, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), bottomLeft, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), 2.0f);
	}

	AABBTreeDrawing.setBuffers();
}

void AABBTree::draw(const glm::mat4 & viewProjMtx)
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Only draw the outline of the shape/triangles
	AABBTreeDrawing.draw(glm::mat4(1.0f), viewProjMtx, treeShader->getProgramID());
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //Draw the solid shape/triangles
}

///////////////////////////////////////////////////////////////////////////////

void AABBTree::insertLeaf(AABBNode & leaf)
{
	if (root == nullptr)
	{
		root = &leaf;
		root->child1 = nullptr;
		root->child2 = nullptr;
		root->parent = nullptr;
		return;
	}

	//Step 1: Find best node to serve as the sibling of the node to insert.
	AABBNode & siblingNode = getBestSiblingNode(leaf);

	//Step 2: Create a new parent node that takes both the leaf and the sibling,
	//then hook up all the nodes, left and right order doesn't matter
	AABBNode * newParent = new AABBNode();
	newParent->objectIndex = AABB_INTERNAL_NODE;

	AABBNode * siblingNodeOldParent = siblingNode.parent;
	newParent->parent = siblingNodeOldParent;
	newParent->child1 = &leaf; //The two children is the node to be inserted and the current best index
	newParent->child2 = &siblingNode;
	leaf.parent = newParent;
	siblingNode.parent = newParent;

	if (siblingNodeOldParent == nullptr) //No parent means it was the root
	{
		root = newParent;
	}
	else
	{
		if (siblingNodeOldParent->child1 == &siblingNode)
		{
			siblingNodeOldParent->child1 = newParent;
		}
		else
		{
			siblingNodeOldParent->child2 = newParent;
		}
	}

	//Step 3: Fix the tree starting from the new parent
	fixTree(*leaf.parent);
}

//Returns the index of the object contained in the node, if found, otherwise return -1 for nothing found;
int AABBTree::find(glm::vec2 searchCoordinate)
{
	if (root == nullptr) { return -1; }

	AABBNode * node = root;
	std::stack<AABBNode *> nodeStack; //Unfortunately u can have overlap :/
	nodeStack.push(root);

	while (!nodeStack.empty())
	{
		AABBNode * currentNode = nodeStack.top();
		nodeStack.pop();

		if (!currentNode->isLeaf())
		{
			if (currentNode->box.contains(searchCoordinate))
			{
				nodeStack.push(currentNode->child1);
				nodeStack.push(currentNode->child2);
			}
			else
			{
				continue;
			}
		}
		else
		{
			if (currentNode->box.contains(searchCoordinate))
			{
				return currentNode->objectIndex;
			}
		}
	}

	return -1;

}

/*
void AABBTree::removeLeaf(AABBNode & leaf)
{
	if (leafIndex == rootIndex)
	{
		rootIndex = AABB_NULL_NODE;
		return;
	}

	AABBNode<T> & nodeToRemove = nodes[leafIndex];
	unsigned int parentIndex = nodeToRemove.parentIndex;
	AABBNode<T> & nodeToRemoveParent = nodes[parentIndex];
	unsigned int grandparentIndex = nodeToRemoveParent.parentIndex;
	unsigned int siblingIndex;
	if (nodeToRemoveParent.child1 == leafIndex)
	{
		siblingIndex = nodeToRemoveParent.child2;
	}
	else
	{
		siblingIndex = nodeToRemoveParent.child1;
	}
	AABBNode<T> & nodeToRemoveSibling = nodes[siblingIndex];

	if (grandparentIndex == AABB_NULL_NODE) //Parent was the root
	{
		rootIndex = siblingIndex;
		nodeToRemoveSibling.parentIndex = AABB_NULL_NODE;
		deallocateNode(parentIndex);
	}
	else
	{
		AABBNode<T> & nodeToRemoveGrandparent = nodes[grandparentIndex];
		if (nodeToRemoveGrandparent.child1 == parentIndex)
		{
			nodeToRemoveGrandparent.child1 = siblingIndex;
		}
		else
		{
			nodeToRemoveGrandparent.child2 = siblingIndex;
		}
		nodeToRemoveSibling.parentIndex = grandparentIndex;
		deallocateNode(parentIndex);

		//Since the leaves changed the tree must be fixed
		fixTree(grandparentIndex);
	}

	nodeToRemove.parentIndex = AABB_NULL_NODE;

}
*/

///////////////////////////////////////////////////////////////////////////////

struct NodeAndCost
{
	AABBNode * node;
	float inheritedCost;
};

AABBNode & AABBTree::getBestSiblingNode(const AABBNode & leaf)
{
	if (root->isLeaf()) { return *root; }

	std::queue<NodeAndCost> toCheckList;
	NodeAndCost bestPair;
	bestPair.node = root;
	bestPair.inheritedCost = 0; //root node has no inheritance cost
	toCheckList.push(bestPair);

	float bestCost = (root->box + leaf.box).surfaceArea;

	while (toCheckList.size() > 0)
	{
		NodeAndCost currentPair = toCheckList.front();
		toCheckList.pop();

		//Cost of current node = SA(current node + leaf node) + refitting cost of all node along the way from root
		//                     = SA(current node + leaf node) + SA(parent node + leaf node) - SA(parent node) + remaining refitting cost of all node along the way to root
		//                     = SA(current node + leaf node) - SA(current node) + cost of parent node (if any)
		float cost = (currentPair.node->box + leaf.box).surfaceArea + currentPair.inheritedCost;
		if (root != currentPair.node) { cost -= currentPair.node->parent->box.surfaceArea; }

		if (cost < bestCost)
		{
			bestPair = currentPair;
			bestCost = cost;
		}

		//Lowerbound cost of child node = SA(leaf node) - SA(current node) + cost of current node
		float costLowerBound = leaf.box.surfaceArea - currentPair.node->box.surfaceArea + cost;
		if (costLowerBound < bestCost)
		{
			if (!currentPair.node->isLeaf())
			{
				NodeAndCost child1pair;
				child1pair.node = currentPair.node->child1;
				child1pair.inheritedCost = cost;
				toCheckList.push(child1pair);

				NodeAndCost child2pair;
				child2pair.node = currentPair.node->child2;
				child2pair.inheritedCost = cost;
				toCheckList.push(child2pair);
			}
		}
	}

	return *bestPair.node;
}

void AABBTree::fixTree(AABBNode & node)
{
	AABBNode * currentNode = &node;
	while (currentNode != nullptr)
	{
		AABBNode & leftNode = *(currentNode->child1);
		AABBNode & rightNode = *(currentNode->child2);

		currentNode->box = leftNode.box + rightNode.box;

		//TODO: Rotate node to rebalance tree
		rotateSubTree(node);

		currentNode = currentNode->parent;
	}
}

void AABBTree::rotateSubTree(AABBNode & node) //index is this node
{
	if (node.parent == nullptr) { return; }

	//unsigned int parentIndex = nodes[index].parentIndex;
	AABBNode * parentNode = node.parent;

	AABBNode * siblingNode = nullptr; //Sibling node will never be nullptr because each parent will always have 2 child
	if (parentNode->child1 == &node)
	{
		siblingNode = parentNode->child2;
	}
	else
	{
		siblingNode = parentNode->child1;
	}

	AABBNode * currentChild1 = node.child1;
	AABBNode * currentChild2 = node.child2;
	AABBNode * siblingChild1 = nullptr;
	AABBNode * siblingChild2 = nullptr;

	float currentNodeBestCostReduction = 0.0f;
	float siblingNodeBestCostReduction = 0.0f;
	unsigned int currentNodeOption = 0;
	unsigned int siblingNodeOption = 0;

	float costCurrent = node.box.surfaceArea;
	float costOfLeftChildSwap = (siblingNode->box + currentChild2->box).surfaceArea; //Theoretical surface area of sibling node with the right child
	float costOfRightChildSwap = (siblingNode->box + currentChild1->box).surfaceArea; //Theoretical surface area of sibling node with the left child

	if (costOfLeftChildSwap < costCurrent)
	{
		//keep child1
		currentNodeOption = 1;
		currentNodeBestCostReduction = costCurrent - costOfLeftChildSwap; //Track if can get better
	}

	if (costOfRightChildSwap < costCurrent && costOfRightChildSwap < costOfLeftChildSwap)
	{
		//keep child2
		currentNodeOption = 2;
		currentNodeBestCostReduction = costCurrent - costOfRightChildSwap; //Track if can get better
	}

	if (!siblingNode->isLeaf())
	{
		siblingChild1 = siblingNode->child1;
		siblingChild2 = siblingNode->child2;

		costCurrent = siblingNode->box.surfaceArea;
		costOfLeftChildSwap = (node.box + siblingChild2->box).surfaceArea;
		costOfRightChildSwap = (node.box + siblingChild1->box).surfaceArea;

		if (costOfLeftChildSwap < costCurrent)
		{
			//keep child1
			siblingNodeOption = 1;
			siblingNodeBestCostReduction = costCurrent - costOfLeftChildSwap; //Track if can get better
		}

		if (costOfRightChildSwap < costCurrent && costOfRightChildSwap < costOfLeftChildSwap)
		{
			//keep child2
			siblingNodeOption = 2;
			siblingNodeBestCostReduction = costCurrent - costOfRightChildSwap; //Track if can get better
		}
	}

	//If sibling node best cost reduction is bigger than current node best cost reduction, it implies sibling node must have valid child
	if (currentNodeBestCostReduction >= siblingNodeBestCostReduction)
	{
		if (currentNodeOption == 1)
		{
			//swap node child1 with sibling node
			node.child1 = siblingNode;
			siblingNode->parent = &node;
			currentChild1->parent = parentNode;
			if (parentNode->child1 == &node)
			{
				parentNode->child2 = currentChild1;
			}
			else
			{
				parentNode->child1 = currentChild1;
			}
			node.box = node.child1->box + node.child2->box;
		}
		else if (currentNodeOption == 2)
		{
			//swap node child2 with sibling node
			node.child2 = siblingNode;
			siblingNode->parent = &node;
			currentChild2->parent = parentNode;
			if (parentNode->child1 == &node)
			{
				parentNode->child2 = currentChild2;
			}
			else
			{
				parentNode->child1 = currentChild2;
			}
			node.box = node.child1->box + node.child2->box;
		}

		//If it gets here this means current node is already at its best condition
	}
	else
	{
		if (siblingNodeOption == 1)
		{
			//swap node with sibling's child1 node
			siblingNode->child1 = &node;
			node.parent = siblingNode;
			siblingChild1->parent = parentNode;
			if (parentNode->child1 == &node)
			{
				parentNode->child1 = siblingChild1;
			}
			else
			{
				parentNode->child2 = siblingChild1;
			}
			siblingNode->box = siblingNode->child1->box + siblingNode->child2->box;
		}
		else if (siblingNodeOption == 2)
		{
			//swap node with sibling's child2 node
			siblingNode->child2 = &node;
			node.parent = siblingNode;
			siblingChild2->parent = parentNode;
			if (parentNode->child1 == &node)
			{
				parentNode->child1 = siblingChild2;
			}
			else
			{
				parentNode->child2 = siblingChild2;
			}
			siblingNode->box = siblingNode->child1->box + siblingNode->child2->box;
		}
		//If it gets here this means sibling node is already at its best condition
	}
}