#ifndef __TAFFY_BRIDGE_H__
#define __TAFFY_BRIDGE_H__

#include <string>

#include "dcTaffy.h"

// extra node types
// start it high enough to be bigger than graph data types
enum NodeType_e
{
    NODE_CLASS_NIL = NODE_GRAPH_DATA_LAST + 1,
    NODE_CLASS_FUNCTION,
    NODE_FLAT_ARITHMETIC_DIVIDE,
    NODE_FLAT_ARITHMETIC_RAISE,
    NODE_FLAT_ARITHMETIC_OTHER
};

typedef int NodeType;

// A singleton bridge for Taffy evaluation
class TaffyBridge
{
public:
    static TaffyBridge &getInstance();

    // can't copy
    TaffyBridge(const TaffyBridge &other) = delete;
    TaffyBridge &operator=(const TaffyBridge &other) = delete;

    dcNode *evaluate(const std::string &input);

    NodeType getNodeType(const dcNode *node) const;

protected:
    TaffyBridge();
    virtual ~TaffyBridge();
};

extern "C"
{
    // help out the Taffy library
    void dcFlatArithmetic_free(dcFlatArithmetic **_arithmetic, dcDepth _depth);
}

#endif
