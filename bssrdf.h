#pragma once

#include "color.h"

struct Bssrdf
{
public:
    // diffuse params
    float specluar;
    Color diffuse;

    // translucent params
    Color scatter;
    Color absorb;
    float eta;
    float g;

    // computed
    Color sigt;
    Color sigsp;
    Color sigtp;
    Color sigtr;
    float zr;
    float fdr;
    float zv;
    
    void UpdateParams()
    {
        sigt = scatter + absorb;
        sigsp = (1 - g) * scatter;
        sigtp = sigsp + absorb;
        sigtr = (3 * absorb * sigtp).sqrt();
        zr = 1.0f / sigtp.meanValue();
        fdr = -1.440f / (eta * eta) + 0.710f / eta + 0.668f + 0.0636f * eta;
        zv = zr * (1 + 4 / 3.0f * (1 + fdr) / (1 - fdr));
    }
};
