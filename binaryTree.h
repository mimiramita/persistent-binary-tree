#ifndef BINARYTREE_H_INCLUDED
#define BINARYTREE_H_INCLUDED

#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/p.hpp>
#include <libpmemobj++/persistent_ptr.hpp>
#include <libpmemobj++/pool.hpp>
#include <libpmemobj++/transaction.hpp>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <sys/stat.h>

struct tree_node {
    pmem::obj::p<int> data;
    pmem::obj::persistent_ptr<tree_node> left;
    pmem::obj::persistent_ptr<tree_node> right;
    pmem::obj::persistent_ptr<tree_node> parent;
};

struct tree_iterator {
    pmem::obj::persistent_ptr<tree_node> currentNode;

    // Check whether there is the next value
    bool hasNext()
    {
        auto temp = currentNode;

        if (temp->right != nullptr)
        {
            return true;
        }

        auto p = temp->parent;
        while (p != nullptr && p->right == temp)
        {
            temp = p;
            p = temp->parent;
        }

        if (p == nullptr) {
            return false;
        }
        else {
            return true;
        }
    }

    // Check whether there is the previous value
    bool hasPrevious()
    {
        auto temp = currentNode;

        if (temp->left != nullptr) {
            return true;
        }

        auto p = temp->parent;
        while (p != nullptr && p->left == temp) {
            temp = p;
            p = temp->parent;
        }

        if (p == nullptr) {
            return false;
        }
        else {
            return true;
        }
    }

    // Move the iterator to the next node and return the value of the next node
    int next() {
        auto temp = currentNode;

        if (temp->right != nullptr) {
            temp = temp->right;
            while (temp->left != nullptr) {
                temp = temp->left;
            }
        }
        else {
            auto p = temp->parent;
            while (p != nullptr && p->right == temp) {
                temp = p;
                p = temp->parent;
            }
            temp = nullptr;
        }

        if (temp == nullptr) {
            throw std::out_of_range("There is no next value.");
        }

        currentNode = temp;
        return currentNode->data;
    }

    // Move the iterator to the previous node and return the original value
    int previous() {
        auto temp = currentNode;

        int d = temp->data;

        if (temp->left != nullptr) {
            temp = temp->left;

            while (temp->right != nullptr) {
                temp = temp->right;
            }
        }
        else {
            auto p = temp->parent;
            while (p != nullptr && p->left == temp) {
                temp = p;
                p = temp->parent;
            }
            temp = p;
        }

        if (temp == nullptr) {
            throw std::out_of_range("There is no previous value.");
        }

        currentNode = temp;
        return d;
    }

    // Set the data in the current node
    void set_value(int value) {
        currentNode->data = value;
    }
};

struct binary_tree {

    // Find the smallest value in the tree
    pmem::obj::persistent_ptr<tree_iterator> findMin(pmem::obj::pool_base &pop) {
    	auto a = findMin(pop, tree_root);
        return a;
    }

    pmem::obj::persistent_ptr<tree_iterator> findMin(pmem::obj::pool_base &pop, pmem::obj::persistent_ptr<tree_node> n) {
    
        if (n == nullptr) {
            return nullptr;
        }
        if (n->left == nullptr) {
        	pmem::obj::persistent_ptr<tree_iterator> itr;
            	pmem::obj::transaction::run(pop, [&]{
                	itr = pmem::obj::make_persistent<tree_iterator>();
                	itr->currentNode = n;
            	});
            	return itr;
        }
   
       return findMin(pop, n->left);   
    }

    // Find the largest value in the tree
    pmem::obj::persistent_ptr<tree_iterator> findMax(pmem::obj::pool_base &pop) {
        return findMax(pop, tree_root);
    }

    pmem::obj::persistent_ptr<tree_iterator> findMax(pmem::obj::pool_base &pop, pmem::obj::persistent_ptr<tree_node> n) {
        if (n == nullptr) {
          	return nullptr;
        }
        if (n->right == nullptr) {
        	pmem::obj::persistent_ptr<tree_iterator> itr;
            	pmem::obj::transaction::run(pop, [&]{
              		itr = pmem::obj::make_persistent<tree_iterator>();
              		itr->currentNode = n;
            	});
            	return itr;
        }
        return findMax(pop, n->right);
    }

    // Find a value in the tree
    bool node_in_the_tree(pmem::obj::pool_base &pop, int v) {
    	auto found = find_node(pop, v);
    	if (found == nullptr) {
    		return false;
    	}
    	
    	return true;
    }
    
