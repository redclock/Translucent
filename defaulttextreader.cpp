//
// THU-Render
//
// Copyright 2007-2008 CG & CAD Institute, Tsinghua University.
// All rights reserved.
//
#include "defaulttextreader.h"
// C++ headers
#include <cstdlib>
#include <cassert>
#include <iostream>
#include <fstream>

DefaultTextReader::DefaultTextReader(void) 
    : vertices(NULL), normals(NULL), tex(NULL), vertexIndices(NULL),
    vertexCount(0), faceCount(0), inversed(false)
{
}

DefaultTextReader::~DefaultTextReader(void)
{
}

bool DefaultTextReader::readFile(const char* filename)
{
    std::ifstream ff;
    ff.open(filename);
    if (!ff.is_open())
        return false;
    f = &ff;
    // Token:
    // 0 - initial
    // 1 - vertex count
    // 2 - face count
    // 3 - vertices
    // 4 - vertex indices
    // 5 - normals
    // 6 - texture coordinates
    // 7 - end of file
    int state = 0;
    vertexCount = 0;
    faceCount = 0;
    bool facesRead = false;
    bool verticesRead = false;

    while (state != 7)
    {
        switch (state)
        {
        case 0: // initial
            {
                std::string token = readToken();
                if (token == "vertexcount")
                {
                    state = 1;
                }
                else if (token == "facecount")
                {
                    state = 2;
                }
                else if (token == "vertices")
                {
                    state = 3;
                }
                else if (token == "vertexindices")
                {
                    state = 4;
                }
                else if (token == "normals")
                {
                    normals = new double[vertexCount * 3];
                    state = 5;
                }
                else if (token == "texturecoords")
                {
                    tex = new double[vertexCount * 2];
                    state = 6;
                }
                else if (token == "inversed")
                {
                    inversed = true;
                }
                else if (token == "")
                {
                    state = 7;
                }
                else
                {
                    return false;
                }
            }
            break;
        case 1: // vertex count
            {
                std::string number = readToken();
                if (number.empty())
                    return false;

                vertexCount = atoi(number.c_str());
                if (vertexCount == 0)
                    return false;
                vertices = new double[vertexCount * 3];
                state = 0;
            }
            break;
        case 2: // face count
            {
                std::string number = readToken();
                if (number.empty())
                    return false;

                faceCount = atoi(number.c_str());
                if (faceCount == 0)
                    return false;
                vertexIndices = new size_t[faceCount * 3];
                state = 0;
            }
            state = 0;
            break;
        case 3: // vertices
            if (!readDoubles(vertices, vertexCount * 3))
                return false;
            verticesRead = true;
            state = 0;
            break;
        case 4: // vertex indices
            if (!readIntegers(vertexIndices, faceCount * 3))
                return false;
            facesRead = true;
            state = 0;
            break;
        case 5: // normals
            if (!readDoubles(normals, vertexCount * 3))
                return false;
            state = 0;
            break;
        case 6: // texture coordinates
            if (!readDoubles(tex, vertexCount * 2))
                return false;
            state = 0;
            break;
        case 7: // end of file
            break;
        default:
            assert(false);
        }
    }

    return verticesRead && facesRead;
}

std::string DefaultTextReader::readToken(void)
{
    std::string result;
    int data = f->peek();
    bool isSpace = true;
    while (data >= 0 && data != '>')
    {
        if (isSpace)
        {
            switch (data)
            {
            case ' ':
            case '\r':
            case '\n':
            case '\t':
            case ':':
            case ',':
                break;
            default:
                isSpace = false;
                result += static_cast<char>(data);
            }
        }
        else
        {
            switch (data)
            {
            case ' ':
            case '\r':
            case '\n':
            case '\t':
            case ':':
            case ',':
                return result;
                break;
            default:
                isSpace = false;
                result += static_cast<char>(data);
            }
        }
        char cc;
        f->read(&cc, 1);
        data = f->peek();
    }
    return result;
}

bool DefaultTextReader::readIntegers(size_t* data, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        std::string token = readToken();
        if (token.empty())
            return false;
        data[i] = atoi(token.c_str());
    }
    return true;
}

bool DefaultTextReader::readDoubles(double* data, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        std::string token = readToken();
        if (token.empty())
            return false;
        data[i] = atof(token.c_str());
    }
    return true;
}
