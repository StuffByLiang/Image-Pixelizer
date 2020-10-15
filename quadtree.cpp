
/**
 *
 * quadtree (pa3)
 * quadtree.cpp
 * This file will be used for grading.
 *
 */

#include "quadtree.h"
using namespace std;


// Node constructor, given.
quadtree::Node::Node(pair<int,int> ul, int d,RGBAPixel a, double v)
	:upLeft(ul),dim(d),avg(a),var(v),NW(NULL),NE(NULL),SE(NULL),SW(NULL)
	{}

// quadtree destructor, given.
quadtree::~quadtree() {
	clear();
}
// quadtree copy constructor, given.
quadtree::quadtree(const quadtree & other) {
	copy(other);
}
// quadtree assignment operator, given.
quadtree & quadtree::operator=(const quadtree & rhs) {
	if (this != &rhs) {
		clear();
		copy(rhs);
	}
	return *this;
}


quadtree::quadtree(PNG & imIn) {
  stats s(imIn);

	edge = std::min( imIn.width(), imIn.height() );
	int k = (int) log2(edge);
	edge = pow(2, k);

	root = buildTree(s, {0, 0}, k);
}


quadtree::Node * quadtree::buildTree(stats & s, pair<int,int> ul, int dim) {
	RGBAPixel avgPixel = s.getAvg(ul, dim);
	double var = s.getVar(ul, dim);
	Node* newNode = new Node(ul, dim, avgPixel, var);

	int o = pow(2, dim) / 2; // offset

	int x = ul.first;
	int y = ul.second;

	if (dim > 0) {
		newNode->NW = buildTree(s, {x    , y    }, dim - 1);
		newNode->NE = buildTree(s, {x + o, y    }, dim - 1);
		newNode->SE = buildTree(s, {x + o, y + o}, dim - 1);
		newNode->SW = buildTree(s, {x    , y + o}, dim - 1);
	}

	return newNode;
}


PNG quadtree::render() {
	PNG result(edge, edge);
	render(result, root);
	return result;
}

void quadtree::render(PNG & img, Node* node) {
	if(node == NULL) return;
	if(isLeaf(node)) {
		// base case, draw
		int x = node->upLeft.first;
		int y = node->upLeft.second;
		int sideLength = pow(2, node->dim);

		// fill square of size sideLength with upperLeft starting at (x, y)
		for(int i=0; i<sideLength; i++) {
			for(int j=0; j<sideLength; j++) {
				*img.getPixel(x + i, y + j) = node->avg;
			}
		}
	} else {
		// has children, do a recursive call
		render(img, node->NW);
		render(img, node->NE);
		render(img, node->SE);
		render(img, node->SW);
	}
}

int quadtree::count() {
	// std::cout << "init count" << std::endl;
	return count(root);
}

int quadtree::count(Node* node) {
	if(node == NULL) return 0;
	if(isLeaf(node)) return 1;

	return
		count(node->NW) +
		count(node->NE) +
		count(node->SE) +
		count(node->SW);
}

bool quadtree::isLeaf(Node* node) {
	return node->NW == NULL;
}

int quadtree::idealPrune(int leaves){


	// anonomus function that lets us know we found our soln
	auto isDone = [this, leaves](int tol) -> bool { return (pruneSize(tol) <= leaves && pruneSize(tol-1) > leaves); };

	bool verbose = false;

	// my second attempt at a binary search algorithm

	int low = 0;
	int high = INT_MAX;
	int mid;
	while (low <= high) {
		 mid = ((unsigned int) low + (unsigned int) high) >> 1;
		if (verbose) std::cout << mid << std::endl;
		if (isDone(mid)) break;
		if (pruneSize(mid) > leaves) {
			// more leaves than required, need larger tolerance
			low = mid + 1;
		} else {
			// need smaller tolerance
			high = mid - 1;
		}
	}

	if (verbose) std::cout << "found " << leaves << ": " << mid << std::endl;

	return mid;

	// binary search for the tolerance
	// int tol = INT_MAX/2;
	// int maxTol = INT_MAX;
	// int minTol = 0;

	// while(!isDone(tol)) {
	// 	// find largest lower bound
	// 	if (verbose) std::cout << "running" << std::endl;
	// 	if (verbose) std::cout << "tol: " << tol
	// 	                       << " "     << pruneSize(tol)
	// 												 << " "     << pruneSize(tol-1)
	// 												 << " "     << pruneSize(tol+1)
	// 												 << " "     << pruneSize(leaves)
	// 												 << std::endl;

	// 	while(pruneSize(tol) > leaves) { 
	// 		if (verbose) std::cout << "tol: " << tol << " min:" << minTol << " max:" << maxTol << std::endl;
	// 		minTol = tol;
	// 		tol = (tol/2+maxTol/2) >= maxTol ? maxTol-1 : (tol/2+maxTol/2);
	// 	}

	// 	if(isDone(tol)) break; // break loop if done

	// 	// find smallest upper bound
	// 	while(pruneSize(tol-1) <= leaves) {
	// 		if (verbose) std::cout << "tol: " << tol << " min:" << minTol << " max:" << maxTol << std::endl;
	// 		maxTol = tol;
	// 		tol = (tol+minTol)/2 <= minTol ? minTol+1 : (tol+minTol)/2;
	// 	}
	// }
	
	// if (verbose) std::cout << "DONE: curentTol: " << tol << " min:" << minTol << " max:" << maxTol << std::endl << std::endl;
	// return tol;
}

int quadtree::pruneSize(int tol){
	return pruneSize(root, tol);
}

int quadtree::pruneSize(Node* node, int tol) {
	if (node == NULL) return 0;
	if (prunable(node, tol)) return 1;
	return pruneSize(node->NE, tol)
	     + pruneSize(node->NW, tol)
			 + pruneSize(node->SE, tol)
			 + pruneSize(node->SW, tol);
}

void quadtree::prune(int tol){
	// std::cout << root->var << " " << tol << std::endl;
	prune(root, tol);
}

void quadtree::prune(Node* node, int tol) {
	if (node == NULL) return;

	if (prunable(node, tol)) {
		//clear subtrees
		clear(node->NW);
		clear(node->NE);
		clear(node->SE);
		clear(node->SW);
	} else {
		prune(node->NW, tol);
		prune(node->NE, tol);
		prune(node->SE, tol);
		prune(node->SW, tol);
	}
}

void quadtree::clear() {
	clear(root);
}

void quadtree::clear(Node* & node) {
	if(node == NULL) return;

	clear(node->SE);
	clear(node->SW);
	clear(node->NE);
	clear(node->NW);
	delete node;
	node = NULL;
}

void quadtree::copy(const quadtree & orig){
	root = copy(orig.root);
	edge = orig.edge;
}

quadtree::Node* quadtree::copy(Node* node) {
	if(node == NULL) return NULL;

	Node* nodeCopy = new Node(node->upLeft, node->dim, node->avg, node->var);

	nodeCopy->NE = copy(node->NE);
	nodeCopy->NW = copy(node->NW);
	nodeCopy->SE = copy(node->SE);
	nodeCopy->SW = copy(node->SW);
	return nodeCopy;
}


