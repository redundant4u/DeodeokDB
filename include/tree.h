#ifndef tree_h
#define tree_h

#include <unistd.h>

#include "row.h"
#include "table.h"

typedef enum {
    NODE_INTERNAL,
    NODE_LEAF,
} NodeType;

// Common Node Header Layout
static const uint32_t NODE_TYPE_SIZE = sizeof(uint8_t);
static const uint32_t NODE_TYPE_OFFSET = 0;
static const uint32_t IS_ROOT_SIZE = sizeof(uint8_t);
static const uint32_t IS_ROOT_OFFSET = NODE_TYPE_SIZE;
static const uint32_t PARENT_POINTER_SIZE = sizeof(uint32_t);
static const uint32_t PARENT_POINTER_OFFSET = IS_ROOT_OFFSET + IS_ROOT_SIZE;
static const uint8_t COMMON_NODE_HEADER_SIZE =
    NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE;

// Leaf Node Header Layout
static const uint32_t LEAF_NODE_NUM_CELLS_SIZE = sizeof(uint32_t);
static const uint32_t LEAF_NODE_NUM_CELLS_OFFSET = COMMON_NODE_HEADER_SIZE;
static const uint32_t LEAF_NODE_NEXT_LEAF_SIZE = sizeof(uint32_t);
static const uint32_t LEAF_NODE_NEXT_LEAF_OFFSET =
    LEAF_NODE_NUM_CELLS_OFFSET + LEAF_NODE_NUM_CELLS_SIZE;
static const uint32_t LEAF_NODE_HEADER_SIZE = COMMON_NODE_HEADER_SIZE +
                                              LEAF_NODE_NUM_CELLS_SIZE +
                                              LEAF_NODE_NEXT_LEAF_SIZE;

// Leaf Node Body Layout
static const uint32_t LEAF_NODE_KEY_SIZE = sizeof(uint32_t);
static const uint32_t LEAF_NODE_KEY_OFFSET = 0;
static const uint32_t LEAF_NODE_VALUE_SIZE = ROW_SIZE;
static const uint32_t LEAF_NODE_VALUE_OFFSET =
    LEAF_NODE_KEY_OFFSET + LEAF_NODE_KEY_SIZE;
static const uint32_t LEAF_NODE_CELL_SIZE =
    LEAF_NODE_KEY_SIZE + LEAF_NODE_VALUE_SIZE;
static const uint32_t LEAF_NODE_SPACE_FOR_CELLS =
    PAGE_SIZE - LEAF_NODE_HEADER_SIZE;
static const uint32_t LEAF_NODE_MAX_CELLS =
    LEAF_NODE_SPACE_FOR_CELLS / LEAF_NODE_CELL_SIZE;

static const uint32_t LEAF_NODE_RIGHT_SPLIT_COUNT =
    (LEAF_NODE_MAX_CELLS + 1) / 2;
static const uint32_t LEAF_NODE_LEFT_SPLIT_COUNT =
    (LEAF_NODE_MAX_CELLS + 1) - LEAF_NODE_RIGHT_SPLIT_COUNT;

// Internal Node Header Layout
static const uint32_t INTERNAL_NODE_NUM_KEYS_SIZE = sizeof(uint32_t);
static const uint32_t INTERNAL_NODE_NUM_KEYS_OFFSET = COMMON_NODE_HEADER_SIZE;
static const uint32_t INTERNAL_NODE_RIGHT_CHILD_SIZE = sizeof(uint32_t);
static const uint32_t INTERNAL_NODE_RIGHT_CHILD_OFFSET =
    INTERNAL_NODE_NUM_KEYS_OFFSET + INTERNAL_NODE_NUM_KEYS_SIZE;
static const uint32_t INTERNAL_NODE_HEADER_SIZE =
    COMMON_NODE_HEADER_SIZE + INTERNAL_NODE_NUM_KEYS_SIZE +
    INTERNAL_NODE_RIGHT_CHILD_SIZE;

// Internal Node Body Layout
static const uint32_t INTERNAL_NODE_KEY_SIZE = sizeof(uint32_t);
static const uint32_t INTERNAL_NODE_CHILD_SIZE = sizeof(uint32_t);
static const uint32_t INTERNAL_NODE_CELL_SIZE =
    INTERNAL_NODE_CHILD_SIZE + INTERNAL_NODE_KEY_SIZE;
static const uint32_t INTERNAL_NODE_MAX_CELLS = 3;

uint32_t *internalNodeNumKeys(void *node);
uint32_t *internalNodeRightChild(void *node);
uint32_t *internalNodeCell(void *node, uint32_t cellNum);
uint32_t *internalNodeChild(void *node, uint32_t childNum);
uint32_t *internalNodeKey(void *node, uint32_t keyNum);
uint32_t internalNodeFindChild(void *node, uint32_t key);
Cursor *internalNodeFind(Table *table, uint32_t pageNum, uint32_t key);
void internalNodeInsert(Table *table, uint32_t parentPageNum,
                        uint32_t childPageNum);

NodeType getNodeType(void *node);
void setNodeType(void *node, NodeType type);

bool isNodeRoot(void *node);
void setNodeRoot(void *node, bool isRoot);

uint32_t *nodeParent(void *node);

void updateInternalNodeKey(void *node, uint32_t oldKey, uint32_t newKey);

uint32_t *leafNodeNumCells(void *node);
void *leafNodeCell(void *node, uint32_t cellNum);
uint32_t *leafNodeKey(void *node, uint32_t cellNum);
void *leafNodeValue(void *node, uint32_t cellNum);
uint32_t *leafNodeNextLeaf(void *node);
Cursor *leafNodeFind(Table *table, uint32_t pageNum, uint32_t key);
void leafNodeSplitAndInsert(Cursor *cursor, uint32_t key, Row *value);
void leafNodeInsert(Cursor *cursor, uint32_t key, Row *value);

void initializeLeafNode(void *node);
void initializeInternalNode(void *node);

uint32_t getUnusedPageNum(Pager *pager);
uint32_t getNodeMaxKey(void *node);

void createNewRoot(Table *table, uint32_t rightChildPageNum);

void indent(uint32_t level);

void printConstants();
void printTree(Pager *pager, uint32_t pageNum, uint32_t indentationLevel);

#endif
