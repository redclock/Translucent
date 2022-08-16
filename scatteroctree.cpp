#include <cmath>
#include "scatteroctree.h"

ScatterOctree::Node::Node()
{
    for (size_t i = 0; i < 8; ++i)
        children[i] = NULL;
    for (size_t i = 0; i < MAX_LEAF; ++i)
        data[i] = NULL;
    count = 0;
}

ScatterOctree::Node::~Node()
{
    for (size_t i = 0; i < 8; ++i)
        delete children[i];
    for (size_t i = 0; i < MAX_LEAF; ++i)
        delete data[i];
}

bool ScatterOctree::Node::isLeaf(void) const
{
    return children[0] == NULL
        && children[1] == NULL
        && children[2] == NULL
        && children[3] == NULL
        && children[4] == NULL
        && children[5] == NULL
        && children[6] == NULL
        && children[7] == NULL;
}

ScatterOctree::ScatterOctree(const BoundBox& box)
    : m_bound(box)
{
}

ScatterOctree::~ScatterOctree(void)
{
}

void ScatterOctree::insert(const NodeData& data)
{
    insertPrivate(&m_root, m_bound, data, 0);
}

size_t ScatterOctree::withinChild(const Point3& mid, const Point3& p) const
{
    // Determine which children the item overlaps
    // no   x y z
    // 0:   0 0 0
    // 1:   0 0 1
    // 2:   0 1 0
    // 3:   0 1 1
    // 4:   1 0 0
    // 5:   1 0 1
    // 6:   1 1 0
    // 7:   1 1 1
    size_t cond[3];

    cond[0] = p.x <= mid.x ? 0 : 1;
    cond[1] = p.y <= mid.y ? 0 : 1;
    cond[2] = p.z <= mid.z ? 0 : 1;

    size_t child = cond[0] << 2 | cond[1] << 1 | cond[2];

    return child;
}

void ScatterOctree::insertPrivate(Node* node, const BoundBox& nodeBound,
                                  const NodeData& data, size_t depth)
{
    Point3 midPoint = nodeBound.center();
    Point3 p = data.position;

    // Possibly add data item to current octree node
    if (node->isLeaf())
    {
        if (node->count < MAX_LEAF)
        {
            node->data[node->count] = new NodeData;
            *node->data[node->count] = data;
            ++node->count;
            return;
        }
        else
        {
            for (size_t i = 0; i < MAX_LEAF; ++i)
            {
                size_t child = withinChild(midPoint, node->data[i]->position);
                if (node->children[child] == NULL)
                    node->children[child] = new Node;
                node->children[child]->data[node->children[child]->count++] = 
                    node->data[i];
                node->data[i] = NULL;
            }
            node->count = 0;
        }
    }

    size_t child = withinChild(midPoint, p);

    if (!node->children[child])
        node->children[child] = new Node;
    // Compute bound box for octree child
    BoundBox childBound;
    childBound.minPoint.x = ((child & 4) ? midPoint.x : nodeBound.minPoint.x);
    childBound.maxPoint.x = ((child & 4) ? nodeBound.maxPoint.x : midPoint.x);
    childBound.minPoint.y = ((child & 2) ? midPoint.y : nodeBound.minPoint.y);
    childBound.maxPoint.y = ((child & 2) ? nodeBound.maxPoint.y : midPoint.y);
    childBound.minPoint.z = ((child & 1) ? midPoint.z : nodeBound.minPoint.z);
    childBound.maxPoint.z = ((child & 1) ? nodeBound.maxPoint.z : midPoint.z);
    insertPrivate(node->children[child], childBound, data, depth + 1);
}

void ScatterOctree::config(void)
{
    configPrivate(&m_root);
}

