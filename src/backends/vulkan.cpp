/////////////////////////////////////////////////////////////////////////////
// Name:        backends/vulkan.cpp
// Purpose:     Vulkan compute backend class
// Author:      Nickolay Babich
// Created:     01.02.2026
// Copyright:   (c) NickWare Group
// Licence:     MIT licence
/////////////////////////////////////////////////////////////////////////////

#include <cstring>
#include <indk/backends/vulkan.h>
#include <indk/math.h>
#include <indk/instance.h>
#include <algorithm>
#include <iostream>

#ifdef INDK_VULKAN_SUPPORT
#include "shaders/vulkan_shaders.h"
#endif

indk::ComputeBackends::Vulkan *vk_handler = nullptr;

indk::ComputeBackends::Vulkan::Vulkan() {
    vk_handler = this;
    BackendName = "Vulkan";
    TranslatorName = indk::Translators::VK::getTranslatorName();
    Ready = false;

#ifdef INDK_VULKAN_SUPPORT
    try {
        // Create Vulkan instance
        vk::ApplicationInfo appInfo;
        appInfo.pApplicationName = "Interference";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Interference NDK";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        vk::InstanceCreateInfo createInfo;
        createInfo.pApplicationInfo = &appInfo;

        Instance = vk::createInstance(createInfo);

        // Enumerate physical devices
        auto physicalDevices = Instance.enumeratePhysicalDevices();

        if (physicalDevices.empty()) {
            std::cerr << "No Vulkan devices found!" << std::endl;
            return;
        }

        for (auto& physDevice : physicalDevices) {
            auto properties = physDevice.getProperties();
            auto name = std::string(properties.deviceName.data());

            if (CurrentDeviceName.empty()) CurrentDeviceName = name;

            auto device = new DeviceContext;
            device->ready = false;
            device->physicalDevice = physDevice;
            device->platform_name = "Vulkan";
            device->device_name = name;

            // Get compute queue family
            auto queueFamilies = physDevice.getQueueFamilyProperties();
            uint32_t computeQueueFamily = UINT32_MAX;
            for (uint32_t i = 0; i < queueFamilies.size(); i++) {
                if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eCompute) {
                    computeQueueFamily = i;
                    device->compute_units = queueFamilies[i].queueCount;
                    break;
                }
            }

            if (computeQueueFamily == UINT32_MAX) {
                delete device;
                continue;
            }

            device->computeQueueFamilyIndex = computeQueueFamily;
            device->workgroup_size = properties.limits.maxComputeWorkGroupSize[0];

            DeviceList.insert(std::make_pair(name, device));
        }

        if (!DeviceList.empty()) Ready = true;
    } catch (const vk::SystemError& e) {
        std::cerr << "Vulkan initialization failed: " << e.what() << std::endl;
    }
#endif
}

#ifdef INDK_VULKAN_SUPPORT
vk::ShaderModule indk::ComputeBackends::Vulkan::createShaderModule(vk::Device device, const std::vector<uint32_t>& spirvCode) {
    vk::ShaderModuleCreateInfo createInfo;
    createInfo.codeSize = spirvCode.size() * sizeof(uint32_t);
    createInfo.pCode = spirvCode.data();

    return device.createShaderModule(createInfo);
}

uint32_t indk::ComputeBackends::Vulkan::findMemoryType(vk::PhysicalDevice physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    auto memProperties = physicalDevice.getMemoryProperties();
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type!");
}

