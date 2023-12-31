#include <stdio.h>
#include <string.h>

#include "tree.h"

uint32_t *internalNodeNumKeys(void *node) {
    return node + INTERNAL_NODE_NUM_KEYS_OFFSET;
}

uint32_t *internalNodeRightChild(void *node) {
    return node + INTERNAL_NODE_RIGHT_CHILD_OFFSET;
}

uint32_t *internalNodeCell(void *node, uint32_t cellNum) {
    return node + INTERNAL_NODE_HEADER_SIZE + cellNum * INTERNAL_NODE_CELL_SIZE;
}

uint32_t *internalNodeChild(void *node, uint32_t childNum) {
    uint32_t numKeys = *internalNodeNumKeys(node);

    if (childNum > numKeys) {
        printf("Tried to access childNum %d > numKeys %d\n", childNum, numKeys);
        exit(EXIT_FAILURE);
    } else if (childNum == numKeys) {
        uint32_t *rightChild = internalNodeRightChild(node);

        if (*rightChild == INVALID_PAGE_NUM) {
            printf(
                "Tried to access right child of node, but was invalid page\n");
            exit(EXIT_FAILURE);
        }

        return rightChild;
    } else {
        uint32_t *child = internalNodeCell(node, childNum);

        if (*child == INVALID_PAGE_NUM) {
            printf("Tried to access child %d of node, but was invalid page\n",
                   childNum);
            exit(EXIT_FAILURE);
        }

        return child;
    }
}

uint32_t *internalNodeKey(void *node, uint32_t keyNum) {
    return (void *)internalNodeCell(node, keyNum) + INTERNAL_NODE_CHILD_SIZE;
}

uint32_t internalNodeFindChild(void *node, uint32_t key) {
    uint32_t numKeys = *internalNodeNumKeys(node);

    uint32_t minIndex = 0;
    uint32_t maxIndex = numKeys;

    while (minIndex != maxIndex) {
        uint32_t index = (minIndex + maxIndex) / 2;
        uint32_t keyToRight = *internalNodeKey(node, index);

        if (keyToRight >= key) {
            maxIndex = index;
        } else {
            minIndex = index + 1;
        }
    }

    return minIndex;
}

Cursor *internalNodeFind(Table *table, uint32_t pageNum, uint32_t key) {
    void *node = getPage(table->pager, pageNum);

    uint32_t childIndex = internalNodeFindChild(node, key);
    uint32_t childNum = *internalNodeChild(node, childIndex);
    void *child = getPage(table->pager, childNum);

    switch (getNodeType(child)) {
    case NODE_LEAF:
        return leafNodeFind(table, childNum, key);
    case NODE_INTERNAL:
        return internalNodeFind(table, childNum, key);
    }
}

void internalNodeSplitAndInsert(Table *table, uint32_t parentPageNum,
                                uint32_t childPageNum) {
    uint32_t oldPageNum = parentPageNum;
    void *oldNode = getPage(table->pager, oldPageNum);
    uint32_t oldMax = getNodeMaxKey(table->pager, oldNode);

    void *child = getPage(table->pager, childPageNum);
    uint32_t childMax = getNodeMaxKey(table->pager, child);

    uint32_t newPageNum = getUnusedPageNum(table->pager);

    uint32_t splittingRoot = isNodeRoot(oldNode);

    void *parent;
    void *newNode;

    if (splittingRoot) {
        createNewRoot(table, newPageNum);
        parent = getPage(table->pager, table->rootPageNum);

        // If we are splitting the root, we need to update oldNode to point to
        // the new root's left child, newPageNum will already point to the new
        // root's right child
        oldPageNum = *internalNodeChild(parent, 0);
        oldNode = getPage(table->pager, oldPageNum);
    } else {
        parent = getPage(table->pager, *nodeParent(oldNode));
        newNode = getPage(table->pager, newPageNum);
        initializeInternalNode(newNode);
    }

    uint32_t *oldNumKeys = internalNodeNumKeys(oldNode);

    uint32_t curPageNum = *internalNodeRightChild(oldNode);
    void *cur = getPage(table->pager, curPageNum);

    // First put right child into new node and set right child of old node to
    // invalid page number
    internalNodeInsert(table, newPageNum, curPageNum);
    *nodeParent(cur) = newPageNum;
    *internalNodeRightChild(oldNode) = INVALID_PAGE_NUM;

    // For each key until you get to the middle key, move the key and the child
    // to the new node
    for (int i = INTERNAL_NODE_MAX_CELLS - 1; i > INTERNAL_NODE_MAX_CELLS / 2;
         i--) {
        curPageNum = *internalNodeChild(oldNode, i);
        cur = getPage(table->pager, curPageNum);

        internalNodeInsert(table, newPageNum, curPageNum);
        *nodeParent(cur) = newPageNum;

        (*oldNumKeys)--;
    }

    // Set child before middle key, which is now the highest key, to be node's
    // right child, and decrement number of keys
    *internalNodeRightChild(oldNode) =
        *internalNodeChild(oldNode, *oldNumKeys - 1);
    (*oldNumKeys)--;

    // Determine which of the two nodes after the split should contain to be
    // inserted, and insert the child
    uint32_t maxAfterSplit = getNodeMaxKey(table->pager, oldNode);

    uint32_t destinationPageNum =
        childMax < maxAfterSplit ? oldPageNum : newPageNum;

    internalNodeInsert(table, destinationPageNum, childPageNum);
    *nodeParent(child) = destinationPageNum;

    updateInternalNodeKey(parent, oldMax, getNodeMaxKey(table->pager, oldNode));

    if (!splittingRoot) {
        internalNodeInsert(table, *nodeParent(oldNode), newPageNum);
        *nodeParent(newNode) = *nodeParent(oldNode);
    }
}