    pmem::obj::persistent_ptr<tree_iterator> find_node(pmem::obj::pool_base &pop, int v) {
        return find_node(pop, v, tree_root);
    }

    pmem::obj::persistent_ptr<tree_iterator> find_node(pmem::obj::pool_base &pop, int v, pmem::obj::persistent_ptr<tree_node> n) {
        if (n == nullptr) {
            	return nullptr;
        }
        if (v == n->data) {
        	pmem::obj::persistent_ptr<tree_iterator> itr;
            	pmem::obj::transaction::run(pop, [&]{
              		itr = pmem::obj::make_persistent<tree_iterator>();
              		itr->currentNode = n;
            	});
            	return itr;
        }
        if (v < n->data) {
            	return find_node(pop, v, n->left);
        }
        else {
            	return find_node(pop, v, n->right);
        }
    }

    // Insert a new node
    pmem::obj::persistent_ptr<tree_node> insert_node(pmem::obj::pool_base &pop, int v) {
        pmem::obj::transaction::run(pop, [&]{
        tree_root = insert_node(pop, v, tree_root, nullptr);
        });
        return tree_root;
    }
    pmem::obj::persistent_ptr<tree_node> insert_node(pmem::obj::pool_base &pop, int v, pmem::obj::persistent_ptr<tree_node> n, pmem::obj::persistent_ptr<tree_node> parent) {
        pmem::obj::transaction::run(pop, [&]{
            if (n == nullptr) {
                n = pmem::obj::make_persistent<tree_node>();
                n->data = v;
                n->left = nullptr;
                n->right = nullptr;
                n->parent = parent;
            }
            else if (v < n->data) {
                n->left = insert_node(pop, v, n->left, n);
            }
            else if (v > n->data) {
                n->right = insert_node(pop, v, n->right, n);
            }
        });
        return n;
    }
    // Remove a node from the tree
    pmem::obj::persistent_ptr<tree_node> remove_node(pmem::obj::pool_base &pop, int v) {
    	pmem::obj::transaction::run(pop, [&]{
    		tree_root = remove_node(pop, v, tree_root, nullptr);
    	});
    	return tree_root;
    }
    
    pmem::obj::persistent_ptr<tree_node> remove_node(pmem::obj::pool_base &pop, int v, pmem::obj::persistent_ptr<tree_node> n, pmem::obj::persistent_ptr<tree_node> parent) {

        pmem::obj::transaction::run(pop, [&]{

            if (n == nullptr) {
                throw std::out_of_range("There is no such node.");
            }
            else if (v < n->data) {
                n->left = remove_node(pop, v, n->left, n);
            }
            else if (v > n->data) {
                n->right = remove_node(pop, v, n->right, n);
            }
            else {
                if (n->left == nullptr && n->right == nullptr) {
                    if (parent->right == n) {
                        parent->right = nullptr;
                    }
                    else {
                        parent->left = nullptr;
                    }

                    auto node_ptr = n;
                    pmem::obj::delete_persistent<tree_node>(node_ptr);
                    tree_size = tree_size - 1;
                    n = nullptr;
                }
                else if (n->left != nullptr && n->right == nullptr) {
                    pmem::obj::persistent_ptr<tree_node> n2 = n->left;
                    n2->parent = parent;
                    
                    if (parent != nullptr) {		     
                    	if (parent->right == n) {
                        	parent->right = n2;
                    	}
                    	else {
                        	parent->left = n2;
                    	}
                    }
                    
                    pmem::obj::delete_persistent<tree_node>(n);
                    tree_size = tree_size - 1;
                    n = n2;
                }
                else if (n->right != nullptr && n->left == nullptr) {
                    pmem::obj::persistent_ptr<tree_node> n2 = n->right;
                    n2->parent = parent; 

                    if (parent != nullptr) {		     
                    	if (parent->right == n) {
                        	parent->right = n2;
                    	}
                    	else {
                        	parent->left = n2;
                    	}
                    }
                    
                    pmem::obj::delete_persistent<tree_node>(n);
                    tree_size = tree_size - 1;
                    n = n2;
                }
                else {
                    auto i = findMin(pop, n->right);
                    int minInRightSubtree = i->currentNode->data;
                    n->data = minInRightSubtree;
                    n->right = remove_node(pop, minInRightSubtree, n->right, n);
                }
            }
        });
        return n;
    }
    
    pmem::obj::p<int> tree_size;
    pmem::obj::persistent_ptr<tree_node> tree_root;

};

#endif // BINARYTREE_H_INCLUDED
