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
        case 'c':
            transformationType = MCG::TransformationType::Composite;
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

        child = element->FirstChildElement("Tonemap");
        if (child) {
            auto tmoElem = child->FirstChildElement("TMO");
            std::string tmoStr{tmoElem->GetText()};
            if (tmoStr == "Photographic") {
                camera.tmo = ToneMappingOperator::Photographic;
            }
            tmoElem = child->FirstChildElement("TMOOptions");
            stream << tmoElem->GetText() << std::endl;
            stream >> camera.tmo_key_value >> camera.burn_percent;
            tmoElem = child->FirstChildElement("Saturation");
            stream << tmoElem->GetText() << std::endl;
            stream >> camera.saturation;
            tmoElem = child->FirstChildElement("Gamma");
            stream << tmoElem->GetText() << std::endl;
            stream >> camera.gamma;
        }

        cameras.push_back(camera);
        element = element->NextSiblingElement("Camera");
    }

    // Get Lights
    element = root->FirstChildElement("Lights");
    // Ambient Light
    auto child = element->FirstChildElement("AmbientLight");
    if (child) {
        stream << child->GetText() << std::endl;
        stream >> ambient_light.x >> ambient_light.y >> ambient_light.z;
    }
    // Point Lights
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
    // Area Lights
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
    // Directional Lights
    element = root->FirstChildElement("Lights");
    element = element->FirstChildElement("DirectionalLight");
    while (element) {
        child = element->FirstChildElement("Direction");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("Radiance");
        stream << child->GetText() << std::endl;

        DirectionalLight directionalLight;
        stream >> directionalLight.direction.x >>
            directionalLight.direction.y >> directionalLight.direction.z;
        stream >> directionalLight.radiance.x >> directionalLight.radiance.y >>
            directionalLight.radiance.z;

        directional_lights.push_back(directionalLight);
        element = element->NextSiblingElement("DirectionalLight");
    }
    stream.clear();
    // Directional Lights
    element = root->FirstChildElement("Lights");
    element = element->FirstChildElement("SpotLight");
    while (element) {
        child = element->FirstChildElement("Position");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("Direction");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("Intensity");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("CoverageAngle");
        stream << child->GetText() << std::endl;
        child = element->FirstChildElement("FalloffAngle");
        stream << child->GetText() << std::endl;

        SpotLight spotLight;
        stream >> spotLight.position.x >> spotLight.position.y >>
            spotLight.position.z;
        stream >> spotLight.direction.x >> spotLight.direction.y >>
            spotLight.direction.z;
        stream >> spotLight.intensity.x >> spotLight.intensity.y >>
            spotLight.intensity.z;
        stream >> spotLight.coverage;
        stream >> spotLight.falloff;

        spot_lights.push_back(spotLight);
        element = element->NextSiblingElement("SpotLight");
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
        material.degamma = false;
        const char *degammaAttr = element->Attribute("degamma");
        if (degammaAttr) {
            std::string degammaStr{degammaAttr};
            if (degammaStr == "true") {
                material.degamma = true;
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
        child = element->FirstChildElement("Roughness");
        if (child) {
            stream << child->GetText() << std::endl;
        } else {
            stream << "0" << std::endl;
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
        stream >> material.roughness;

        materials.push_back(material);
        element = element->NextSiblingElement("Material");
    }
    stream.clear();

    // Get Images
    element = root->FirstChildElement("Textures");
    if (element) {
        element = element->FirstChildElement("Images");
        if (element) {
            element = element->FirstChildElement("Image");
        }
    }
    while (element) {
        Image image;
        std::filesystem::path imagePath{element->GetText()};
        if (imagePath.is_relative()) {
            std::filesystem::path path{filepath};
            path.replace_filename(imagePath);
            imagePath = path;
        }
        image.path = imagePath.string();
        images.push_back(image);
        element = element->NextSiblingElement("Image");
    }
    stream.clear();

    // Get Texture Maps
    element = root->FirstChildElement("Textures");
    if (element) {
        element = element->FirstChildElement("TextureMap");
    }
    while (element) {
        TextureMap textureMap;
        const char *textureTypeText = element->Attribute("type");
        if (!textureTypeText) {
            textureMap.type = TextureType::Image;
        } else {
            std::string textureType{textureTypeText};
            if (textureType == "perlin") {
                textureMap.type = TextureType::Perlin;
            } else if (textureType == "checkerboard") {
                textureMap.type = TextureType::Checker;
            } else {
                textureMap.type = TextureType::Image;
            }
        }
        switch (textureMap.type) {
        case TextureType::Image: {
            child = element->FirstChildElement("ImageId");
            stream << child->GetText() << std::endl;
            stream >> textureMap.image_id;
            child = element->FirstChildElement("Interpolation");
            if (!child) {
                textureMap.interpolation = Interpolation::Bilinear;
            } else {
                std::string interpolation = {child->GetText()};
                if (interpolation == "nearest") {
                    textureMap.interpolation = Interpolation::NearestNeighbour;
                } else {
                    textureMap.interpolation = Interpolation::Bilinear;
                }
            }
            child = element->FirstChildElement("Normalizer");
            if (child) {
                stream << child->GetText() << std::endl;
                stream >> textureMap.normalizer;
            } else {
                textureMap.normalizer = 255;
            }
        } break;
        case TextureType::Perlin: {
            child = element->FirstChildElement("NoiseConversion");
            const char *noiseConversionText = child->GetText();
            std::string noiseConversion{noiseConversionText};
            if (noiseConversion == "absval") {
                textureMap.noise_conversion = NoiseConversion::Abs;
            } else {
                textureMap.noise_conversion = NoiseConversion::Linear;
            }
            child = element->FirstChildElement("NoiseScale");
            if (!child) {
                textureMap.scale = 1.F;
            } else {
                stream << child->GetText() << std::endl;
                stream >> textureMap.scale;
            }
        } break;
        case TextureType::Checker: {
            child = element->FirstChildElement("Scale");
            stream << child->GetText() << std::endl;
            stream >> textureMap.scale;
            child = element->FirstChildElement("Offset");
            stream << child->GetText() << std::endl;
            stream >> textureMap.offset;
            child = element->FirstChildElement("BlackColor");
            stream << child->GetText() << std::endl;
            stream >> textureMap.black.x >> textureMap.black.y >>
                textureMap.black.z;
            child = element->FirstChildElement("WhiteColor");
            stream << child->GetText() << std::endl;
            stream >> textureMap.white.x >> textureMap.white.y >>
                textureMap.white.z;
        } break;
        }

        child = element->FirstChildElement("DecalMode");
        std::string decalMode{child->GetText()};
        if (decalMode == "replace_kd") {
            textureMap.decal_mode = DecalMode::ReplaceKD;
        } else if (decalMode == "blend_kd") {
            textureMap.decal_mode = DecalMode::BlendKD;
        } else if (decalMode == "replace_ks") {
            textureMap.decal_mode = DecalMode::ReplaceKS;
        } else if (decalMode == "replace_background") {
            textureMap.decal_mode = DecalMode::ReplaceBackground;
        } else if (decalMode == "replace_normal") {
            textureMap.decal_mode = DecalMode::ReplaceNormal;
        } else if (decalMode == "bump_normal") {
            textureMap.decal_mode = DecalMode::BumpNormal;
        } else {
            textureMap.decal_mode = DecalMode::ReplaceAll;
        }

        child = element->FirstChildElement("BumpFactor");
        if (!child) {
            textureMap.bump_factor = 1.F;
        } else {
            stream << child->GetText() << std::endl;
            stream >> textureMap.bump_factor;
        }

        textureMaps.push_back(textureMap);
        element = element->NextSiblingElement("TextureMap");
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
        element = root->FirstChildElement("Transformations");
        element = element->FirstChildElement("Composite");
        while (element) {
            Transformation transformation;
            stream << element->GetText() << std::endl;
            transformation.type = TransformationType::Composite;
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    stream >> transformation.data[i][j];
                }
            }
            transformations.push_back(transformation);
            element = element->NextSiblingElement("Composite");
        }
        stream.clear();
    }

    // Get VertexData
    element = root->FirstChildElement("VertexData");
    if (element) {
        stream << element->GetText() << std::endl;
        Vertex vertex;
        while (!(stream >> vertex.position.x).eof()) {
            stream >> vertex.position.y >> vertex.position.z;
            vertex_data.push_back(vertex);
        }
        stream.clear();
    }

    // Get Texture Coordinates
    element = root->FirstChildElement("TexCoordData");
    if (element) {
        stream << element->GetText() << std::endl;
        Vec2f texCoord;
        while (!(stream >> texCoord.x).eof()) {
            stream >> texCoord.y;
            texture_coordinates.push_back(texCoord);
        }
        stream.clear();
    }

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
        int materialID{};
        child = element->FirstChildElement("Material");
        stream << child->GetText() << std::endl;
        stream >> materialID;

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
            plyMesh.material_id = materialID;
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
            child = element->FirstChildElement("Textures");
            if (child) {
                int texture_id;
                stream << child->GetText() << std::endl;
                while (stream >> texture_id) {
                    plyMesh.texture_ids.push_back(texture_id);
                }
                stream.clear();
            }
            plyMeshes.push_back(plyMesh);
            plyMesh.transformation_indexes.clear();
            plyMesh.texture_ids.clear();
        } else {
            Mesh mesh;
            mesh.id = meshID;
            mesh.material_id = materialID;
            const char *shadingModeText = element->Attribute("shadingMode");
            if (shadingModeText) {
                std::string shadingMode{shadingModeText};
                if (shadingMode == "smooth") {
                    mesh.shading_mode = ShadingMode::Smooth;
                }
            }
            int vertexOffset{0};
            const char *vertexOffsetText = child->Attribute("vertexOffset");
            if (vertexOffsetText) {
                stream << vertexOffsetText << std::endl;
                stream >> vertexOffset;
            }
            int textureOffset{0};
            const char *textureOffsetText = child->Attribute("textureOffset");
            if (textureOffsetText) {
                stream << textureOffsetText << std::endl;
                stream >> textureOffset;
            }
            stream << child->GetText() << std::endl;
            Face face;
            while (!(stream >> face.v0_id).eof()) {
                stream >> face.v1_id >> face.v2_id;
                int uv0_id = face.v0_id + textureOffset;
                int uv1_id = face.v1_id + textureOffset;
                int uv2_id = face.v2_id + textureOffset;
                face.v0_id += vertexOffset;
                face.v1_id += vertexOffset;
                face.v2_id += vertexOffset;
                if (uv0_id <= texture_coordinates.size()) {
                    vertex_data[face.v0_id - 1].uv =
                        texture_coordinates[uv0_id - 1];
                } else {
                    vertex_data[face.v0_id - 1].uv = {0.F, 0.F};
                }
                if (uv1_id <= texture_coordinates.size()) {
                    vertex_data[face.v1_id - 1].uv =
                        texture_coordinates[uv1_id - 1];
                } else {
                    vertex_data[face.v1_id - 1].uv = {0.F, 0.F};
                }
                if (uv2_id <= texture_coordinates.size()) {
                    vertex_data[face.v2_id - 1].uv =
                        texture_coordinates[uv2_id - 1];
                } else {
                    vertex_data[face.v2_id - 1].uv = {0.F, 0.F};
                }
                mesh.faces.push_back(face);
            }
            stream.clear();
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
            child = element->FirstChildElement("Textures");
            if (child) {
                int texture_id;
                stream << child->GetText() << std::endl;
                while (stream >> texture_id) {
                    mesh.texture_ids.push_back(texture_id);
                }
                stream.clear();
            }
            meshes.push_back(mesh);
            mesh.faces.clear();
            mesh.transformation_indexes.clear();
            mesh.texture_ids.clear();
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
        child = element->FirstChildElement("Textures");
        if (child) {
            int texture_id;
            stream << child->GetText() << std::endl;
            while (stream >> texture_id) {
                meshInstance.texture_ids.push_back(texture_id);
            }
            stream.clear();
        }
        meshInstances.push_back(meshInstance);
        meshInstance.transformation_indexes.clear();
        meshInstance.texture_ids.clear();
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
        child = element->FirstChildElement("Textures");
        if (child) {
            int texture_id;
            stream << child->GetText() << std::endl;
            while (stream >> texture_id) {
                triangle.texture_ids.push_back(texture_id);
            }
            stream.clear();
        }
        triangles.push_back(triangle);
        triangle.transformation_indexes.clear();
        triangle.texture_ids.clear();
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
        child = element->FirstChildElement("Textures");
        if (child) {
            int texture_id;
            stream << child->GetText() << std::endl;
            while (stream >> texture_id) {
                sphere.texture_ids.push_back(texture_id);
            }
            stream.clear();
        }
        spheres.push_back(sphere);
        sphere.transformation_indexes.clear();
        sphere.texture_ids.clear();
        element = element->NextSiblingElement("Sphere");
    }
}
