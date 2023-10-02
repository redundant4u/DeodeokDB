#include <stdio.h>
#include <string.h>

#include "tree.h"

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

void leafNodeInsert(Cursor *cursor, uint32_t key, Row *value) {
    void *node = getPage(cursor->table->pager, cursor->pageNum);

    uint32_t numCells = *leafNodeNumCells(node);

    // Node full
    if (numCells >= LEAF_NODE_MAX_CELLS) {
        printf("Need to implement splitting a leaf node.\n");
        exit(EXIT_FAILURE);
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
    *leafNodeNumCells(node) = 0;
}

NodeType getNodeType(void *node) {
    uint8_t value = *((uint8_t *)(node + NODE_TYPE_OFFSET));
    return (NodeType)value;
}

void setNodeType(void *node, NodeType type) {
    uint8_t value = type;
    *((uint8_t *)(node + NODE_TYPE_OFFSET)) = value;
}

void printConstants() {
    printf("ROW_SIZE: %d\n", ROW_SIZE);
    printf("COMMON_NODE_HEADER_SIZE: %d\n", COMMON_NODE_HEADER_SIZE);
    printf("LEAF_NODE_HEADER_SIZE: %d\n", LEAF_NODE_HEADER_SIZE);
    printf("LEAF_NODE_CELL_SIZE: %d\n", LEAF_NODE_CELL_SIZE);
    printf("LEAF_NODE_SPACE_FOR_CELLS: %d\n", LEAF_NODE_SPACE_FOR_CELLS);
    printf("LEAF_NODE_MAX_CELLS: %d\n", LEAF_NODE_MAX_CELLS);
}

void printLeafNode(void *node) {
    uint32_t numCells = *leafNodeNumCells(node);
    printf("leaf (size %d)\n", numCells);

    for (uint32_t i = 0; i < numCells; i++) {
        uint32_t key = *leafNodeKey(node, i);
        printf("  - %d : %d\n", i, key);
    }
}