void indk::ComputeBackends::Vulkan::createComputePipeline(DeviceContext* dcontext) {
    // Create descriptor set layout with 5 bindings (max needed by neurons shader)
    std::vector<vk::DescriptorSetLayoutBinding> bindings(5);
    for (uint32_t i = 0; i < 5; i++) {
        bindings[i].binding = i;
        bindings[i].descriptorType = vk::DescriptorType::eStorageBuffer;
        bindings[i].descriptorCount = 1;
        bindings[i].stageFlags = vk::ShaderStageFlagBits::eCompute;
    }

    vk::DescriptorSetLayoutCreateInfo layoutInfo;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    dcontext->descriptorSetLayout = dcontext->device.createDescriptorSetLayout(layoutInfo);

    // Create pipeline layout
    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &dcontext->descriptorSetLayout;

    dcontext->pipelineLayout = dcontext->device.createPipelineLayout(pipelineLayoutInfo);

    // Create shader modules
    dcontext->pairsShader = createShaderModule(dcontext->device, indk::shaders::pairs_spirv);
    dcontext->receptorsShader = createShaderModule(dcontext->device, indk::shaders::receptors_spirv);
    dcontext->neuronsShader = createShaderModule(dcontext->device, indk::shaders::neurons_spirv);

    // Create compute pipelines
    auto createPipeline = [&](vk::ShaderModule shader) -> vk::Pipeline {
        vk::PipelineShaderStageCreateInfo stageInfo;
        stageInfo.stage = vk::ShaderStageFlagBits::eCompute;
        stageInfo.module = shader;
        stageInfo.pName = "main";

        vk::ComputePipelineCreateInfo pipelineInfo;
        pipelineInfo.stage = stageInfo;
        pipelineInfo.layout = dcontext->pipelineLayout;

        auto result = dcontext->device.createComputePipeline(nullptr, pipelineInfo);
        return result.value;
    };

    dcontext->pairsPipeline = createPipeline(dcontext->pairsShader);
    dcontext->receptorsPipeline = createPipeline(dcontext->receptorsShader);
    dcontext->neuronsPipeline = createPipeline(dcontext->neuronsShader);

    // Create descriptor pool
    vk::DescriptorPoolSize poolSize;
    poolSize.type = vk::DescriptorType::eStorageBuffer;
    poolSize.descriptorCount = 15;  // 5 bindings * 3 pipelines

    vk::DescriptorPoolCreateInfo poolInfo;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 3;

    dcontext->descriptorPool = dcontext->device.createDescriptorPool(poolInfo);
}
#endif

indk::ComputeBackends::Vulkan::DeviceContext* indk::ComputeBackends::Vulkan::doInitCurrentDevice() {
    if (!Ready) return nullptr;

#ifdef INDK_VULKAN_SUPPORT
    auto found = DeviceList.find(CurrentDeviceName);

    if (found == DeviceList.end()) {
        throw indk::Error(indk::Error::EX_BACKEND_CL_DEVICE_NOT_FOUND, "selected device name " + CurrentDeviceName);
    }

    auto dcontext = found->second;

    if (dcontext->ready) return dcontext;

    // Create logical device
    float queuePriority = 1.0f;
    vk::DeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.queueFamilyIndex = dcontext->computeQueueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    vk::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

    dcontext->device = dcontext->physicalDevice.createDevice(deviceCreateInfo);
    dcontext->computeQueue = dcontext->device.getQueue(dcontext->computeQueueFamilyIndex, 0);

    // Create command pool
    vk::CommandPoolCreateInfo poolInfo;
    poolInfo.queueFamilyIndex = dcontext->computeQueueFamilyIndex;
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

    dcontext->commandPool = dcontext->device.createCommandPool(poolInfo);

    // Create pipelines
    createComputePipeline(dcontext);

    dcontext->ready = true;

    return dcontext;
#endif
    return nullptr;
}

void* indk::ComputeBackends::Vulkan::doTranslate(const std::vector<indk::Neuron*>& neurons, const std::vector<std::string> &outputs, const indk::StateSyncMap &sync) {
    return indk::Translators::VK::doTranslate(neurons, outputs, sync);
}

