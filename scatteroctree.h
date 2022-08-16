#pragma once

#include <d3dx9.h>
#include <vector>
#include "boundbox.h"
#include "point3.h"
#include "color.h"
#include "bssrdf.h"

struct ScatterSample
{
    Point3 position;
    Color irradiance;
    float area;
};

typedef ScatterSample NodeData;

class ScatterOctree
{
public:

    // Max number of data stored in the leaf node.
    static const size_t MAX_LEAF = 8;
    struct Node
    {
        Node();
        ~Node();
        Node* children[8];
        ScatterSample* data[MAX_LEAF];
        ScatterSample value;
        size_t count;
        bool isLeaf(void) const;
    };

public:

    ScatterOctree(const BoundBox& box);
    ~ScatterOctree(void);

    void insert(const NodeData& data);
    void config(void);
    Color lookup(const Point3& p, float maxErr, Bssrdf* bssrdf);

// private member functions
private:
    size_t withinChild(const Point3& mid, const Point3& p) const;
    void insertPrivate(Node* node, const BoundBox& nodeBound,
                       const NodeData& data, size_t depth);
    void lookupPrivate(Node* node, const BoundBox& nodeBound,
                       const Point3& p);
    void processData(const NodeData& data, const Point3& p);
    void configPrivate(Node* node);

// private data members
private:
    BoundBox m_bound;
    Node m_root;
    float m_maxErr;
    Color m_resultE;
    Bssrdf* m_bssrdf;

};
