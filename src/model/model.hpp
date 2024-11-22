//
// Created by Gianni on 18/11/2024.
//

#ifndef VULKAN3DMODELVIEWER_MODEL_HPP
#define VULKAN3DMODELVIEWER_MODEL_HPP

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "mesh.hpp"
#include "vertex.hpp"


struct Model
{
    std::vector<Mesh> meshes;
    std::unordered_map<std::string, VulkanTexture> textures;
    std::string directory;
};

void createModel(Model& model, VulkanRenderDevice& renderDevice, const std::string& filename);
void destroyModel(Model& model, VulkanRenderDevice& renderDevice);

void renderModel(Model& model,
                 VulkanRenderDevice& renderDevice,
                 VkDescriptorSet descriptorSet,
                 VkPipelineLayout pipelineLayout,
                 VkCommandBuffer commandBuffer);

void processNode(Model& model, VulkanRenderDevice& renderDevice, aiNode* node, const aiScene* scene);
void processMesh(Model& model, VulkanRenderDevice& renderDevice, aiMesh& mesh, const aiScene* scene);

std::vector<Vertex> getVertices(aiMesh& mesh);
std::vector<uint32_t> getIndices(aiMesh& mesh);

VulkanTexture getTexture(Model& model,
                         VulkanRenderDevice& renderDevice,
                         aiMaterial& material,
                         aiTextureType textureType);

#endif //VULKAN3DMODELVIEWER_MODEL_HPP