void ScatterOctree::configPrivate(Node* node)
{
    node->value.area = 0;
    node->value.irradiance = Color(0, 0, 0);
    node->value.position = Point3(0, 0, 0);
    float weight = 0.0f;
    if (node->isLeaf())
    {
        for (size_t i = 0; i < node->count; ++i)
        {
            node->value.area += node->data[i]->area;
            float w = node->data[i]->irradiance.meanValue();
            node->value.position += node->data[i]->position * w;
            node->value.irradiance += node->data[i]->irradiance * node->data[i]->area;
            weight += w;
        }
        if (weight > 0)
            node->value.position /= weight;
        if (node->value.area > 0)
            node->value.irradiance /= node->value.area;
    }
    else
    {
        for (size_t i = 0; i < 8; ++i)
        {
            if (node->children[i])
            {
                configPrivate(node->children[i]);
                node->value.area += node->children[i]->value.area;
                float w = node->children[i]->value.irradiance.meanValue();
                node->value.position += node->children[i]->value.position * w; 
                node->value.irradiance += 
                    node->children[i]->value.irradiance 
                    * node->children[i]->value.area;
                weight += w;
            }
        }
        if (weight > 0)
            node->value.position /= weight;
        if (node->value.area > 0)
            node->value.irradiance /= node->value.area;
    }
}

Color ScatterOctree::lookup(const Point3& p, float maxErr, Bssrdf* bssrdf)
{
    m_bssrdf = bssrdf;

    m_maxErr = maxErr;
    m_resultE = Color(0.0f, 0.0f, 0.0f);

    lookupPrivate(&m_root, m_bound, p);

    m_resultE *= (0.25f / 3.14159265f) * (1 - m_bssrdf->fdr) / m_bssrdf->fdr; 
    return m_resultE;
}

void ScatterOctree::lookupPrivate(Node* node, const BoundBox& nodeBound,
                                  const Point3& p)
{
    if (node->isLeaf())
    {
        for (size_t i = 0; i < node->count; ++i)
        {
            processData(*node->data[i], p);
        }
        return;
    }
    const ScatterSample& sample = node->value; 
    float d = sample.position.x - p.x;
    float d2 = d * d;
    d = sample.position.y - p.y;
    d2 += d * d;
    d = sample.position.z - p.z;
    d2 += d * d;
    float err = sample.area / d2;
    Point3 midPoint = nodeBound.center();
    if (err > m_maxErr || nodeBound.isInside(p))
    {
        for (size_t child = 0; child < 8; ++child)
        {
            if (node->children[child])
            {
                BoundBox childBound;
                childBound.minPoint.x = ((child & 4) ? 
                    midPoint.x : nodeBound.minPoint.x);
                childBound.maxPoint.x = ((child & 4) ? 
                    nodeBound.maxPoint.x : midPoint.x);
                childBound.minPoint.y = ((child & 2) ? 
                    midPoint.y : nodeBound.minPoint.y);
                childBound.maxPoint.y = ((child & 2) ? 
                    nodeBound.maxPoint.y : midPoint.y);
                childBound.minPoint.z = ((child & 1) ? 
                    midPoint.z : nodeBound.minPoint.z);
                childBound.maxPoint.z = ((child & 1) ? 
                    nodeBound.maxPoint.z : midPoint.z);
                lookupPrivate(node->children[child], childBound, p);
            }
        }
    }
    else
    {
        processData(node->value, p);
    }
}

void ScatterOctree::processData(const NodeData& sample,
                                const Point3& p)
{
    float r = (sample.position - p).GetLength();

    float dr = sqrt(m_bssrdf->zr * m_bssrdf->zr + r * r);
    float dv = sqrt(m_bssrdf->zv * m_bssrdf->zv + r * r);

    Color rd1 = m_bssrdf->zr * (1 + m_bssrdf->sigtr * dr) 
        * m_bssrdf->sigtr.exp(-dr) / (dr * dr * dr);
    Color rd2 = m_bssrdf->zv * (1 + m_bssrdf->sigtr * dv) 
        * m_bssrdf->sigtr.exp(-dv) / (dv * dv * dv);

    Color rd = rd1 + rd2;

    m_resultE += rd * sample.irradiance * sample.area;
}
