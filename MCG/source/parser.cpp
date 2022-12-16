#include "parser.h"
#include "tinyxml2.h"
#include <cstring>
#include <filesystem>
#include <sstream>
#include <stdexcept>

template <typename TObject>
auto get_transformations(TObject &object, const MCG::Scene &scene,
                         const char *transformationText) -> void {
    char s[256];
    strcpy(s, transformationText);
    char *transformation = strtok(s, " ");
    while (transformation) {
        int transformationIndex{atoi(&transformation[1])};
        MCG::TransformationType transformationType;
        switch (transformation[0]) {
        case 't':
            transformationType = MCG::TransformationType::Translation;
            break;
        case 'r':
            transformationType = MCG::TransformationType::Rotation;
            break;
        case 's':
            transformationType = MCG::TransformationType::Scaling;
            break;
        }
        for (int i = 0, typeIndex = 1; i < scene.transformations.size(); i++) {
            const MCG::Transformation &t = scene.transformations[i];
            if (t.type == transformationType) {
                if (typeIndex == transformationIndex) {
                    transformationIndex = i;
                    break;
                }
                ++typeIndex;
            }
        }
        object.transformation_indexes.push_back(transformationIndex);
        transformation = strtok(NULL, " ");
    }
}

void MCG::Scene::loadFromXml(const std::string &filepath) {
    tinyxml2::XMLDocument file;
    std::stringstream stream;

    auto res = file.LoadFile(filepath.c_str());
    if (res) {
        throw std::runtime_error("Error: The xml file cannot be loaded.");
    }

    auto root = file.FirstChild();
    if (!root) {
        throw std::runtime_error("Error: Root is not found.");
    }

    // Get BackgroundColor
    auto element = root->FirstChildElement("BackgroundColor");
    if (element) {
        stream << element->GetText() << std::endl;
    } else {
        stream << "0 0 0" << std::endl;
    }
    stream >> background_color.x >> background_color.y >> background_color.z;

    // Get ShadowRayEpsilon
    element = root->FirstChildElement("ShadowRayEpsilon");
    if (element) {
        stream << element->GetText() << std::endl;
    } else {
        stream << "0.001" << std::endl;
    }
    stream >> shadow_ray_epsilon;

    // Get MaxRecursionDepth
    element = root->FirstChildElement("MaxRecursionDepth");
    if (element) {
        stream << element->GetText() << std::endl;
    } else {
        stream << "0" << std::endl;
    }
    stream >> max_recursion_depth;

    // Get Cameras
    element = root->FirstChildElement("Cameras");
    element = element->FirstChildElement("Camera");
    Camera camera;
    while (element) {
        tinyxml2::XMLElement *child;
        bool isLookAt = (element->Attribute("type", "lookAt") != NULL);
        if (isLookAt) {
            camera.type = CameraType::LookAt;
            child = element->FirstChildElement("GazePoint");
            stream << child->GetText() << std::endl;
            child = element->FirstChildElement("FovY");
            stream << child->GetText() << std::endl;

            stream >> camera.gazePoint.x >> camera.gazePoint.y >>
                camera.gazePoint.z;
            stream >> camera.fovY;
        } else {
            camera.type = CameraType::Default;
            child = element->FirstChildElement("Gaze");
            stream << child->GetText() << std::endl;
            child = element->FirstChildElement("NearPlane");
            stream << child->GetText() << std::endl;

            stream >> camera.gaze.x >> camera.gaze.y >> camera.gaze.z;
            stream >> camera.near_plane.x >> camera.near_plane.y >>
                camera.near_plane.z >> camera.near_plane.w;
        }

        child = element->FirstChildElement("Position");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("Up");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("NearDistance");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("ImageResolution");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("ImageName");
        stream << child->GetText() << std::endl;

        stream >> camera.position.x >> camera.position.y >> camera.position.z;
        stream >> camera.up.x >> camera.up.y >> camera.up.z;
        stream >> camera.near_distance;
        stream >> camera.image_width >> camera.image_height;
        stream >> camera.image_name;

        child = element->FirstChildElement("NumSamples");
        if (child) {
            stream << child->GetText() << std::endl;
            stream >> camera.number_of_samples;
        } else {
            camera.number_of_samples = 1;
        }

        child = element->FirstChildElement("ApertureSize");
        if (child) {
            stream << child->GetText() << std::endl;
            stream >> camera.aperture_size;
            camera.has_depth_of_field = true;
        } else {
            camera.has_depth_of_field = false;
        }

        child = element->FirstChildElement("FocusDistance");
        if (child) {
            stream << child->GetText() << std::endl;
            stream >> camera.focus_distance;
            camera.has_depth_of_field = true;
        } else {
            camera.has_depth_of_field = false;
        }

        cameras.push_back(camera);
        element = element->NextSiblingElement("Camera");
    }

    // Get Lights
    element = root->FirstChildElement("Lights");
    auto child = element->FirstChildElement("AmbientLight");
    stream << child->GetText() << std::endl;
    stream >> ambient_light.x >> ambient_light.y >> ambient_light.z;
    element = element->FirstChildElement("PointLight");
    PointLight point_light;
    while (element) {
        child = element->FirstChildElement("Position");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("Intensity");
        stream << child->GetText() << std::endl;

        stream >> point_light.position.x >> point_light.position.y >>
            point_light.position.z;
        stream >> point_light.intensity.x >> point_light.intensity.y >>
            point_light.intensity.z;

        point_lights.push_back(point_light);
        element = element->NextSiblingElement("PointLight");
    }
    stream.clear();
    element = root->FirstChildElement("Lights");
    element = element->FirstChildElement("AreaLight");
    while (element) {
        child = element->FirstChildElement("Position");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("Radiance");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("Normal");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("Size");
        stream << child->GetText() << std::endl;

        AreaLight area_light;
        stream >> area_light.position.x >> area_light.position.y >>
            area_light.position.z;
        stream >> area_light.radiance.x >> area_light.radiance.y >>
            area_light.radiance.z;
        stream >> area_light.normal.x >> area_light.normal.y >>
            area_light.normal.z;
        stream >> area_light.extent;

        area_lights.push_back(area_light);
        element = element->NextSiblingElement("AreaLight");
    }
    stream.clear();

    // Get Materials
    element = root->FirstChildElement("Materials");
    element = element->FirstChildElement("Material");
    Material material;
    while (element) {
        material.type = MaterialType::Default;
        const char *matTypeAttr = element->Attribute("type");
        if (matTypeAttr) {
            if (strcmp(matTypeAttr, "conductor") == 0) {
                material.type = MaterialType::Conductor;
            } else if (strcmp(matTypeAttr, "dielectric") == 0) {
                material.type = MaterialType::Dielectric;
            } else if (strcmp(matTypeAttr, "mirror") == 0) {
                material.type = MaterialType::Mirror;
            }
        }
        child = element->FirstChildElement("AmbientReflectance");
        if (child) {
            stream << child->GetText() << std::endl;
        } else {
            stream << "0 0 0" << std::endl;
        }
        child = element->FirstChildElement("DiffuseReflectance");
        if (child) {
            stream << child->GetText() << std::endl;
        } else {
            stream << "0 0 0" << std::endl;
        }
        child = element->FirstChildElement("SpecularReflectance");
        if (child) {
            stream << child->GetText() << std::endl;
        } else {
            stream << "0 0 0" << std::endl;
        }
        child = element->FirstChildElement("MirrorReflectance");
        if (child) {
            stream << child->GetText() << std::endl;
        } else {
            stream << "0 0 0" << std::endl;
        }
        child = element->FirstChildElement("PhongExponent");
        if (child) {
            stream << child->GetText() << std::endl;
        } else {
            stream << "0.1" << std::endl;
        }
        child = element->FirstChildElement("AbsorptionCoefficient");
        if (child) {
            stream << child->GetText() << std::endl;
        } else {
            stream << "0.01 0.01 0.01" << std::endl;
        }
        child = element->FirstChildElement("RefractionIndex");
        if (child) {
            stream << child->GetText() << std::endl;
        } else {
            stream << "1" << std::endl;
        }
        child = element->FirstChildElement("AbsorptionIndex");
        if (child) {
            stream << child->GetText() << std::endl;
        } else {
            stream << "1" << std::endl;
        }

        stream >> material.ambient.x >> material.ambient.y >>
            material.ambient.z;
        stream >> material.diffuse.x >> material.diffuse.y >>
            material.diffuse.z;
        stream >> material.specular.x >> material.specular.y >>
            material.specular.z;
        stream >> material.mirror.x >> material.mirror.y >> material.mirror.z;
        stream >> material.phong_exponent;
        stream >> material.absorption_coefficient.x >>
            material.absorption_coefficient.y >>
            material.absorption_coefficient.z;
        stream >> material.refraction_index;
        stream >> material.absorption_index;

        materials.push_back(material);
        element = element->NextSiblingElement("Material");
    }
    stream.clear();

    // Get Transformations
    element = root->FirstChildElement("Transformations");
    if (element) {
        element = element->FirstChildElement("Translation");
        while (element) {
            Transformation transformation;
            stream << element->GetText() << std::endl;
            transformation.type = TransformationType::Translation;
            stream >> transformation.x >> transformation.y >> transformation.z;
            transformations.push_back(transformation);
            element = element->NextSiblingElement("Translation");
        }
        element = root->FirstChildElement("Transformations");
        element = element->FirstChildElement("Rotation");
        while (element) {
            Transformation transformation;
            stream << element->GetText() << std::endl;
            transformation.type = TransformationType::Rotation;
            stream >> transformation.theta >> transformation.x >>
                transformation.y >> transformation.z;
            transformations.push_back(transformation);
            element = element->NextSiblingElement("Rotation");
        }
        element = root->FirstChildElement("Transformations");
        element = element->FirstChildElement("Scaling");
        while (element) {
            Transformation transformation;
            stream << element->GetText() << std::endl;
            transformation.type = TransformationType::Scaling;
            stream >> transformation.x >> transformation.y >> transformation.z;
            transformations.push_back(transformation);
            element = element->NextSiblingElement("Scaling");
        }
        stream.clear();
    }

    // Get VertexData
    element = root->FirstChildElement("VertexData");
    stream << element->GetText() << std::endl;
    Vec3f vertex;
    while (!(stream >> vertex.x).eof()) {
        stream >> vertex.y >> vertex.z;
        vertex_data.push_back(vertex);
    }
    stream.clear();

    // Get Meshes
    element = root->FirstChildElement("Objects");
    element = element->FirstChildElement("Mesh");
    while (element) {
        int meshID{};
        const char *meshIDText = element->Attribute("id");
        if (meshIDText) {
            stream << meshIDText << std::endl;
            stream >> meshID;
        }
        child = element->FirstChildElement("Material");
        stream << child->GetText() << std::endl;

        child = element->FirstChildElement("Faces");
        const char *plyFilename = child->Attribute("plyFile");
        if (plyFilename) {
            std::filesystem::path plyPath{plyFilename};
            if (plyPath.is_relative()) {
                std::filesystem::path path{filepath};
                path.replace_filename(plyPath);
                plyPath = path;
            }
            PlyMesh plyMesh;
            plyMesh.id = meshID;
            stream >> plyMesh.material_id;
            ply::parsePly(plyPath.string().c_str(), plyMesh.mesh);
            child = element->FirstChildElement("Transformations");
            if (child) {
                const char *transformationText = child->GetText();
                get_transformations(plyMesh, *this, transformationText);
            }
            child = element->FirstChildElement("MotionBlur");
            if (child) {
                stream << child->GetText() << std::endl;
                stream >> plyMesh.motion.x >> plyMesh.motion.y >>
                    plyMesh.motion.z;
                plyMesh.has_motion = true;
            }
            plyMeshes.push_back(plyMesh);
            plyMesh.transformation_indexes.clear();
        } else {
            Mesh mesh;
            mesh.id = meshID;
            stream >> mesh.material_id;
            stream << child->GetText() << std::endl;
            Face face;
            while (!(stream >> face.v0_id).eof()) {
                stream >> face.v1_id >> face.v2_id;
                mesh.faces.push_back(face);
            }
            child = element->FirstChildElement("Transformations");
            if (child) {
                const char *transformationText = child->GetText();
                get_transformations(mesh, *this, transformationText);
            }
            child = element->FirstChildElement("MotionBlur");
            if (child) {
                stream << child->GetText() << std::endl;
                stream >> mesh.motion.x >> mesh.motion.y >> mesh.motion.z;
                mesh.has_motion = true;
            }
            meshes.push_back(mesh);
            mesh.faces.clear();
            mesh.transformation_indexes.clear();
        }
        stream.clear();
        element = element->NextSiblingElement("Mesh");
    }
    stream.clear();

    // Get Mesh Instances
    element = root->FirstChildElement("Objects");
    element = element->FirstChildElement("MeshInstance");
    while (element) {
        MeshInstance meshInstance;
        int meshInstanceID{};
        const char *meshInstanceIDText = element->Attribute("id");
        if (meshInstanceIDText) {
            stream << meshInstanceIDText << std::endl;
            stream >> meshInstanceID;
        }
        meshInstance.id = meshInstanceID;
        const char *baseMeshID = element->Attribute("baseMeshId");
        stream << baseMeshID << std::endl;
        stream >> meshInstance.base_mesh_id;
        meshInstance.resets_transform = false;
        const char *resetTransformText = element->Attribute("resetTransform");
        if (resetTransformText) {
            meshInstance.resets_transform =
                strcmp(resetTransformText, "true") == 0;
        }
        child = element->FirstChildElement("Material");
        if (child) {
            stream << child->GetText() << std::endl;
            stream >> meshInstance.material_id;
            meshInstance.overrides_material = true;
        } else {
            meshInstance.overrides_material = false;
        }
        child = element->FirstChildElement("Transformations");
        if (child) {
            const char *transformationText = child->GetText();
            get_transformations(meshInstance, *this, transformationText);
        }
        child = element->FirstChildElement("MotionBlur");
        if (child) {
            stream << child->GetText() << std::endl;
            stream >> meshInstance.motion.x >> meshInstance.motion.y >>
                meshInstance.motion.z;
            meshInstance.has_motion = true;
        }
        meshInstances.push_back(meshInstance);
        meshInstance.transformation_indexes.clear();

        element = element->NextSiblingElement("MeshInstance");
    }
    stream.clear();

    // Get Triangles
    element = root->FirstChildElement("Objects");
    element = element->FirstChildElement("Triangle");
    Triangle triangle;
    while (element) {
        child = element->FirstChildElement("Material");
        stream << child->GetText() << std::endl;
        stream >> triangle.material_id;

        child = element->FirstChildElement("Indices");
        stream << child->GetText() << std::endl;
        stream >> triangle.indices.v0_id >> triangle.indices.v1_id >>
            triangle.indices.v2_id;

        child = element->FirstChildElement("Transformations");
        if (child) {
            const char *transformationText = child->GetText();
            get_transformations(triangle, *this, transformationText);
        }
        child = element->FirstChildElement("MotionBlur");
        if (child) {
            stream << child->GetText() << std::endl;
            stream >> triangle.motion.x >> triangle.motion.y >>
                triangle.motion.z;
            triangle.has_motion = true;
        }
        triangles.push_back(triangle);
        triangle.transformation_indexes.clear();
        element = element->NextSiblingElement("Triangle");
    }

    // Get Spheres
    element = root->FirstChildElement("Objects");
    element = element->FirstChildElement("Sphere");
    Sphere sphere;
    while (element) {
        child = element->FirstChildElement("Material");
        stream << child->GetText() << std::endl;
        stream >> sphere.material_id;

        child = element->FirstChildElement("Center");
        stream << child->GetText() << std::endl;
        stream >> sphere.center_vertex_id;

        child = element->FirstChildElement("Radius");
        stream << child->GetText() << std::endl;
        stream >> sphere.radius;

        child = element->FirstChildElement("Transformations");
        if (child) {
            const char *transformationText = child->GetText();
            get_transformations(sphere, *this, transformationText);
        }
        child = element->FirstChildElement("MotionBlur");
        if (child) {
            stream << child->GetText() << std::endl;
            stream >> sphere.motion.x >> sphere.motion.y >> sphere.motion.z;
            sphere.has_motion = true;
        }
        spheres.push_back(sphere);
        sphere.transformation_indexes.clear();
        element = element->NextSiblingElement("Sphere");
    }
}