void internalNodeInsert(Table *table, uint32_t parentPageNum,
                        uint32_t childPageNum) {
    // Add a new child/key pair to parent that corresponds to child
    void *parent = getPage(table->pager, parentPageNum);
    void *child = getPage(table->pager, childPageNum);

    uint32_t childMaxKey = getNodeMaxKey(table->pager, child);
    uint32_t index = internalNodeFindChild(parent, childMaxKey);

    uint32_t originalNumKeys = *internalNodeNumKeys(parent);

    if (originalNumKeys >= INTERNAL_NODE_MAX_CELLS) {
        internalNodeSplitAndInsert(table, parentPageNum, childPageNum);
        return;
    }

    uint32_t rightChildPageNum = *internalNodeRightChild(parent);

    if (rightChildPageNum == INVALID_PAGE_NUM) {
        *internalNodeRightChild(parent) = childPageNum;
        return;
    }

    void *rightChild = getPage(table->pager, rightChildPageNum);

    *internalNodeNumKeys(parent) = originalNumKeys + 1;

    if (childMaxKey > getNodeMaxKey(table->pager, rightChild)) {
        // Replace right child
        *internalNodeChild(parent, originalNumKeys) = rightChildPageNum;
        *internalNodeKey(parent, originalNumKeys) =
            getNodeMaxKey(table->pager, rightChild);
        *internalNodeRightChild(parent) = childPageNum;
    } else {
        // Make room for the new cell
        for (uint32_t i = originalNumKeys; i > index; i++) {
            void *destination = internalNodeCell(parent, i);
            void *source = internalNodeCell(parent, i - 1);
            memcpy(destination, source, INTERNAL_NODE_CELL_SIZE);
        }

        *internalNodeChild(parent, index) = childPageNum;
        *internalNodeKey(parent, index) = childMaxKey;
    }
}

NodeType getNodeType(void *node) {
    uint8_t value = *((uint8_t *)(node + NODE_TYPE_OFFSET));
    return (NodeType)value;
}

void setNodeType(void *node, NodeType type) {
    uint8_t value = type;
    *((uint8_t *)(node + NODE_TYPE_OFFSET)) = value;
}

bool isNodeRoot(void *node) {
    uint8_t value = *((uint8_t *)(node + IS_ROOT_OFFSET));
    return (bool)value;
}

void setNodeRoot(void *node, bool isRoot) {
    uint8_t value = isRoot;
    *((uint8_t *)(node + IS_ROOT_OFFSET)) = value;
}

uint32_t *nodeParent(void *node) { return node + PARENT_POINTER_OFFSET; }

void updateInternalNodeKey(void *node, uint32_t oldKey, uint32_t newKey) {
    uint32_t oldChildIndex = internalNodeFindChild(node, oldKey);
    *internalNodeKey(node, oldChildIndex) = newKey;
}

uint32_t *leafNodeNumCells(void *node) {
    return node + LEAF_NODE_NUM_CELLS_OFFSET;
}

void *leafNodeCell(void *node, uint32_t cellNum) {
    return node + LEAF_NODE_HEADER_SIZE + cellNum * LEAF_NODE_CELL_SIZE;
}

uint32_t *leafNodeKey(void *node, uint32_t cellNum) {
    return leafNodeCell(node, cellNum);
}

void *leafNodeValue(void *node, uint32_t cellNum) {
    return leafNodeCell(node, cellNum) + LEAF_NODE_KEY_SIZE;
}