void indk::ComputeBackends::Vulkan::doCompute(const std::vector<indk::Neuron*> &neurons, const std::vector<std::vector<float>> &x, const std::vector<std::string>& inputs, void *_model) {
    auto model = (indk::Translators::VK::ModelData*)_model;

#ifdef INDK_VULKAN_SUPPORT
    auto dcontext = doInitCurrentDevice();
    if (!dcontext || !dcontext->ready) return;

    // Calculate buffer sizes
    vk::DeviceSize pairsSize = model->pair_pool_size * 16 * sizeof(float);
    vk::DeviceSize receptorsSize = model->receptor_pool_size * 8 * sizeof(float);
    vk::DeviceSize neuronsSize = model->neuron_pool_size * 4 * sizeof(float);
    vk::DeviceSize inputsSize = model->input_pool_size * 2 * sizeof(float);
    vk::DeviceSize outputsSize = model->neuron_pool_size * sizeof(float);
    vk::DeviceSize timesSize = model->neuron_pool_size * sizeof(int32_t);

    // Create buffers
    auto createBufferWithMemory = [&](vk::DeviceSize size) -> std::pair<vk::Buffer, vk::DeviceMemory> {
        vk::BufferCreateInfo bufferInfo;
        bufferInfo.size = size;
        bufferInfo.usage = vk::BufferUsageFlagBits::eStorageBuffer;
        bufferInfo.sharingMode = vk::SharingMode::eExclusive;

        auto buffer = dcontext->device.createBuffer(bufferInfo);

        auto memRequirements = dcontext->device.getBufferMemoryRequirements(buffer);
        vk::MemoryAllocateInfo allocInfo;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(dcontext->physicalDevice, memRequirements.memoryTypeBits,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        auto memory = dcontext->device.allocateMemory(allocInfo);
        dcontext->device.bindBufferMemory(buffer, memory, 0);

        return {buffer, memory};
    };

    auto pairsBufMem = createBufferWithMemory(pairsSize);
    vk::Buffer pairsBuffer = pairsBufMem.first;
    vk::DeviceMemory pairsMemory = pairsBufMem.second;

    auto receptorsBufMem = createBufferWithMemory(receptorsSize);
    vk::Buffer receptorsBuffer = receptorsBufMem.first;
    vk::DeviceMemory receptorsMemory = receptorsBufMem.second;

    auto neuronsBufMem = createBufferWithMemory(neuronsSize);
    vk::Buffer neuronsBuffer = neuronsBufMem.first;
    vk::DeviceMemory neuronsMemory = neuronsBufMem.second;

    auto inputsBufMem = createBufferWithMemory(inputsSize);
    vk::Buffer inputsBuffer = inputsBufMem.first;
    vk::DeviceMemory inputsMemory = inputsBufMem.second;

    auto outputsBufMem = createBufferWithMemory(outputsSize);
    vk::Buffer outputsBuffer = outputsBufMem.first;
    vk::DeviceMemory outputsMemory = outputsBufMem.second;

    auto timesBufMem = createBufferWithMemory(timesSize);
    vk::Buffer timesBuffer = timesBufMem.first;
    vk::DeviceMemory timesMemory = timesBufMem.second;

    // Copy initial data to buffers
    auto copyToBuffer = [&](vk::DeviceMemory memory, const void* data, vk::DeviceSize size) {
        void* mapped = dcontext->device.mapMemory(memory, 0, size);
        memcpy(mapped, data, size);
        dcontext->device.unmapMemory(memory);
    };

    copyToBuffer(pairsMemory, model->PairsInfo.data(), pairsSize);
    copyToBuffer(receptorsMemory, model->ReceptorsInfo.data(), receptorsSize);
    copyToBuffer(neuronsMemory, model->NeuronsInfo.data(), neuronsSize);
    copyToBuffer(outputsMemory, model->Outputs.data(), outputsSize);

    std::fill(model->Times.begin(), model->Times.end(), 0);

    model->batch_size = x[0].size();

    bool done = false;

    std::vector<std::pair<uint64_t, bool>> incache;
    uint64_t i = 0;
    for (const auto &n: model->objects) {
        auto elist = n->getEntries();

        for (const auto &e: elist) {
            auto ifound = std::find(inputs.begin(), inputs.end(), e);
            if (ifound != inputs.end()) {
                auto index = std::distance(inputs.begin(), ifound);
                incache.emplace_back(index, false);
            } else {
                auto nfound = std::find_if(model->objects.begin(), model->objects.end(), [e](indk::Neuron *n) { return n->getName() == e; });
                if (nfound != model->objects.end()) {
                    auto index = std::distance(model->objects.begin(), nfound);
                    incache.emplace_back(index, true);
                }
            }
        }
        model->output_sequence.emplace_back();

        while (model->Times[i] < n->getLatency()) {
            model->Times[i]++;
            model->output_sequence.back().emplace_back(0);
        }

        i++;
    }

    copyToBuffer(timesMemory, model->Times.data(), timesSize);

    // Allocate descriptor sets
    std::vector<vk::DescriptorSetLayout> layouts(3, dcontext->descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool = dcontext->descriptorPool;
    allocInfo.descriptorSetCount = 3;
    allocInfo.pSetLayouts = layouts.data();

    auto descriptorSets = dcontext->device.allocateDescriptorSets(allocInfo);

    // Update descriptor sets for pairs kernel
    auto updateDescriptorSet = [&](vk::DescriptorSet set, const std::vector<std::pair<vk::Buffer, vk::DeviceSize>>& buffers) {
        std::vector<vk::DescriptorBufferInfo> bufferInfos(buffers.size());
        std::vector<vk::WriteDescriptorSet> writes(buffers.size());

        for (size_t i = 0; i < buffers.size(); i++) {
            bufferInfos[i].buffer = buffers[i].first;
            bufferInfos[i].offset = 0;
            bufferInfos[i].range = buffers[i].second;

            writes[i].dstSet = set;
            writes[i].dstBinding = static_cast<uint32_t>(i);
            writes[i].descriptorCount = 1;
            writes[i].descriptorType = vk::DescriptorType::eStorageBuffer;
            writes[i].pBufferInfo = &bufferInfos[i];
        }

        dcontext->device.updateDescriptorSets(writes, {});
    };

    // Pairs kernel: pairs, inputs, outputs
    updateDescriptorSet(descriptorSets[0], {{pairsBuffer, pairsSize}, {inputsBuffer, inputsSize}, {outputsBuffer, outputsSize}});
    // Receptors kernel: pairs, receptors, inputs
    updateDescriptorSet(descriptorSets[1], {{pairsBuffer, pairsSize}, {receptorsBuffer, receptorsSize}, {inputsBuffer, inputsSize}});
    // Neurons kernel: receptors, neurons, inputs, outputs, times
    updateDescriptorSet(descriptorSets[2], {{receptorsBuffer, receptorsSize}, {neuronsBuffer, neuronsSize}, {inputsBuffer, inputsSize}, {outputsBuffer, outputsSize}, {timesBuffer, timesSize}});

    // Create command buffer
    vk::CommandBufferAllocateInfo cmdAllocInfo;
    cmdAllocInfo.commandPool = dcontext->commandPool;
    cmdAllocInfo.level = vk::CommandBufferLevel::ePrimary;
    cmdAllocInfo.commandBufferCount = 1;

    auto commandBuffers = dcontext->device.allocateCommandBuffers(cmdAllocInfo);
    auto commandBuffer = commandBuffers[0];

    // Main compute loop
    while (!done) {
        done = true;

        uint64_t xi = 0, xistart;
        i = 0;
        for (const auto &n: model->objects) {
            auto elist = n->getEntries();
            bool ready = true;

            if (model->Times[i] == model->batch_size) {
                ready = false;
            } else {
                xistart = xi;
                for (const auto &e: elist) {
                    if (std::get<1>(incache[xi])) {
                        auto source = std::get<0>(incache[xi]);
                        auto st = model->Times[source];
                        if (st <= model->Times[i] - n->getLatency()) {
                            ready = false;
                            break;
                        }
                    }
                    xi++;
                }
                xi = xistart;
            }

            for (const auto &e: elist) {
                uint64_t inputBase = xi * 2;
                if (ready) {
                    if (!std::get<1>(incache[xi])) {
                        model->Inputs[inputBase] = 1.0f;
                        model->Inputs[inputBase + 1] = x[std::get<0>(incache[xi])][model->Times[i]];
                    } else {
                        auto source = std::get<0>(incache[xi]);
                        model->Inputs[inputBase] = 1.0f;
                        model->Inputs[inputBase + 1] = model->output_sequence[source][model->Times[i] - n->getLatency()];
                    }
                } else {
                    model->Inputs[inputBase] = 0.0f;
                    model->Inputs[inputBase + 1] = 0.0f;
                }
                xi++;
            }
            i++;
        }

        copyToBuffer(inputsMemory, model->Inputs.data(), inputsSize);

        // Record and submit command buffer
        commandBuffer.reset();
        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        commandBuffer.begin(beginInfo);

        // Dispatch pairs kernel
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, dcontext->pairsPipeline);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, dcontext->pipelineLayout, 0, descriptorSets[0], {});
        commandBuffer.dispatch((model->pair_pool_size + 63) / 64, 1, 1);

        vk::MemoryBarrier barrier;
        barrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, {}, barrier, {}, {});

        // Dispatch receptors kernel
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, dcontext->receptorsPipeline);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, dcontext->pipelineLayout, 0, descriptorSets[1], {});
        commandBuffer.dispatch((model->receptor_pool_size + 63) / 64, 1, 1);

        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, {}, barrier, {}, {});

        // Dispatch neurons kernel
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, dcontext->neuronsPipeline);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, dcontext->pipelineLayout, 0, descriptorSets[2], {});
        commandBuffer.dispatch((model->neuron_pool_size + 63) / 64, 1, 1);

        commandBuffer.end();

        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        dcontext->computeQueue.submit(submitInfo);
        dcontext->computeQueue.waitIdle();

        // Read back results
        void* mapped = dcontext->device.mapMemory(timesMemory, 0, timesSize);
        memcpy(model->Times.data(), mapped, timesSize);
        dcontext->device.unmapMemory(timesMemory);

        for (i = 0; i < model->neuron_pool_size; i++) {
            if (model->Times[i] != model->batch_size) {
                done = false;
                break;
            }
        }

        mapped = dcontext->device.mapMemory(outputsMemory, 0, outputsSize);
        memcpy(model->Outputs.data(), mapped, outputsSize);
        dcontext->device.unmapMemory(outputsMemory);

        for (i = 0; i < model->neuron_pool_size; i++) {
            if (model->Outputs[i] >= 0) model->output_sequence[i].emplace_back(model->Outputs[i]);
        }
    }

    // Read back final pairs and receptors data
    void* mapped = dcontext->device.mapMemory(pairsMemory, 0, pairsSize);
    memcpy(model->PairsInfo.data(), mapped, pairsSize);
    dcontext->device.unmapMemory(pairsMemory);

    mapped = dcontext->device.mapMemory(receptorsMemory, 0, receptorsSize);
    memcpy(model->ReceptorsInfo.data(), mapped, receptorsSize);
    dcontext->device.unmapMemory(receptorsMemory);

    // Cleanup buffers
    dcontext->device.freeMemory(pairsMemory);
    dcontext->device.freeMemory(receptorsMemory);
    dcontext->device.freeMemory(neuronsMemory);
    dcontext->device.freeMemory(inputsMemory);
    dcontext->device.freeMemory(outputsMemory);
    dcontext->device.freeMemory(timesMemory);

    dcontext->device.destroyBuffer(pairsBuffer);
    dcontext->device.destroyBuffer(receptorsBuffer);
    dcontext->device.destroyBuffer(neuronsBuffer);
    dcontext->device.destroyBuffer(inputsBuffer);
    dcontext->device.destroyBuffer(outputsBuffer);
    dcontext->device.destroyBuffer(timesBuffer);

    dcontext->device.freeCommandBuffers(dcontext->commandPool, commandBuffer);
    dcontext->device.freeDescriptorSets(dcontext->descriptorPool, descriptorSets);
