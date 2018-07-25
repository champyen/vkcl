
#include <vulkan/vulkan.h>

#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "vkcl.h"

#define VK_CHK(result) {  if (VK_SUCCESS != (result)) { fprintf(stderr, "Failure at %u %s\n", __LINE__, __FILE__); exit(-1); }  }

VkResult vkGetBestComputeQueueNPH(VkPhysicalDevice physicalDevice, uint32_t* queueFamilyIndex) {
    uint32_t queueFamilyPropertiesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, 0);

    VkQueueFamilyProperties* const queueFamilyProperties = (VkQueueFamilyProperties*)alloca(sizeof(VkQueueFamilyProperties) * queueFamilyPropertiesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, queueFamilyProperties);

    // first try and find a queue that has just the compute bit set
    for (uint32_t i = 0; i < queueFamilyPropertiesCount; i++) {
        // mask out the sparse binding bit that we aren't caring about (yet!) and the transfer bit
        const VkQueueFlags maskedFlags = (~(VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT) & queueFamilyProperties[i].queueFlags);
        if (!(VK_QUEUE_GRAPHICS_BIT & maskedFlags) && (VK_QUEUE_COMPUTE_BIT & maskedFlags)) {
            *queueFamilyIndex = i;
            return VK_SUCCESS;
        }
    }

    // lastly get any queue that'll work for us
    for (uint32_t i = 0; i < queueFamilyPropertiesCount; i++) {
        // mask out the sparse binding bit that we aren't caring about (yet!) and the transfer bit
        const VkQueueFlags maskedFlags = (~(VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT) & queueFamilyProperties[i].queueFlags);
        if (VK_QUEUE_COMPUTE_BIT & maskedFlags) {
            *queueFamilyIndex = i;
            return VK_SUCCESS;
        }
    }

    return VK_ERROR_INITIALIZATION_FAILED;
}


