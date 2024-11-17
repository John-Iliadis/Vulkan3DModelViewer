//
// Created by Gianni on 17/11/2024.
//

#include "debug.hpp"


void vulkanCheck(VkResult result, const char* msg, std::source_location location)
{
    if (result == VK_SUCCESS)
        return;

    std::stringstream errorMsg;
    errorMsg << '`' << location.function_name() << "`: " << msg << '\n';

    throw std::runtime_error(errorMsg.str());
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantFunctionResult"
#pragma ide diagnostic ignored "UnusedParameter"
VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                  VkDebugUtilsMessageTypeFlagsEXT type,
                                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                  void *userPointer)
{
//    if (pCallbackData->messageIdNumber == 1387471518)
//        return VK_FALSE;

    std::cerr << "----- Validation Layer -----" << '\n';

    std::cerr << "Severity: ";
    switch (severity)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: std::cerr << "VERBOSE"; break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: std::cerr << "INFO"; break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: std::cerr << "WARNING"; break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: std::cerr << "ERROR"; break;
        default: std::cerr << "debugCallback: Uncovered code path reached";
    }

    std::cerr << "\nMessage Type: ";
    switch (type)
    {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: std::cerr << "GENERAL"; break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: std::cerr << "VALIDATION"; break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: std::cerr << "PERFORMANCE"; break;
        default: std::cerr << "debugCallback: Uncovered code path reached";
    }

    std::cerr << "\nMessage: " << pCallbackData->pMessage;
    std::cerr << "\n----------------------------\n\n";

    __debugbreak();

    return VK_FALSE;
}
#pragma clang diagnostic pop