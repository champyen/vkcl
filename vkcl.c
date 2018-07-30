
#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "vkcl.h"

#define TEST_BUFLEN 16384

enum {
    RESERVED_ID = 0,
    FUNC_ID,
    IN_ID,
    OUT_ID,
    GLOBAL_INVOCATION_ID,
    VOID_TYPE_ID,
    FUNC_TYPE_ID,
    INT_TYPE_ID,
    INT_ARRAY_TYPE_ID,
    STRUCT_ID,
    POINTER_TYPE_ID,
    ELEMENT_POINTER_TYPE_ID,
    INT_VECTOR_TYPE_ID,
    INT_VECTOR_POINTER_TYPE_ID,
    INT_POINTER_TYPE_ID,
    CONSTANT_ZERO_ID,
    CONSTANT_ARRAY_LENGTH_ID,
    LABEL_ID,
    IN_ELEMENT_ID,
    OUT_ELEMENT_ID,
    GLOBAL_INVOCATION_X_ID,
    GLOBAL_INVOCATION_X_PTR_ID,
    TEMP_LOADED_ID,
    BOUND
};

enum {
    INPUT = 1,
    UNIFORM = 2,
    BUFFER_BLOCK = 3,
    ARRAY_STRIDE = 6,
    BUILTIN = 11,
    BINDING = 33,
    OFFSET = 35,
    DESCRIPTOR_SET = 34,
    GLOBAL_INVOCATION = 28,
    OP_TYPE_VOID = 19,
    OP_TYPE_FUNCTION = 33,
    OP_TYPE_INT = 21,
    OP_TYPE_VECTOR = 23,
    OP_TYPE_ARRAY = 28,
    OP_TYPE_STRUCT = 30,
    OP_TYPE_POINTER = 32,
    OP_VARIABLE = 59,
    OP_DECORATE = 71,
    OP_MEMBER_DECORATE = 72,
    OP_FUNCTION = 54,
    OP_LABEL = 248,
    OP_ACCESS_CHAIN = 65,
    OP_CONSTANT = 43,
    OP_LOAD = 61,
    OP_STORE = 62,
    OP_RETURN = 253,
    OP_FUNCTION_END = 56,
    OP_CAPABILITY = 17,
    OP_MEMORY_MODEL = 14,
    OP_ENTRY_POINT = 15,
    OP_EXECUTION_MODE = 16,
    OP_COMPOSITE_EXTRACT = 81,
};

