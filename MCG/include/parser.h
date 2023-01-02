#ifndef MCG_PARSER_H
#define MCG_PARSER_H

#include "parsePly.h"
#include <string>
#include <vector>

namespace MCG {
// Notice that all the structures are as simple as possible
// so that you are not enforced to adopt any style or design.
struct Vec2f {
    float x, y;
};

struct Vec3f {
    float x, y, z;
};

struct Vec3i {
    int x, y, z;
};

struct Vec4f {
    float x, y, z, w;
};

enum class CameraType { Default, LookAt };

struct Camera {
    CameraType type;
    Vec3f position;
    Vec3f gaze;
    Vec3f up;
    Vec3f gazePoint;
    float fovY;
    Vec4f near_plane;
    float near_distance;
    int image_width, image_height;
    std::string image_name;
    int number_of_samples;
    bool has_depth_of_field{false};
    float aperture_size;
    float focus_distance;
};

struct PointLight {
    Vec3f position;
    Vec3f intensity;
};

struct AreaLight {
    Vec3f position;
    Vec3f normal;
    Vec3f radiance;
    float extent;
};

struct DirectionalLight {
    Vec3f direction;
    Vec3f radiance;
};

struct SpotLight {
    Vec3f position;
    Vec3f direction;
    Vec3f intensity;
    float coverage;
    float falloff;
};

enum class MaterialType { Default, Conductor, Dielectric, Mirror };

struct Material {
    MaterialType type;
    Vec3f ambient;
    Vec3f diffuse;
    Vec3f specular;
    Vec3f mirror;
    float phong_exponent;
    Vec3f absorption_coefficient;
    float refraction_index;
    float absorption_index;
    float roughness;
};

struct Vertex {
    Vec3f position;
    Vec2f uv;
};

struct Face {
    int v0_id;
    int v1_id;
    int v2_id;
};

enum class ShadingMode { Flat, Smooth };

struct Mesh {
    int id;
    int material_id;
    std::vector<Face> faces;
    std::vector<int> transformation_indexes;
    bool has_motion{false};
    Vec3f motion;
    std::vector<int> texture_ids;
    ShadingMode shading_mode{ShadingMode::Flat};
};

struct PlyMesh {
    int id;
    int material_id;
    ply::PlyMesh mesh;
    std::vector<int> transformation_indexes;
    bool has_motion{false};
    Vec3f motion;
    std::vector<int> texture_ids;
};

struct MeshInstance {
    int id;
    int base_mesh_id; // mesh, ply mesh or mesh instance
    bool overrides_material;
    int material_id;
    bool resets_transform;
    std::vector<int> transformation_indexes;
    bool has_motion{false};
    Vec3f motion;
    std::vector<int> texture_ids;
};

enum class TransformationType { Translation, Rotation, Scaling, Composite };

struct Transformation {
    TransformationType type;
    float x;
    float y;
    float z;
    float theta;
    float data[4][4];
};

struct Image {
    std::string path;
};

enum class DecalMode {
    ReplaceKD,
    BlendKD,
    ReplaceKS,
    ReplaceBackground,
    ReplaceNormal,
    BumpNormal,
    ReplaceAll
};
enum class Interpolation { NearestNeighbour, Bilinear };
enum class TextureType { Image, Perlin, Checker };
enum class NoiseConversion { Abs, Linear };

struct TextureMap {
    int image_id;
    DecalMode decal_mode;
    Interpolation interpolation;
    TextureType type;
    NoiseConversion noise_conversion;
    int normalizer;
    float scale;
    float offset;
    Vec3f black;
    Vec3f white;
    float bump_factor;
};

struct Triangle {
    int material_id;
    Face indices;
    std::vector<int> transformation_indexes;
    bool has_motion{false};
    Vec3f motion;
    std::vector<int> texture_ids;
};

struct Sphere {
    int material_id;
    int center_vertex_id;
    float radius;
    std::vector<int> transformation_indexes;
    bool has_motion{false};
    Vec3f motion;
    std::vector<int> texture_ids;
};

struct Scene {
    // Data
    Vec3i background_color;
    float shadow_ray_epsilon;
    int max_recursion_depth;
    std::vector<Camera> cameras;
    Vec3f ambient_light;
    std::vector<PointLight> point_lights;
    std::vector<AreaLight> area_lights;
    std::vector<DirectionalLight> directional_lights;
    std::vector<SpotLight> spot_lights;
    std::vector<Material> materials;
    std::vector<Vertex> vertex_data;
    std::vector<Vec2f> texture_coordinates;
    std::vector<Mesh> meshes;
    std::vector<Triangle> triangles;
    std::vector<Sphere> spheres;
    std::vector<PlyMesh> plyMeshes;
    std::vector<MeshInstance> meshInstances;
    std::vector<Transformation> transformations;
    std::vector<Image> images;
    std::vector<TextureMap> textureMaps;

    // Functions
    void loadFromXml(const std::string &filepath);
};
} // namespace MCG

#endif
