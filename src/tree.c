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
        return internalNodeRightChild(node);
    } else {
        return internalNodeCell(node, childNum);
    }
}

uint32_t *internalNodeKey(void *node, uint32_t keyNum) {
    return internalNodeCell(node, keyNum) + INTERNAL_NODE_CHILD_SIZE;
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
    uint32_t newPageNum = getUnusedPageNum(cursor->table->pager);
    void *newNode = getPage(cursor->table->pager, newPageNum);

    initializeLeafNode(newNode);

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
            serializeRow(value, destination);
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
        printf("Need to implement updating parent after split\n");
        exit(EXIT_FAILURE);
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
}

void initializeInternalNode(void *node) {
    setNodeType(node, NODE_INTERNAL);
    setNodeRoot(node, false);
    *internalNodeNumKeys(node) = 0;
}

uint32_t getUnusedPageNum(Pager *pager) { return pager->numPages; }

uint32_t getNodeMaxKey(void *node) {
    switch (getNodeType(node)) {
    case NODE_INTERNAL:
        return *internalNodeKey(node, *internalNodeNumKeys(node) - 1);
    case NODE_LEAF:
        return *leafNodeKey(node, *leafNodeNumCells(node) - 1);
    }
}

void createNewRoot(Table *table, uint32_t rightChildPageNum) {
    // Handle splitting the root
    // Old root copied to new page, becomes left child
    // Address of right child passed in
    // Re-initialize root page to contain the new root node
    // New root node points to two children
    void *root = getPage(table->pager, table->rootPageNum);
    void *rightChild = getPage(table->pager, rightChildPageNum);

    printf("%d\n", rightChildPageNum);

    uint32_t leftChildPageNum = getUnusedPageNum(table->pager);
    void *leftChild = getPage(table->pager, leftChildPageNum);

    // Left child has data copied from old rootx
    memcpy(leftChild, root, PAGE_SIZE);
    setNodeRoot(leftChild, false);

    // Root node is a new internal node with one key and two children
    initializeInternalNode(root);
    setNodeRoot(root, true);

    *internalNodeNumKeys(root) = 1;
    *internalNodeChild(root, 0) = leftChildPageNum;

    uint32_t leftChildMaxKey = getNodeMaxKey(leftChild);

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

        for (uint32_t i = 0; i < numKeys; i++) {
            child = *internalNodeChild(node, i);
            printTree(pager, child, indentationLevel + 1);

            indent(indentationLevel + 1);
            printf("- key %d\n", *internalNodeKey(node, i));
        }

        child = *internalNodeRightChild(node);
        printTree(pager, child, indentationLevel + 1);
        break;
    }
}