#endif
}

void indk::ComputeBackends::Vulkan::doReset(const std::vector<std::string> &neurons, void *model) {
    indk::Translators::VK::doReset((indk::Translators::VK::ModelData*)model);
}

void indk::ComputeBackends::Vulkan::doClear(void *model) {
    indk::Translators::VK::doClear((indk::Translators::VK::ModelData*)model);
}

void indk::ComputeBackends::Vulkan::setMode(void *_model, bool learning) {
    auto model = (indk::Translators::VK::ModelData*)_model;
    model->learning_mode = learning;
}

void indk::ComputeBackends::Vulkan::setParameters(indk::ComputeBackend::Parameters *parameters) {
    CurrentDeviceName = ((Parameters*)parameters)->device_name;
}

std::vector<indk::OutputValue> indk::ComputeBackends::Vulkan::getOutputValues(const std::vector<std::string> &neurons, void *_model) {
    auto model = (indk::Translators::VK::ModelData*)_model;

    std::vector<indk::OutputValue> outputs;
    outputs.reserve(model->outputs.size());

    for (const auto &o: model->outputs) {
        auto found = std::find_if(model->objects.begin(), model->objects.end(), [o](indk::Neuron *n) { return n->getName() == o; });
        if (found != model->objects.end()) {
            auto index = std::distance(model->objects.begin(), found);
            auto value = model->output_sequence[index][model->batch_size - 1];
            indk::OutputValue output = {.value = value, .source = o, .time = model->batch_size};
            outputs.emplace_back(output);
        }
    }

    return outputs;
}

