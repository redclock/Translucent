//
// THU-Render
//
// Copyright 2007-2008 CG & CAD Institute, Tsinghua University.
// All rights reserved.
//

#pragma once

#include <string>
#include <vector>
#include <fstream>

/**
 * Reads text formatted mesh in default format.
 */
class DefaultTextReader
{
// public constructors & destructor
public:
    DefaultTextReader(void);
    ~DefaultTextReader(void);

// public member functions
public:
    virtual bool readFile(const char* filename);
    double* vertices;
    double* normals;
    size_t* vertexIndices;
    double* tex;
    size_t vertexCount;
    size_t faceCount;
    std::ifstream* f;
    bool inversed;

// private member functions
private:
    bool readData(void);
    // Reads a token from the input stream.
    // Returns the next token, or an empty string if reaches EOF.
    std::string readToken(void);
    // Reads a series of size_ts into the data.
    // Returns true when succeeded or false if error occurred.
    bool readIntegers(size_t* data, size_t count);
    // Reads a series of doubles into the data.
    // Returns true when succeeded or false if error occurred.
    bool readDoubles(double* data, size_t count);
};

