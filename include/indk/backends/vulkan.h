/////////////////////////////////////////////////////////////////////////////
// Name:        indk/backends/vulkan.h
// Purpose:     Vulkan compute backend class header
// Author:      Nickolay Babich
// Created:     01.02.2026
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#ifndef INTERFERENCE_BACKENDS_VULKAN_H
#define INTERFERENCE_BACKENDS_VULKAN_H

#include <indk/backend.h>
#include <indk/position.h>
#include <indk/translators/vk.h>

#ifdef INDK_VULKAN_SUPPORT
    #define VULKAN_HPP_NO_CONSTRUCTORS
    #include <vulkan/vulkan.hpp>
#endif

namespace indk {
    namespace ComputeBackends {
        class Vulkan : public indk::ComputeBackend {
        public:
            typedef struct device_context {
#ifdef INDK_VULKAN_SUPPORT
                vk::PhysicalDevice physicalDevice;
                vk::Device device;
                vk::Queue computeQueue;
                uint32_t computeQueueFamilyIndex;
                vk::CommandPool commandPool;
                vk::DescriptorPool descriptorPool;
                vk::DescriptorSetLayout descriptorSetLayout;
                vk::PipelineLayout pipelineLayout;

                // Pipelines for three kernels
                vk::Pipeline pairsPipeline;
                vk::Pipeline receptorsPipeline;
                vk::Pipeline neuronsPipeline;

                // Shader modules
                vk::ShaderModule pairsShader;
                vk::ShaderModule receptorsShader;
                vk::ShaderModule neuronsShader;
#endif
                bool ready;
                std::string platform_name;
                std::string device_name;
                uint32_t compute_units;
                uint32_t workgroup_size;
            } DeviceContext;

            typedef struct vk_device_info {
                std::string platform_name;
                std::string device_name;
                uint32_t compute_units;
                uint32_t workgroup_size;
            } DeviceInfo;

        private:
            std::map<std::string, indk::ComputeBackends::Vulkan::DeviceContext*> DeviceList;
            std::string CurrentDeviceName;
#ifdef INDK_VULKAN_SUPPORT
            vk::Instance Instance;

            static vk::ShaderModule doCreateShaderModule(vk::Device device, const std::vector<uint32_t>& spirvCode);
            static uint32_t doFindMemoryType(vk::PhysicalDevice physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties);
            static void doCreateComputePipeline(DeviceContext* dcontext);
#endif
        public:
            typedef struct Parameters : public indk::ComputeBackend::Parameters {
                std::string device_name;
            } Parameters;

            Vulkan();
            indk::ComputeBackends::Vulkan::DeviceContext* doInitCurrentDevice();
            void* doTranslate(const std::vector<indk::Neuron*>& neurons, const std::vector<std::string>& outputs, const indk::StateSyncMap& sync) override;
            void doCompute(const std::vector<indk::Neuron*> &neurons, const std::vector<std::vector<float>> &x, const std::vector<std::string>& inputs, void *_instance) override;
            void doReset(const std::vector<std::string> &neurons, void*) override;
            void doClear(void*) override;
            void setMode(void *model, bool learning) override;
            void setParameters(indk::ComputeBackend::Parameters*) override;
            std::vector<indk::OutputValue> getOutputValues(const std::vector<std::string> &neurons, void *_model) override;
            std::map<std::string, std::vector<indk::Position>> getReceptorPositions(const std::vector<std::string> &neurons, void *_model) override;
            std::vector<DeviceInfo> getDeviceInfoList();
            static std::vector<DeviceInfo> getDevicesInfo();
            ~Vulkan();
        };
    }
}

#endif //INTERFERENCE_BACKENDS_VULKAN_H
