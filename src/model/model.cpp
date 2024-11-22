//
// Created by Gianni on 18/11/2024.
//

#include "model.hpp"


static constexpr uint32_t import_flags
{
    aiProcess_CalcTangentSpace |
    aiProcess_JoinIdenticalVertices |
    aiProcess_Triangulate |
    aiProcess_RemoveComponent |
    aiProcess_GenNormals |
    aiProcess_OptimizeMeshes |
    aiProcess_OptimizeGraph |
    aiProcess_PreTransformVertices |
    aiProcess_RemoveRedundantMaterials |
    aiProcess_SortByPType
};

static constexpr int remove_components
{
    aiComponent_BONEWEIGHTS |
    aiComponent_ANIMATIONS |
    aiComponent_LIGHTS |
    aiComponent_CAMERAS
};

static constexpr int remove_primitives
{
    aiPrimitiveType_POINT |
    aiPrimitiveType_LINE
};

void createModel(Model& model, VulkanRenderDevice& renderDevice, const std::string& filename)
{
    model.directory = filename.substr(0, filename.find_last_of('/') + 1);

    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, remove_components);
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, remove_primitives);
    importer.SetPropertyBool(AI_CONFIG_PP_PTV_NORMALIZE, true);

    const aiScene* scene = importer.ReadFile(filename, import_flags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        vulkanCheck(static_cast<VkResult>(~VK_SUCCESS), "Failed to load model.");

    processNode(model, renderDevice, scene->mRootNode, scene);
}

void destroyModel(Model& model, VulkanRenderDevice& renderDevice)
{
    for (auto& [path, texture] : model.textures)
        destroyTexture(renderDevice, texture);

    for (Mesh& mesh : model.meshes)
        destroyMesh(mesh, renderDevice);
}

void renderModel(Model& model,
                 VulkanRenderDevice& renderDevice,
                 VkDescriptorSet descriptorSet,
                 VkPipelineLayout pipelineLayout,
                 VkCommandBuffer commandBuffer)
{
    for (Mesh& mesh : model.meshes)
    {
        renderMesh(mesh, renderDevice, descriptorSet, pipelineLayout, commandBuffer);
    }
}

void processNode(Model& model, VulkanRenderDevice& renderDevice, aiNode* node, const aiScene* scene)
{
    for (uint32_t i = 0; i < node->mNumMeshes; ++i)
    {
        uint32_t meshIndex = node->mMeshes[i];
        aiMesh& mesh = *scene->mMeshes[meshIndex];

        processMesh(model, renderDevice, mesh, scene);
    }

    for (uint32_t i = 0; i < node->mNumChildren; ++i)
        processNode(model, renderDevice, node, scene);
}

void processMesh(Model& model, VulkanRenderDevice& renderDevice, aiMesh& mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices = getVertices(mesh);
    std::vector<uint32_t> indices = getIndices(mesh);

    VulkanBuffer vertexBuffer = createVertexBuffer(renderDevice, vertices.size() * sizeof(Vertex), vertices.data());
    IndexBuffer indexBuffer = createIndexBuffer(renderDevice, indices.size() * sizeof(uint32_t), indices.data());

    aiMaterial& material = *scene->mMaterials[mesh.mMaterialIndex];
    VulkanTexture diffuseMap = getTexture(model, renderDevice, material, aiTextureType_DIFFUSE);
    VulkanTexture specularMap = getTexture(model, renderDevice, material, aiTextureType_SPECULAR);
    VulkanTexture normalMap = getTexture(model, renderDevice, material, aiTextureType_HEIGHT);

    model.meshes.emplace_back(vertexBuffer, indexBuffer, diffuseMap, specularMap, normalMap);
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

VulkanTexture getTexture(Model& model,
                         VulkanRenderDevice& renderDevice,
                         aiMaterial& material,
                         aiTextureType textureType)
{
    aiString filename;

    if (material.GetTextureCount(textureType))
    {
        material.GetTexture(textureType, 0, &filename);

        std::string path = model.directory + std::string(filename.C_Str());

        if (model.textures.contains(path))
            return model.textures.at(path);

        VulkanTexture texture = createTextureWithMips(renderDevice, path);
        model.textures.emplace(path, texture);

        return texture;
    }

    return {};
}