uint32_t *leafNodeNextLeaf(void *node) {
    return node + LEAF_NODE_NEXT_LEAF_OFFSET;
}

Cursor *leafNodeFind(Table *table, uint32_t pageNum, uint32_t key) {
    void *node = getPage(table->pager, pageNum);
    uint32_t numCells = *leafNodeNumCells(node);

    Cursor *cursor = malloc(sizeof(Cursor));
    cursor->table = table;
    cursor->pageNum = pageNum;

    uint32_t minIndex = 0;
    uint32_t onePastMaxIndex = numCells;

    while (onePastMaxIndex != minIndex) {
        uint32_t index = (minIndex + onePastMaxIndex) / 2;
        uint32_t keyAtIndex = *leafNodeKey(node, index);

        if (key == keyAtIndex) {
            cursor->cellNum = index;
            return cursor;
        }

        if (key < keyAtIndex) {
            onePastMaxIndex = index;
        } else {
            minIndex = index + 1;
        }
    }

    cursor->cellNum = minIndex;
    return cursor;
}

void leafNodeSplitAndInsert(Cursor *cursor, uint32_t key, Row *value) {
    // Create a new node and move half the cells over
    // Insert the new value in one of the two nodes
    // Update parent or create a new parent
    void *oldNode = getPage(cursor->table->pager, cursor->pageNum);
    uint32_t oldMax = getNodeMaxKey(cursor->table->pager, oldNode);

    uint32_t newPageNum = getUnusedPageNum(cursor->table->pager);
    void *newNode = getPage(cursor->table->pager, newPageNum);

    initializeLeafNode(newNode);

    *nodeParent(newNode) = *nodeParent(oldNode);

    *leafNodeNextLeaf(newNode) = *leafNodeNextLeaf(oldNode);
    *leafNodeNextLeaf(oldNode) = newPageNum;

    // All existing keys plus new key should be divided
    // evently between old (left) and new (right) nodes
    // Starting fromt the right, move each key to correct position
    for (int32_t i = LEAF_NODE_MAX_CELLS; i >= 0; i--) {
        void *destinationNode;

        if (i >= LEAF_NODE_LEFT_SPLIT_COUNT) {
            destinationNode = newNode;
        } else {
            destinationNode = oldNode;
        }

        uint32_t indexWithinNode = i % LEAF_NODE_LEFT_SPLIT_COUNT;
        void *destination = leafNodeCell(destinationNode, indexWithinNode);

        if (i == cursor->cellNum) {
            serializeRow(value,
                         leafNodeValue(destinationNode, indexWithinNode));
            *leafNodeKey(destinationNode, indexWithinNode) = key;
        } else if (i > cursor->cellNum) {
            memcpy(destination, leafNodeCell(oldNode, i - 1),
                   LEAF_NODE_CELL_SIZE);
        } else {
            memcpy(destination, leafNodeCell(oldNode, i), LEAF_NODE_CELL_SIZE);
        }
    }

    // Update cell count on both leaf nodes
    *(leafNodeNumCells(oldNode)) = LEAF_NODE_LEFT_SPLIT_COUNT;
    *(leafNodeNumCells(newNode)) = LEAF_NODE_RIGHT_SPLIT_COUNT;

    if (isNodeRoot(oldNode)) {
        return createNewRoot(cursor->table, newPageNum);
    } else {
        uint32_t parentPageNum = *nodeParent(oldNode);
        uint32_t newMax = getNodeMaxKey(cursor->table->pager, oldNode);

        void *parent = getPage(cursor->table->pager, parentPageNum);

        updateInternalNodeKey(parent, oldMax, newMax);
        internalNodeInsert(cursor->table, parentPageNum, newPageNum);

        return;
    }
}

void leafNodeInsert(Cursor *cursor, uint32_t key, Row *value) {
    void *node = getPage(cursor->table->pager, cursor->pageNum);

    uint32_t numCells = *leafNodeNumCells(node);

    // Node full
    if (numCells >= LEAF_NODE_MAX_CELLS) {
        leafNodeSplitAndInsert(cursor, key, value);
        return;
    }

    // Make room for new cell
    if (cursor->cellNum < numCells) {
        for (uint32_t i = numCells; i > cursor->cellNum; i--) {
            memcpy(leafNodeCell(node, i), leafNodeCell(node, i - 1),
                   LEAF_NODE_CELL_SIZE);
        }
    }

    *(leafNodeNumCells(node)) += 1;
    *(leafNodeKey(node, cursor->cellNum)) = key;
    serializeRow(value, leafNodeValue(node, cursor->cellNum));
}