int32_t shader[] = {
    // first is the SPIR-V header
    0x07230203, // magic header ID
    0x00010000, // version 1.0.0
    0,          // generator (optional)
    BOUND,      // bound
    0,          // schema

    // OpCapability Shader
    (2 << 16) | OP_CAPABILITY, 1,

    // OpMemoryModel Logical Simple
    (3 << 16) | OP_MEMORY_MODEL, 0, 0,

    // OpEntryPoint GLCompute %FUNC_ID "f" %IN_ID %OUT_ID
    (5 << 16) | OP_ENTRY_POINT, 5, FUNC_ID, 0x6C636B76, 0,

    // OpExecutionMode %FUNC_ID LocalSize 1 1 1
    (6 << 16) | OP_EXECUTION_MODE, FUNC_ID, 17, 1, 1, 1,

    // next declare decorations
    (3 << 16) | OP_DECORATE, STRUCT_ID, BUFFER_BLOCK,
    (4 << 16) | OP_DECORATE, GLOBAL_INVOCATION_ID, BUILTIN, GLOBAL_INVOCATION,
    (4 << 16) | OP_DECORATE, IN_ID, DESCRIPTOR_SET, 0,
    (4 << 16) | OP_DECORATE, IN_ID, BINDING, 0,
    (4 << 16) | OP_DECORATE, OUT_ID, DESCRIPTOR_SET, 0,
    (4 << 16) | OP_DECORATE, OUT_ID, BINDING, 1,
    (4 << 16) | OP_DECORATE, INT_ARRAY_TYPE_ID, ARRAY_STRIDE, 4,
    (5 << 16) | OP_MEMBER_DECORATE, STRUCT_ID, 0, OFFSET, 0,

    // next declare types
    (2 << 16) | OP_TYPE_VOID, VOID_TYPE_ID,
    (3 << 16) | OP_TYPE_FUNCTION, FUNC_TYPE_ID, VOID_TYPE_ID,
    (4 << 16) | OP_TYPE_INT, INT_TYPE_ID, 32, 1,
    (4 << 16) | OP_CONSTANT, INT_TYPE_ID, CONSTANT_ARRAY_LENGTH_ID, TEST_BUFLEN,
    (4 << 16) | OP_TYPE_ARRAY, INT_ARRAY_TYPE_ID, INT_TYPE_ID, CONSTANT_ARRAY_LENGTH_ID,
    (3 << 16) | OP_TYPE_STRUCT, STRUCT_ID, INT_ARRAY_TYPE_ID,
    (4 << 16) | OP_TYPE_POINTER, POINTER_TYPE_ID, UNIFORM, STRUCT_ID,
    (4 << 16) | OP_TYPE_POINTER, ELEMENT_POINTER_TYPE_ID, UNIFORM, INT_TYPE_ID,
    (4 << 16) | OP_TYPE_VECTOR, INT_VECTOR_TYPE_ID, INT_TYPE_ID, 3,
    (4 << 16) | OP_TYPE_POINTER, INT_VECTOR_POINTER_TYPE_ID, INPUT, INT_VECTOR_TYPE_ID,
    (4 << 16) | OP_TYPE_POINTER, INT_POINTER_TYPE_ID, INPUT, INT_TYPE_ID,

    // then declare constants
    (4 << 16) | OP_CONSTANT, INT_TYPE_ID, CONSTANT_ZERO_ID, 0,

    // then declare variables
    (4 << 16) | OP_VARIABLE, POINTER_TYPE_ID, IN_ID, UNIFORM,
    (4 << 16) | OP_VARIABLE, POINTER_TYPE_ID, OUT_ID, UNIFORM,
    (4 << 16) | OP_VARIABLE, INT_VECTOR_POINTER_TYPE_ID, GLOBAL_INVOCATION_ID, INPUT,

    // then declare function
    (5 << 16) | OP_FUNCTION, VOID_TYPE_ID, FUNC_ID, 0, FUNC_TYPE_ID,
    (2 << 16) | OP_LABEL, LABEL_ID,
    (5 << 16) | OP_ACCESS_CHAIN, INT_POINTER_TYPE_ID, GLOBAL_INVOCATION_X_PTR_ID, GLOBAL_INVOCATION_ID, CONSTANT_ZERO_ID,
    (4 << 16) | OP_LOAD, INT_TYPE_ID, GLOBAL_INVOCATION_X_ID, GLOBAL_INVOCATION_X_PTR_ID,
    (6 << 16) | OP_ACCESS_CHAIN, ELEMENT_POINTER_TYPE_ID, IN_ELEMENT_ID, IN_ID, CONSTANT_ZERO_ID, GLOBAL_INVOCATION_X_ID,
    (4 << 16) | OP_LOAD, INT_TYPE_ID, TEMP_LOADED_ID, IN_ELEMENT_ID,
    (6 << 16) | OP_ACCESS_CHAIN, ELEMENT_POINTER_TYPE_ID, OUT_ELEMENT_ID, OUT_ID, CONSTANT_ZERO_ID, GLOBAL_INVOCATION_X_ID,
    (3 << 16) | OP_STORE, OUT_ELEMENT_ID, TEMP_LOADED_ID,
    (1 << 16) | OP_RETURN,
    (1 << 16) | OP_FUNCTION_END,
};