int main(int argc, char** argv) {

    const VkApplicationInfo applicationInfo = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,
        0,
        "VKComputeSample",
        0,
        "",
        0,
        VK_MAKE_VERSION(1, 0, 54)
    };

    const VkInstanceCreateInfo instanceCreateInfo = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        0,
        0,
        &applicationInfo,
        0,
        0,
        0,
        0
    };

    VkInstance instance;
    VK_CHK(vkCreateInstance(&instanceCreateInfo, 0, &instance));

    uint32_t physicalDeviceCount = 0;
    VK_CHK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, 0));
    printf("%d Vulkan Devices\n", physicalDeviceCount);

    VkPhysicalDevice* const physicalDevices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);
    VK_CHK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices));

    /*
    for (uint32_t i = 0; i < physicalDeviceCount; i++) {
        uint32_t queueFamilyIndex = 0;
        VK_CHK(vkGetBestComputeQueueNPH(physicalDevices[i], &queueFamilyIndex));
        printf("%d %d\n", i, queueFamilyIndex);
    }
    */
    {
        uint32_t queueFamilyIndex = 0;
        VK_CHK(vkGetBestComputeQueueNPH(physicalDevices[0], &queueFamilyIndex));
        printf("queueFamilyIndex %d\n", queueFamilyIndex);
        const float queuePrioritory = 1.0f;
        const VkDeviceQueueCreateInfo deviceQueueCreateInfo = {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            0,
            0,
            queueFamilyIndex,
            1,
            &queuePrioritory
        };

        const VkDeviceCreateInfo deviceCreateInfo = {
            VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            0,
            0,
            1,
            &deviceQueueCreateInfo,
            0,
            0,
            0,
            0,
            0
        };

        VkDevice device;
        VK_CHK(vkCreateDevice(physicalDevices[0], &deviceCreateInfo, 0, &device));

        VkPhysicalDeviceMemoryProperties properties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevices[0], &properties);

        const int32_t bufferLength = TEST_BUFLEN;
        const uint32_t bufferSize = sizeof(int32_t) * bufferLength;
        // we are going to need two buffers from this one memory
        const VkDeviceSize memorySize = bufferSize * 2;
        // set memoryTypeIndex to an invalid entry in the properties.memoryTypes array
        uint32_t memoryTypeIndex = VK_MAX_MEMORY_TYPES;

        for (uint32_t k = 0; k < properties.memoryTypeCount; k++) {
            if ((VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT & properties.memoryTypes[k].propertyFlags) &&
            (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT & properties.memoryTypes[k].propertyFlags) &&
            (memorySize < properties.memoryHeaps[properties.memoryTypes[k].heapIndex].size)) {
                memoryTypeIndex = k;
                break;
            }
        }

        VK_CHK(memoryTypeIndex == VK_MAX_MEMORY_TYPES ? VK_ERROR_OUT_OF_HOST_MEMORY : VK_SUCCESS);

        const VkMemoryAllocateInfo memoryAllocateInfo = {
            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            0,
            memorySize,
            memoryTypeIndex
        };

        VkDeviceMemory memory;
        int32_t *payload;
        VK_CHK(vkAllocateMemory(device, &memoryAllocateInfo, 0, &memory));
        VK_CHK(vkMapMemory(device, memory, 0, memorySize, 0, (void *)&payload));
        for (uint32_t k = 1; k < memorySize / sizeof(int32_t); k++) {
            payload[k] = rand();
        }
        vkUnmapMemory(device, memory);

        const VkBufferCreateInfo bufferCreateInfo = {
            VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            0,
            0,
            bufferSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_SHARING_MODE_EXCLUSIVE,
            1,
            &queueFamilyIndex
        };

        VkBuffer in_buffer;
        VK_CHK(vkCreateBuffer(device, &bufferCreateInfo, 0, &in_buffer));
        VK_CHK(vkBindBufferMemory(device, in_buffer, memory, 0));

        VkBuffer out_buffer;
        VK_CHK(vkCreateBuffer(device, &bufferCreateInfo, 0, &out_buffer));
        VK_CHK(vkBindBufferMemory(device, out_buffer, memory, bufferSize));

        // read-in SPIR-V binary from CL compilation
        int spv_len = 0;
        char *spv_shader = NULL;
        {
            struct stat buffer;
            int         status;

            status = stat(argv[1], &buffer);
            if(stat(argv[1], &buffer) == 0){
                printf("spir-v size %ld\n", buffer.st_size);
                spv_shader = (char*)malloc(buffer.st_size);
                spv_len = buffer.st_size;
                FILE *fptr = fopen(argv[1], "rb");
                fread(spv_shader, 1, spv_len, fptr);
                fclose(fptr);
            }else{
                printf("open shader file failed\n");
            }
        }
        VkShaderModuleCreateInfo shaderModuleCreateInfo = {
            VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            0,
            0,
            #if 0
            sizeof(shader),
            (unsigned int*)(&shader)
            #else
            spv_len,
            (unsigned int*)(spv_shader)
            #endif
        };
        VkShaderModule shader_module;
        VK_CHK(vkCreateShaderModule(device, &shaderModuleCreateInfo, 0, &shader_module));

        VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[2] = {
            {
                0,
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                1,
                VK_SHADER_STAGE_COMPUTE_BIT,
                0
            },
            {
                1,
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                1,
                VK_SHADER_STAGE_COMPUTE_BIT,
                0
            }
        };
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            0,
            0,
            2,
            descriptorSetLayoutBindings
        };
        VkDescriptorSetLayout descriptorSetLayout;
        VK_CHK(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, 0, &descriptorSetLayout));

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            0,
            0,
            1,
            &descriptorSetLayout,
            0,
            0
        };
        VkPipelineLayout pipelineLayout;
        VK_CHK(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, 0, &pipelineLayout));

        VkComputePipelineCreateInfo computePipelineCreateInfo = {
            VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            0,
            0,
            {
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                0,
                0,
                VK_SHADER_STAGE_COMPUTE_BIT,
                shader_module,
                "vkcl",
                0
            },
            pipelineLayout,
            0,
            0
        };
        VkPipeline pipeline;
        VK_CHK(vkCreateComputePipelines(device, 0, 1, &computePipelineCreateInfo, 0, &pipeline));

        VkDescriptorPoolSize descriptorPoolSize = {
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            2
        };
        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            0,
            0,
            1,
            1,
            &descriptorPoolSize
        };
        VkDescriptorPool descriptorPool;
        VK_CHK(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, 0, &descriptorPool));

        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            0,
            descriptorPool,
            1,
            &descriptorSetLayout
        };
        VkDescriptorSet descriptorSet;
        VK_CHK(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet));

        VkDescriptorBufferInfo in_descriptorBufferInfo = {
            in_buffer,
            0,
            VK_WHOLE_SIZE
        };
        VkDescriptorBufferInfo out_descriptorBufferInfo = {
            out_buffer,
            0,
            VK_WHOLE_SIZE
        };
        VkWriteDescriptorSet writeDescriptorSet[2] = {
            {
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                0,
                descriptorSet,
                0,
                0,
                1,
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                0,
                &in_descriptorBufferInfo,
                0
            },
            {
                VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                0,
                descriptorSet,
                1,
                0,
                1,
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                0,
                &out_descriptorBufferInfo,
                0
            }
        };
        vkUpdateDescriptorSets(device, 2, writeDescriptorSet, 0, 0);

        VkCommandPoolCreateInfo commandPoolCreateInfo = {
            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            0,
            0,
            queueFamilyIndex
        };
        VkCommandPool commandPool;
        VK_CHK(vkCreateCommandPool(device, &commandPoolCreateInfo, 0, &commandPool));

        VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            0,
            commandPool,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            1
        };
        VkCommandBuffer commandBuffer;
        VK_CHK(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer));

        VkCommandBufferBeginInfo commandBufferBeginInfo = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            0,
            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            0
        };

        VK_CHK(vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo));
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, 0);
        vkCmdDispatch(commandBuffer, bufferSize / sizeof(int32_t), 1, 1);
        VK_CHK(vkEndCommandBuffer(commandBuffer));

        VkQueue queue;
        vkGetDeviceQueue(device, queueFamilyIndex, 0, &queue);

        VkSubmitInfo submitInfo = {
            VK_STRUCTURE_TYPE_SUBMIT_INFO,
            0,
            0,
            0,
            0,
            1,
            &commandBuffer,
            0,
            0
        };
        VK_CHK(vkQueueSubmit(queue, 1, &submitInfo, 0));
        VK_CHK(vkQueueWaitIdle(queue));

        VK_CHK(vkMapMemory(device, memory, 0, memorySize, 0, (void *)&payload));
        for (uint32_t k = 0, e = bufferSize / sizeof(int32_t); k < e; k++) {
            VK_CHK(payload[k + e] == payload[k] ? VK_SUCCESS : VK_ERROR_VALIDATION_FAILED_EXT);
        }
        vkUnmapMemory(device, memory);

        free(spv_shader);
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        vkDestroyCommandPool(device, commandPool, NULL);
        vkDestroyDescriptorPool(device, descriptorPool, NULL);
        vkDestroyPipeline(device, pipeline, NULL);
        vkDestroyPipelineLayout(device, pipelineLayout, NULL);
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, NULL);
        vkDestroyShaderModule(device, shader_module, NULL);
        vkDestroyBuffer(device, out_buffer, NULL);
        vkDestroyBuffer(device, in_buffer, NULL);
        vkDestroyDevice(device, NULL);
    }

    free(physicalDevices);

    printf("DONE\n");
    return 0;
}
