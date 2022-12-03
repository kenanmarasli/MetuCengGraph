//
// A simple PLY parser by Ahmet Oguz Akyuz. No guarantees are
// given that it will parse all PLY files. But it seems to work
// for the PLY models that we use in the context of CENG795
// class at METU. Use it at your own risk and include this
// description if you distribute it.
//
// Contact: akyuz@ceng.metu.edu.tr
//

#ifndef __PARSEPLY_H__
#define __PARSEPLY_H__

#include <vector>

namespace ply {

struct Vector3 {
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    float x, y, z;
};

struct Vector2 {
    Vector2(float x, float y) : x(x), y(y) {}
    float x, y;
};

struct PlyMesh {
    std::vector<Vector3> vertices;  /// Vector of vertex positions
    std::vector<Vector3> normals;   /// Vector of vertex normals
    std::vector<Vector2> texCoords; /// Vector of vertex texture coordinates
    std::vector<int> indices; /// Vector of indices into the vertices vector
    std::vector<unsigned char>
        nIndexPerFace; /// Number of indices per each face. Its size is equal to
                       /// the face count
};

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
              PlyMesh &plyData      ///< [out] Ply Mesh
);

} // namespace ply

#endif // __PARSEPLY_H__
