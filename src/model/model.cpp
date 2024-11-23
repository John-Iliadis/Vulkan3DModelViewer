//
// Created by Gianni on 18/11/2024.
//

#include "model.hpp"


static constexpr uint32_t importFlags {
    aiProcess_CalcTangentSpace |
    aiProcess_JoinIdenticalVertices |
    aiProcess_Triangulate |
    aiProcess_RemoveComponent |
    aiProcess_GenNormals |
    aiProcess_FlipUVs |
    aiProcess_OptimizeMeshes |
    aiProcess_OptimizeGraph |
    aiProcess_PreTransformVertices |
    aiProcess_RemoveRedundantMaterials |
    aiProcess_SortByPType
};

static constexpr int removeComponents {
    aiComponent_BONEWEIGHTS |
    aiComponent_ANIMATIONS |
    aiComponent_LIGHTS |
    aiComponent_CAMERAS
};

static constexpr int removePrimitives {
    aiPrimitiveType_POINT |
    aiPrimitiveType_LINE
};

void createModel(Model& model, VulkanRenderDevice& renderDevice, const std::string& filename)
{
    model.directory = filename.substr(0, filename.find_last_of('/') + 1);

    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, removeComponents);
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, removePrimitives);
    importer.SetPropertyBool(AI_CONFIG_PP_PTV_NORMALIZE, true);

    const aiScene* scene = importer.ReadFile(filename, importFlags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        vulkanCheck(static_cast<VkResult>(~VK_SUCCESS), "Failed to load model.");

    loadMaterials(model, renderDevice, *scene);
    createMaterialBuffer(model, renderDevice);
    processNode(model, renderDevice, *scene, *scene->mRootNode);
}

void destroyModel(Model& model, VulkanRenderDevice& renderDevice)
{
    for (VulkanTexture& texture : model.textures)
        destroyTexture(renderDevice, texture);

    destroyBuffer(renderDevice, model.materialBuffer);

    for (Mesh& mesh : model.meshes)
        destroyMesh(mesh, renderDevice);
}

void renderModel(Model& model,
                 VulkanRenderDevice& renderDevice,
                 VkPipelineLayout pipelineLayout,
                 VkCommandBuffer commandBuffer)
{
    for (Mesh& mesh : model.meshes)
    {
        renderMesh(mesh, commandBuffer, pipelineLayout);
    }
}

void loadMaterials(Model& model, VulkanRenderDevice& renderDevice, const aiScene& scene)
{
    model.materials.reserve(scene.mNumMaterials);

    for (uint32_t i = 0; i < scene.mNumMaterials; ++i)
    {
        aiMaterial& aiMaterial = *scene.mMaterials[i];
        Material material {};

        std::optional<size_t> diffuseMapIndex = loadTexture(model, renderDevice, aiMaterial, aiTextureType_DIFFUSE);
        std::optional<size_t> specularMapIndex = loadTexture(model, renderDevice, aiMaterial, aiTextureType_SPECULAR);
        std::optional<size_t> normalMapIndex = loadTexture(model, renderDevice, aiMaterial, aiTextureType_HEIGHT);

        if (diffuseMapIndex.has_value())
        {
            material.diffuseMapIndex = diffuseMapIndex.value();
            material.hasDiffuseMap = 1;
        }

        if (specularMapIndex.has_value())
        {
            material.specularMapIndex = specularMapIndex.value();
            material.hasSpecularMap = 1;
        }

        if (normalMapIndex.has_value())
        {
            material.normalMapIndex = normalMapIndex.value();
            material.hasNormalMap = 1;
        }

        model.materials.push_back(material);
    }
}

std::optional<size_t> loadTexture(Model& model, VulkanRenderDevice& renderDevice, const aiMaterial& material, aiTextureType textureType)
{
    if (!material.GetTextureCount(textureType))
        return {};

    aiString filename;

    material.GetTexture(textureType, 0, &filename);

    std::string path = model.directory + std::string(filename.C_Str());

    if (model.loadedTextureCache.contains(path))
        return model.loadedTextureCache.at(path);

    model.textures.push_back(createTextureWithMips(renderDevice, path));

    size_t textureIndex = model.textures.size() - 1;

    model.loadedTextureCache.emplace(path, textureIndex);

    return textureIndex;
}

void createMaterialBuffer(Model& model, VulkanRenderDevice& renderDevice)
{
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    VkMemoryPropertyFlags memoryProperties {
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    model.materialBuffer = createBuffer(renderDevice,
                                        model.materials.size() * sizeof(Material),
                                        usage,
                                        memoryProperties,
                                        model.materials.data());
}

void processNode(Model& model, VulkanRenderDevice& renderDevice, const aiScene& scene, aiNode& node)
{
    for (uint32_t i = 0; i < node.mNumMeshes; ++i)
    {
        uint32_t meshIndex = node.mMeshes[i];
        aiMesh& mesh = *scene.mMeshes[meshIndex];

        processMesh(model, renderDevice, scene, mesh);
    }

    for (uint32_t i = 0; i < node.mNumChildren; ++i)
        processNode(model, renderDevice, scene, *node.mChildren[i]);
}

void processMesh(Model& model, VulkanRenderDevice& renderDevice, const aiScene& scene, aiMesh& mesh)
{
    std::vector<Vertex> vertices = getVertices(mesh);
    std::vector<uint32_t> indices = getIndices(mesh);

    VulkanBuffer vertexBuffer = createVertexBuffer(renderDevice, vertices.size() * sizeof(Vertex), vertices.data());
    IndexBuffer indexBuffer = createIndexBuffer(renderDevice, indices.size() * sizeof(uint32_t), indices.data());
    uint32_t materialIndex = mesh.mMaterialIndex;

    model.meshes.emplace_back(vertexBuffer, indexBuffer, materialIndex);
}

std::vector<Vertex> getVertices(aiMesh& mesh)
{
    size_t vertexCount = mesh.mNumVertices;

    std::vector<Vertex> vertices;
    vertices.reserve(vertexCount);

    for (size_t i = 0; i < vertexCount; ++i)
    {
        Vertex vertex {};

         vertex.position = *reinterpret_cast<glm::vec3*>(&mesh.mVertices[i]);
         vertex.normal = *reinterpret_cast<glm::vec3*>(&mesh.mNormals[i]);

        if (mesh.HasTextureCoords(0))
        {
            vertex.tangent = *reinterpret_cast<glm::vec3*>(&mesh.mTangents[i]);
            vertex.bitangent = *reinterpret_cast<glm::vec3*>(&mesh.mBitangents[i]);
            vertex.texCoords = *reinterpret_cast<glm::vec2*>(&mesh.mTextureCoords[0][i]);
        }

        vertices.push_back(vertex);
    }

    return vertices;
}

std::vector<uint32_t> getIndices(aiMesh& mesh)
{
    uint32_t faceCount = mesh.mNumFaces;
    size_t indexCount = faceCount * 3;

    std::vector<uint32_t> indices;
    indices.reserve(indexCount);

    for (uint32_t i = 0; i < faceCount; ++i)
    {
        aiFace& face = mesh.mFaces[i];

        indices.push_back(face.mIndices[0]);
        indices.push_back(face.mIndices[1]);
        indices.push_back(face.mIndices[2]);
    }

    return indices;
}
