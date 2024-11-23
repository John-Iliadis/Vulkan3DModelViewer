//
// Created by Gianni on 18/11/2024.
//

#ifndef VULKAN3DMODELVIEWER_MODEL_HPP
#define VULKAN3DMODELVIEWER_MODEL_HPP

#include <vector>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "mesh.hpp"
#include "material.hpp"
#include "vertex.hpp"


struct Model
{
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    std::vector<VulkanTexture> textures;
    VulkanBuffer materialBuffer;
    std::unordered_map<std::string, size_t> loadedTextureCache;
    std::string directory;
};

void createModel(Model& model, VulkanRenderDevice& renderDevice, const std::string& filename);
void destroyModel(Model& model, VulkanRenderDevice& renderDevice);

void renderModel(Model& model,
                 VulkanRenderDevice& renderDevice,
                 VkPipelineLayout pipelineLayout,
                 VkCommandBuffer commandBuffer);

void loadMaterials(Model& model, VulkanRenderDevice& renderDevice, const aiScene& scene);
std::optional<size_t> loadTexture(Model& model, VulkanRenderDevice& renderDevice, const aiMaterial& material, aiTextureType textureType);

void createMaterialBuffer(Model& model, VulkanRenderDevice& renderDevice);

void processNode(Model& model, VulkanRenderDevice& renderDevice, const aiScene& scene, aiNode& node);
void processMesh(Model& model, VulkanRenderDevice& renderDevice, const aiScene& scene, aiMesh& mesh);

std::vector<Vertex> getVertices(aiMesh& mesh);
std::vector<uint32_t> getIndices(aiMesh& mesh);

VulkanTexture getTexture(Model& model,
                         VulkanRenderDevice& renderDevice,
                         aiMaterial& material,
                         aiTextureType textureType);

#endif //VULKAN3DMODELVIEWER_MODEL_HPP