void initializeLeafNode(void *node) {
    setNodeType(node, NODE_LEAF);
    setNodeRoot(node, false);
    *leafNodeNumCells(node) = 0;
    *leafNodeNextLeaf(node) = 0;
}

void initializeInternalNode(void *node) {
    setNodeType(node, NODE_INTERNAL);
    setNodeRoot(node, false);
    *internalNodeNumKeys(node) = 0;
    *internalNodeRightChild(node) = INVALID_PAGE_NUM;
}

uint32_t getUnusedPageNum(Pager *pager) { return pager->numPages; }

uint32_t getNodeMaxKey(Pager *pager, void *node) {
    if (getNodeType(node) == NODE_LEAF) {
        return *leafNodeKey(node, *leafNodeNumCells(node) - 1);
    }

    void *rightChild = getPage(pager, *internalNodeRightChild(node));
    return getNodeMaxKey(pager, rightChild);
}

void createNewRoot(Table *table, uint32_t rightChildPageNum) {
    // Handle splitting the root
    // Old root copied to new page, becomes left child
    // Address of right child passed in
    // Re-initialize root page to contain the new root node
    // New root node points to two children
    void *root = getPage(table->pager, table->rootPageNum);
    void *rightChild = getPage(table->pager, rightChildPageNum);

    uint32_t leftChildPageNum = getUnusedPageNum(table->pager);
    void *leftChild = getPage(table->pager, leftChildPageNum);

    if (getNodeType(root) == NODE_INTERNAL) {
        initializeInternalNode(rightChild);
        initializeInternalNode(leftChild);
    }

    // Left child has data copied from old rootx
    memcpy(leftChild, root, PAGE_SIZE);
    setNodeRoot(leftChild, false);

    if (getNodeType(leftChild) == NODE_INTERNAL) {
        void *child;

        for (int i = 0; i < *internalNodeNumKeys(leftChild); i++) {
            child = getPage(table->pager, *internalNodeChild(leftChild, i));
            *nodeParent(child) = leftChildPageNum;
        }

        child = getPage(table->pager, *internalNodeRightChild(leftChild));
        *nodeParent(child) = leftChildPageNum;
    }

    // Root node is a new internal node with one key and two children
    initializeInternalNode(root);
    setNodeRoot(root, true);

    *internalNodeNumKeys(root) = 1;
    *internalNodeChild(root, 0) = leftChildPageNum;

    uint32_t leftChildMaxKey = getNodeMaxKey(table->pager, leftChild);

    *internalNodeKey(root, 0) = leftChildMaxKey;
    *internalNodeRightChild(root) = rightChildPageNum;
}

void indent(uint32_t level) {
    for (uint32_t i = 0; i < level; i++) {
        printf("  ");
    }
}

void printConstants() {
    printf("ROW_SIZE: %d\n", ROW_SIZE);
    printf("COMMON_NODE_HEADER_SIZE: %d\n", COMMON_NODE_HEADER_SIZE);
    printf("LEAF_NODE_HEADER_SIZE: %d\n", LEAF_NODE_HEADER_SIZE);
    printf("LEAF_NODE_CELL_SIZE: %d\n", LEAF_NODE_CELL_SIZE);
    printf("LEAF_NODE_SPACE_FOR_CELLS: %d\n", LEAF_NODE_SPACE_FOR_CELLS);
    printf("LEAF_NODE_MAX_CELLS: %d\n", LEAF_NODE_MAX_CELLS);
}

void printTree(Pager *pager, uint32_t pageNum, uint32_t indentationLevel) {
    void *node = getPage(pager, pageNum);
    uint32_t numKeys, child;

    switch (getNodeType(node)) {
    case NODE_LEAF:
        numKeys = *leafNodeNumCells(node);

        indent(indentationLevel);
        printf("- leaf (size %d)\n", numKeys);

        for (uint32_t i = 0; i < numKeys; i++) {
            indent(indentationLevel + 1);
            printf("- %d\n", *leafNodeKey(node, i));
        }
        break;
    case NODE_INTERNAL:
        numKeys = *internalNodeNumKeys(node);

        indent(indentationLevel);
        printf("- internal (size %d)\n", numKeys);

        if (numKeys > 0) {
            for (uint32_t i = 0; i < numKeys; i++) {
                child = *internalNodeChild(node, i);
                printTree(pager, child, indentationLevel + 1);

                indent(indentationLevel + 1);
                printf("- key %d\n", *internalNodeKey(node, i));
            }
        }

        child = *internalNodeRightChild(node);
        printTree(pager, child, indentationLevel + 1);
        break;
    }
}