std::map<std::string, std::vector<indk::Position>> indk::ComputeBackends::Vulkan::getReceptorPositions(const std::vector<std::string> &neurons, void *_model) {
    auto model = (indk::Translators::VK::ModelData*)_model;

    std::map<std::string, std::vector<indk::Position>> list;

    uint64_t ri = 0;
    for (uint64_t i = 0; i < model->objects.size(); i++) {
        auto n = model->objects[i];
        std::vector<indk::Position> positions;

        for (uint64_t r = 0; r < n->getReceptorsCount(); r++) {
            std::vector<float> coords;

            // Get receptor position from pairs (first pair of each receptor)
            uint64_t rbase = ri * 8;
            uint64_t pairIdx = static_cast<uint64_t>(model->ReceptorsInfo[rbase]);  // left pairs range
            uint64_t pbase = pairIdx * 16;

            coords.emplace_back(model->PairsInfo[pbase]);      // receptor x
            coords.emplace_back(model->PairsInfo[pbase + 1]);  // receptor y

            while (coords.size() < n->getDimensionsCount()) coords.emplace_back(0);

            positions.emplace_back(n->getXm(), coords);
            ri++;
        }
        list.insert(std::make_pair(n->getName(), positions));
    }

    return list;
}

std::vector<indk::ComputeBackends::Vulkan::DeviceInfo> indk::ComputeBackends::Vulkan::getDeviceInfoList() {
    std::vector<DeviceInfo> info;
    for (auto &d: DeviceList) {
        DeviceInfo di;
        di.platform_name = d.second->platform_name;
        di.device_name = d.second->device_name;
        di.compute_units = d.second->compute_units;
        di.workgroup_size = d.second->workgroup_size;
        info.emplace_back(di);
    }
    return info;
}

