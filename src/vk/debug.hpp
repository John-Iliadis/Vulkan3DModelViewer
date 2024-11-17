//
// Created by Gianni on 17/11/2024.
//

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedParameter"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#ifndef VULKAN3DMODELVIEWER_DEBUG_HPP
#define VULKAN3DMODELVIEWER_DEBUG_HPP

#include <iostream>
#include <sstream>
#include <source_location>
#include <vulkan/vulkan.h>

void vulkanCheck(VkResult result, const char* msg, std::source_location location = std::source_location::current());

VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                  VkDebugUtilsMessageTypeFlagsEXT type,
                                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                  void *userPointer);


#endif //VULKAN3DMODELVIEWER_DEBUG_HPP

#pragma clang diagnostic pop
