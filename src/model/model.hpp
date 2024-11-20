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
};

void createModel(Model& model, VulkanRenderDevice& renderDevice, const std::string& filename);
void destroyModel(Model& model, VulkanRenderDevice& renderDevice);

void processNode(Model& model, VulkanRenderDevice& renderDevice, aiNode* node, const aiScene* scene);
void processMesh(Model& model, VulkanRenderDevice& renderDevice, aiMesh& mesh, const aiScene* scene);

std::vector<Vertex> getVertices(aiMesh& mesh);
std::vector<uint32_t> getIndices(aiMesh& mesh);

#endif //VULKAN3DMODELVIEWER_MODEL_HPP