VkResult vkGetBestComputeQueueNPH(VkPhysicalDevice physicalDevice, uint32_t* qfam_idx) {
    uint32_t queueFamilyPropertiesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, 0);

    VkQueueFamilyProperties* const queueFamilyProperties = (VkQueueFamilyProperties*)alloca(sizeof(VkQueueFamilyProperties) * queueFamilyPropertiesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertiesCount, queueFamilyProperties);

    // first try and find a queue that has just the compute bit set
    for (uint32_t i = 0; i < queueFamilyPropertiesCount; i++) {
        // mask out the sparse binding bit that we aren't caring about (yet!) and the transfer bit
        const VkQueueFlags maskedFlags = (~(VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT) & queueFamilyProperties[i].queueFlags);
        if (!(VK_QUEUE_GRAPHICS_BIT & maskedFlags) && (VK_QUEUE_COMPUTE_BIT & maskedFlags)) {
            *qfam_idx = i;
            return VK_SUCCESS;
        }
    }

    // lastly get any queue that'll work for us
    for (uint32_t i = 0; i < queueFamilyPropertiesCount; i++) {
        // mask out the sparse binding bit that we aren't caring about (yet!) and the transfer bit
        const VkQueueFlags maskedFlags = (~(VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT) & queueFamilyProperties[i].queueFlags);
        if (VK_QUEUE_COMPUTE_BIT & maskedFlags) {
            *qfam_idx = i;
            return VK_SUCCESS;
        }
    }

    return VK_ERROR_INITIALIZATION_FAILED;
}

void vkcl_init(vkcl_context *ctx)
{
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

    VK_CHK(vkCreateInstance(&instanceCreateInfo, 0, &ctx->instance));

    uint32_t physicalDeviceCount = 0;
    VK_CHK(vkEnumeratePhysicalDevices(ctx->instance, &physicalDeviceCount, 0));
    printf("%d Vulkan Devices\n", physicalDeviceCount);
    if(physicalDeviceCount > 1)
        physicalDeviceCount = 1;

    //VkPhysicalDevice* const physicalDevices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);
    VK_CHK(vkEnumeratePhysicalDevices(ctx->instance, &physicalDeviceCount, &ctx->phy_dev));

    VK_CHK(vkGetBestComputeQueueNPH(ctx->phy_dev, &ctx->qfam_idx));
    const float queuePrioritory = 1.0f;
    const VkDeviceQueueCreateInfo deviceQueueCreateInfo = {
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        0,
        0,
        ctx->qfam_idx,
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

    VK_CHK(vkCreateDevice(ctx->phy_dev, &deviceCreateInfo, 0, &ctx->dev));
    vkGetPhysicalDeviceMemoryProperties(ctx->phy_dev, &ctx->phy_mem_prop);

    vkGetDeviceQueue(ctx->dev, ctx->qfam_idx, 0, &ctx->queue);
}

void vkcl_end(vkcl_context *ctx)
{
    vkDestroyDevice(ctx->dev, NULL);
}

void vkcl_waitfinish(vkcl_context *ctx, VkCommandBuffer *commandBuffer)
{
    VkSubmitInfo submitInfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        0,
        0,
        0,
        0,
        1,
        commandBuffer,
        0,
        0
    };
    VK_CHK(vkQueueSubmit(ctx->queue, 1, &submitInfo, 0));
    VK_CHK(vkQueueWaitIdle(ctx->queue));
}

vkcl_memory *vkcl_memory_allocate(vkcl_context *ctx, uint32_t flags, VkDeviceSize size)
{
    vkcl_memory *mem = (vkcl_memory*)malloc(sizeof(vkcl_memory));
    mem->ctx = ctx;

    uint32_t memoryTypeIndex = VK_MAX_MEMORY_TYPES;
    for (uint32_t k = 0; k < ctx->phy_mem_prop.memoryTypeCount; k++) {
        if ((flags & ctx->phy_mem_prop.memoryTypes[k].propertyFlags) == flags &&
        (size < ctx->phy_mem_prop.memoryHeaps[ctx->phy_mem_prop.memoryTypes[k].heapIndex].size)) {
            memoryTypeIndex = k;
            break;
        }
    }

    VK_CHK(memoryTypeIndex == VK_MAX_MEMORY_TYPES ? VK_ERROR_OUT_OF_HOST_MEMORY : VK_SUCCESS);
    mem->size = size;
    mem->type = memoryTypeIndex;

    const VkMemoryAllocateInfo memoryAllocateInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        0,
        size,
        memoryTypeIndex
    };
    VK_CHK(vkAllocateMemory(ctx->dev, &memoryAllocateInfo, 0, &mem->memory));

    return mem;
}

