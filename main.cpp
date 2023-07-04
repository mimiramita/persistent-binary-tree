#include <iostream> 
#include "binaryTree.h"
#include <libpmemobj.h>

using namespace std;

int main(int argc, char *argv[])
{
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " path_to_pool" << endl;
        return 1;
    }

    auto path = argv[1];
    
    // Create a memory pool.
	auto pool = pmem::obj::pool<binary_tree>::create(path, "binary_tree", PMEMOBJ_MIN_POOL);
    
    	auto q = pool.root();
    
    // Insert ten thousands nodes to the tree.
  	for (int i = 0; i < 10000; i++) {
  		int j = rand() % 100000;
    		q->insert_node(pool, j);
  	}
  	
    // Find the maximum value in the tree.
  	auto max = q->findMax(pool);
  	if (max == nullptr) {
  		cout << "null" << endl; 
  	}
  	else 
  	{
  		cout << max->currentNode->data << endl;
  	} 
  	
    // Find the minimum value in the tree.
  	auto min = q->findMin(pool); 
  	if (min == nullptr) {
  		cout << "null" << endl;
  	}  
  	else
  	{
  		cout << min->currentNode->data << endl;
  	}
  	
    // Remove some of the nodes from the tree.
  	for (int i = 0; i < 10000; i++) {
  		int j = rand() % 100000;
  		if (q->node_in_the_tree(pool, j)) {
  			q->remove_node(pool, j);
  		}
  	}
}