std::vector<indk::ComputeBackends::Vulkan::DeviceInfo> indk::ComputeBackends::Vulkan::getDevicesInfo() {
    if (vk_handler) return vk_handler->getDeviceInfoList();
    return {};
}

indk::ComputeBackends::Vulkan::~Vulkan() {
#ifdef INDK_VULKAN_SUPPORT
    for (auto &d: DeviceList) {
        if (d.second->ready) {
            d.second->device.destroyPipeline(d.second->pairsPipeline);
            d.second->device.destroyPipeline(d.second->receptorsPipeline);
            d.second->device.destroyPipeline(d.second->neuronsPipeline);
            d.second->device.destroyShaderModule(d.second->pairsShader);
            d.second->device.destroyShaderModule(d.second->receptorsShader);
            d.second->device.destroyShaderModule(d.second->neuronsShader);
            d.second->device.destroyPipelineLayout(d.second->pipelineLayout);
            d.second->device.destroyDescriptorSetLayout(d.second->descriptorSetLayout);
            d.second->device.destroyDescriptorPool(d.second->descriptorPool);
            d.second->device.destroyCommandPool(d.second->commandPool);
            d.second->device.destroy();
        }
        delete d.second;
    }
    DeviceList.clear();

    if (Instance) {
        Instance.destroy();
    }
#endif
}