void vkcl_memory_free(vkcl_memory *mem)
{
    if(mem){
        vkcl_context *ctx = mem->ctx;
        if(mem->ptr)
            vkUnmapMemory(ctx->dev, mem->memory);
        vkFreeMemory(ctx->dev, mem->memory, NULL);
    }
}

void *vkcl_memory_map(vkcl_memory *mem)
{
    if(mem->ptr == NULL){
        vkcl_context *ctx = mem->ctx;
        VK_CHK(vkMapMemory(ctx->dev, mem->memory, 0, mem->size, 0, (void *)&mem->ptr));
    }
    return mem->ptr;
}

void vkcl_memory_unmap(vkcl_memory *mem)
{
    if(mem->ptr != NULL){
        vkcl_context *ctx = mem->ctx;
        vkUnmapMemory(ctx->dev, mem->memory);
        mem->ptr = NULL;
    }
}

vkcl_descset* vkcl_descset_create(vkcl_context *ctx, uint32_t set_id)
{
    vkcl_descset *set = (vkcl_descset*)malloc(sizeof(vkcl_descset));
    set->set_id = set_id;
    return set;
}

vkcl_buffer *vkcl_buffer_create(vkcl_context *ctx, uint32_t binding, uint32_t size, vkcl_memory *mem, uint32_t offset)
{
    vkcl_buffer *buf = (vkcl_buffer*)calloc(1, sizeof(vkcl_buffer));
    const VkBufferCreateInfo bufferCreateInfo = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        0,
        0,
        size,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        1,
        &ctx->qfam_idx
    };
    VK_CHK(vkCreateBuffer(ctx->dev, &bufferCreateInfo, 0, &buf->buffer));
    VK_CHK(vkBindBufferMemory(ctx->dev, buf->buffer, mem->memory, offset));

    buf->binding.binding = binding;
    buf->binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    buf->binding.descriptorCount = 1;
    buf->binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    buf->binding.pImmutableSamplers = NULL;

    buf->pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    buf->pool_size.descriptorCount = 1;

    buf->desc_info.buffer = buf->buffer;
    buf->desc_info.offset = 0;
    buf->desc_info.range = VK_WHOLE_SIZE;

    buf->ctx = ctx;
    return buf;
}

void vkcl_buffer_destroy(vkcl_buffer *buf)
{
    if(buf){
        vkcl_context *ctx = buf->ctx;
        vkDestroyBuffer(ctx->dev, buf->buffer, NULL);
        free(buf);
    }
}

