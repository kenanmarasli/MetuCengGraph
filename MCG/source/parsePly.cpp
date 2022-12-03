//
// A simple PLY parser by Ahmet Oguz Akyuz. No guarantees are
// given that it will parse all PLY files. But it seems to work
// for the PLY models that we use in the context of CENG795
// class at METU. Use it at your own risk and include this
// description if you distribute it.
//
// Contact: akyuz@ceng.metu.edu.tr
//

#include "parsePly.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

namespace ply {

void parseVertexProperties(std::ifstream &input, int &normalCount,
                           int &uvCount) {
    std::string line, word1, word2, word3;

    int i = 0;

    while (true) {
        int len = input.tellg(); // how many bytes we read so far

        std::getline(input, line);

        std::stringstream stream(line);

        stream >> word1 >> word2 >> word3;

        if (word1 == "property") {
            if (word2 == "float") {
                if (word3 == "x" || word3 == "y" || word3 == "z") {
                    ++i;
                } else if (word3 == "nx" || word3 == "ny" || word3 == "nz") {
                    normalCount++;
                } else if (word3 == "u" || word3 == "v") {
                    uvCount++;
                }
            } else {
                assert(!"Unknown vertex data type");
            }
        } else {
            input.seekg(
                len,
                std::ios_base::beg); // return back to before the last getline
            break;
        }
    }

    //
    // The number of vertex properties must be equal to three
    //
    assert(i == 3);
}

void parseFaceProperties(std::ifstream &input, std::string &indexCountType,
                         std::string &indexType, int &skipBytes) {
    std::string line, word1, word2;

    while (true) {
        int len = input.tellg(); // how many bytes we read so far

        std::getline(input, line);

        std::stringstream stream(line);

        stream >> word1 >> word2;

        if (word1 == "property") {
            if (word2 == "list") {
                stream >> indexCountType;
                stream >> indexType;

                assert(indexCountType == "uchar" || indexCountType == "uint8");
                assert(indexType == "int" || indexType == "uint");
            } else if (word2 == "float") {
                //
                // We do not recognize other properties but we must know
                // how many bytes to skip, when parsing a binary file
                //
                skipBytes += 4;
            } else {
                assert(!"Unknown word after property");
            }
        } else {
            input.seekg(
                len,
                std::ios_base::beg); // return back to before the last getline
            break;
        }
    }
}

/**
***************************************************************************************
* parsePly
*
* @brief
*     Extracts the vertex and index data without actually constructing the faces
*
* @return
*	  true if successful, false otherwise
**************************************************************************************
*/
bool parsePly(const char *filename, ///< [in]  Name of the PLY file
              PlyMesh &plyMesh      ///< [out] Ply Mesh
) {
    std::ifstream input(filename, std::ios::binary);

    bool isBinary = false;
    int skipBytes = 0, normalCount = 0, uvCount = 0;
    int numVertices, numFaces;
    std::string line, word1, word2;
    std::string indexCountType, indexType;

    if (input.is_open()) {
        while (true) {
            std::getline(input, line);

            std::stringstream stream(line);
            stream >> word1;

            if (word1 == "element") {
                stream >> word2;

                if (word2 == "vertex") {
                    stream >> numVertices;
                    parseVertexProperties(input, normalCount, uvCount);
                } else if (word2 == "face") {
                    stream >> numFaces;
                    parseFaceProperties(input, indexCountType, indexType,
                                        skipBytes);
                }
            } else if (word1 == "format") {
                stream >> word2;

                assert(word2 == "ascii" || word2 == "binary_little_endian");

                if (word2 == "binary_little_endian") {
                    isBinary = true;
                }
            } else if (word1 == "end_header") {
                break;
            }
        }
    }

    //
    // We have parsed the header. The rest depends on whether the
    // file is binary or ascii.
    //
    int c, index;
    unsigned char cBin;
    float x, y, z;
    float v[3];

    char tmp[64]; // we will store unsupported properties here
    assert(skipBytes < 64);
    assert(isBinary ||
           skipBytes == 0); // no support for skipping in a non-binary file
    assert(normalCount == 0 || normalCount == 3);
    assert(uvCount == 0 || uvCount == 2);

    if (isBinary) {
        for (int i = 0; i < numVertices; ++i) {
            input.read((char *)v, sizeof(float) * 3);
            plyMesh.vertices.push_back(Vector3(v[0], v[1], v[2]));

            if (normalCount > 0) {
                input.read((char *)v, sizeof(float) * normalCount);
                plyMesh.normals.push_back(Vector3(v[0], v[1], v[2]));
            }

            if (uvCount > 0) {
                input.read((char *)v, sizeof(float) * uvCount);
                plyMesh.texCoords.push_back(Vector2(v[0], v[1]));
            }
        }

        for (int i = 0; i < numFaces; ++i) {
            input.read((char *)&cBin, sizeof(unsigned char));
            plyMesh.nIndexPerFace.push_back(cBin);

            for (int j = 0; j < cBin; ++j) {
                input.read((char *)&index, sizeof(int));
                plyMesh.indices.push_back(index);
            }

            input.read(tmp, skipBytes);
        }
    } else {
        for (int i = 0; i < numVertices; ++i) {
            input >> x >> y >> z;
            plyMesh.vertices.push_back(Vector3(x, y, z));

            if (normalCount > 0) {
                input >> x >> y >> z;
                plyMesh.normals.push_back(Vector3(x, y, z));
            }

            if (uvCount > 0) {
                input >> x >> y;
                plyMesh.texCoords.push_back(Vector2(x, y));
            }
        }

        for (int i = 0; i < numFaces; ++i) {
            input >> c;
            assert(c >= 0 && c <= 255);
            plyMesh.nIndexPerFace.push_back(c);

            for (int j = 0; j < c; ++j) {
                input >> index;
                plyMesh.indices.push_back(index);
            }
        }
    }

    input.close();

    return true;
}

} // namespace ply