int main(int argc, char** argv) {

    vkcl_context ctx;
    vkcl_init(&ctx);
    /*
    for (uint32_t i = 0; i < physicalDeviceCount; i++) {
        uint32_t ctx.qfam_idx = 0;
        VK_CHK(vkGetBestComputeQueueNPH(physicalDevices[i], &ctx.qfam_idx));
        printf("%d %d\n", i, ctx.qfam_idx);
    }
    */
    {


        const int32_t bufferLength = TEST_BUFLEN;
        const uint32_t bufferSize = sizeof(int32_t) * bufferLength;
        // we are going to need two buffers from this one memory
        const VkDeviceSize memorySize = bufferSize * 2;
        // set memoryTypeIndex to an invalid entry in the properties.memoryTypes array
        vkcl_memory *mem = vkcl_memory_allocate(&ctx, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memorySize);

        int32_t *payload = vkcl_memory_map(mem);
        for (uint32_t k = 1; k < memorySize / sizeof(int32_t); k++) {
            payload[k] = rand();
        }
        vkcl_memory_unmap(mem);

        vkcl_buffer *in_buffer = vkcl_buffer_create(&ctx, 0, bufferSize, mem, 0);
        vkcl_buffer *out_buffer = vkcl_buffer_create(&ctx, 1, bufferSize, mem, bufferSize);

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
        VK_CHK(vkCreateShaderModule(ctx.dev, &shaderModuleCreateInfo, 0, &shader_module));
        free(spv_shader);

        VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[2] = {
            in_buffer->binding,
            out_buffer->binding
        };
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            0,
            0,
            2,
            descriptorSetLayoutBindings
        };
        VkDescriptorSetLayout descriptorSetLayout;
        VK_CHK(vkCreateDescriptorSetLayout(ctx.dev, &descriptorSetLayoutCreateInfo, 0, &descriptorSetLayout));

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
        VK_CHK(vkCreatePipelineLayout(ctx.dev, &pipelineLayoutCreateInfo, 0, &pipelineLayout));

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
        VK_CHK(vkCreateComputePipelines(ctx.dev, 0, 1, &computePipelineCreateInfo, 0, &pipeline));

        VkDescriptorPoolSize descriptorPoolSize[2] = {
            in_buffer->pool_size,
            out_buffer->pool_size
        };
        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            0,
            0,
            1,
            2,
            descriptorPoolSize
        };
        VkDescriptorPool descriptorPool;
        VK_CHK(vkCreateDescriptorPool(ctx.dev, &descriptorPoolCreateInfo, 0, &descriptorPool));

        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            0,
            descriptorPool,
            1,
            &descriptorSetLayout
        };
        VkDescriptorSet descriptorSet;
        VK_CHK(vkAllocateDescriptorSets(ctx.dev, &descriptorSetAllocateInfo, &descriptorSet));

        /*
        VkDescriptorPoolSize descriptorPoolSize[2] = {
            {
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                1
            },
            {
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                1
            }
        };
        */
        //TODO: wrapped in buffer struct
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
                &in_buffer->desc_info,
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
                &out_buffer->desc_info,
                0
            }
        };
        vkUpdateDescriptorSets(ctx.dev, 2, writeDescriptorSet, 0, 0);

        VkCommandPoolCreateInfo commandPoolCreateInfo = {
            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            0,
            0,
            ctx.qfam_idx
        };
        VkCommandPool commandPool;
        VK_CHK(vkCreateCommandPool(ctx.dev, &commandPoolCreateInfo, 0, &commandPool));

        VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            0,
            commandPool,
            VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            1
        };
        VkCommandBuffer commandBuffer;
        VK_CHK(vkAllocateCommandBuffers(ctx.dev, &commandBufferAllocateInfo, &commandBuffer));

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

        vkcl_waitfinish(&ctx, &commandBuffer);
        //VK_CHK(vkMapMemory(ctx.dev, mem->memory, 0, memorySize, 0, (void *)&payload));
        payload = vkcl_memory_map(mem);
        for (uint32_t k = 0, e = bufferSize / sizeof(int32_t); k < e; k++) {
            VK_CHK(payload[k + e] == payload[k] ? VK_SUCCESS : VK_ERROR_VALIDATION_FAILED_EXT);
        }
        vkcl_memory_unmap(mem);

        vkFreeCommandBuffers(ctx.dev, commandPool, 1, &commandBuffer);
        vkDestroyCommandPool(ctx.dev, commandPool, NULL);
        vkDestroyDescriptorPool(ctx.dev, descriptorPool, NULL);
        vkDestroyPipeline(ctx.dev, pipeline, NULL);
        vkDestroyPipelineLayout(ctx.dev, pipelineLayout, NULL);
        vkDestroyDescriptorSetLayout(ctx.dev, descriptorSetLayout, NULL);
        vkDestroyShaderModule(ctx.dev, shader_module, NULL);
        //vkDestroyBuffer(ctx.dev, out_buffer, NULL);
        vkcl_buffer_destroy(out_buffer);
        //vkDestroyBuffer(ctx.dev, in_buffer, NULL);
        vkcl_buffer_destroy(in_buffer);
        //vkFreeMemory(ctx.dev, memory, NULL);
        vkcl_memory_free(mem);
        //vkDestroyDevice(ctx.dev, NULL);
        vkcl_end(&ctx);
    }

    //free(physicalDevices);

    printf("DONE\n");
    return 0;
}
