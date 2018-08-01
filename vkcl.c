
#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "vkcl.h"

#define TEST_BUFLEN 16384


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
    vkcl_descset *set = (vkcl_descset*)calloc(1, sizeof(vkcl_descset));
    set->set_id = set_id;
    return set;
}

void vkcl_descset_destroy(vkcl_descset *set)
{
    if(set){
        vkcl_context *ctx = set->ctx;
        //vkDestroyDescriptorPool(ctx->dev, set->descriptorPool, NULL);
        //vkDestroyDescriptorSetLayout(ctx->dev, set->descriptorSetLayout, NULL);
        free(set);
    }
}

vkcl_buffer *vkcl_buffer_create(vkcl_context *ctx, vkcl_descset *set, uint32_t size, vkcl_memory *mem, uint32_t offset)
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
    buf->set = set;

    /*
    buf->binding.binding = binding;
    buf->binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    buf->binding.descriptorCount = 1;
    buf->binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    buf->binding.pImmutableSamplers = NULL;

    buf->pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    buf->pool_size.descriptorCount = 1;
    */
    buf->desc_info.buffer = buf->buffer;
    buf->desc_info.offset = 0;
    buf->desc_info.range = VK_WHOLE_SIZE;

    uint32_t binding = set->bindings++;
    set->descriptorSetLayoutBindings[binding].binding = binding;
    set->descriptorSetLayoutBindings[binding].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    set->descriptorSetLayoutBindings[binding].descriptorCount = 1;
    set->descriptorSetLayoutBindings[binding].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    set->descriptorSetLayoutBindings[binding].pImmutableSamplers = NULL;

    set->descriptorPoolSize[binding].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    set->descriptorPoolSize[binding].descriptorCount = 1;

    VkWriteDescriptorSet writeDescriptorSet = {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        0,
        0, //descriptorSet,
        binding,
        0,
        1,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        0,
        &buf->desc_info,
        0
    };
    /*
    set->writeDescriptorSet[binding].buffer = buf->buffer;
    set->writeDescriptorSet[binding].offset = 0;
    set->writeDescriptorSet[binding].range = VK_WHOLE_SIZE;
    */
    set->writeDescriptorSet[binding] = writeDescriptorSet;

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

vkcl_pipeline* vkcl_pipeline_create(vkcl_context *ctx, vkcl_descset **sets, uint32_t num_set, char *spv_fname)
{
    vkcl_pipeline *pipeline = (vkcl_pipeline*)malloc(sizeof(vkcl_pipeline));
    // read-in SPIR-V binary from CL compilation
    int spv_len = 0;
    char *spv_shader = NULL;
    {
        struct stat buffer;
        int         status;

        status = stat(spv_fname, &buffer);
        if(stat(spv_fname, &buffer) == 0){
            printf("spir-v size %ld\n", buffer.st_size);
            spv_shader = (char*)malloc(buffer.st_size);
            spv_len = buffer.st_size;
            FILE *fptr = fopen(spv_fname, "rb");
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
        spv_len,
        (unsigned int*)(spv_shader)
    };
    VK_CHK(vkCreateShaderModule(ctx->dev, &shaderModuleCreateInfo, 0, &pipeline->shader_module));
    free(spv_shader);

    for(int i = 0; i < num_set; i++){
        vkcl_descset *set = sets[i];
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            0,
            0,
            set->bindings,
            set->descriptorSetLayoutBindings
        };
        //VkDescriptorSetLayout descriptorSetLayout;
        VK_CHK(vkCreateDescriptorSetLayout(ctx->dev, &descriptorSetLayoutCreateInfo, 0, &pipeline->descriptorSetLayouts[i]));
    }

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        0,
        0,
        num_set,
        pipeline->descriptorSetLayouts,
        0,
        0
    };
    VK_CHK(vkCreatePipelineLayout(ctx->dev, &pipelineLayoutCreateInfo, 0, &pipeline->pipelineLayout));

    VkComputePipelineCreateInfo computePipelineCreateInfo = {
        VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        0,
        0,
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            0,
            0,
            VK_SHADER_STAGE_COMPUTE_BIT,
            pipeline->shader_module,
            "vkcl",
            0
        },
        pipeline->pipelineLayout,
        0,
        0
    };
    VK_CHK(vkCreateComputePipelines(ctx->dev, 0, 1, &computePipelineCreateInfo, 0, &pipeline->pipeline));

    for(int i = 0; i < num_set; i++){
        vkcl_descset *set = sets[i];
        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            0,
            0,
            1,
            set->bindings,
            set->descriptorPoolSize
        };
        //VkDescriptorPool descriptorPool;
        VK_CHK(vkCreateDescriptorPool(ctx->dev, &descriptorPoolCreateInfo, 0, &pipeline->descriptorPools[i]));

        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            0,
            pipeline->descriptorPools[i],
            1,
            &pipeline->descriptorSetLayouts[i]
        };
        //VkDescriptorSet descriptorSet;
        VK_CHK(vkAllocateDescriptorSets(ctx->dev, &descriptorSetAllocateInfo, &pipeline->descriptorSets[i]));

        for(int j = 0; j < set->bindings; j++){
            set->writeDescriptorSet[j].dstSet = pipeline->descriptorSets[i];
            set->writeDescriptorSet[j].dstSet = pipeline->descriptorSets[i];
        }
        vkUpdateDescriptorSets(ctx->dev, set->bindings, set->writeDescriptorSet, 0, 0);
    }

    VkCommandPoolCreateInfo commandPoolCreateInfo = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        0,
        0,
        ctx->qfam_idx
    };
    VK_CHK(vkCreateCommandPool(ctx->dev, &commandPoolCreateInfo, 0, &pipeline->commandPool));

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        0,
        pipeline->commandPool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        1
    };
    VK_CHK(vkAllocateCommandBuffers(ctx->dev, &commandBufferAllocateInfo, &pipeline->commandBuffer));

    pipeline->sets = num_set;
    pipeline->ctx = ctx;

    return pipeline;
}

void vkcl_pipeline_destroy(vkcl_pipeline* pipeline)
{
    if(pipeline){
        vkcl_context *ctx = pipeline->ctx;
        vkFreeCommandBuffers(ctx->dev, pipeline->commandPool, 1, &pipeline->commandBuffer);
        vkDestroyCommandPool(ctx->dev, pipeline->commandPool, NULL);
        vkDestroyPipeline(ctx->dev, pipeline->pipeline, NULL);
        vkDestroyPipelineLayout(ctx->dev, pipeline->pipelineLayout, NULL);
        for(int i = 0; i < pipeline->sets; i++){
            vkDestroyDescriptorPool(ctx->dev, pipeline->descriptorPools[i], NULL);
            vkDestroyDescriptorSetLayout(ctx->dev, pipeline->descriptorSetLayouts[i], NULL);
        }
        vkDestroyShaderModule(ctx->dev, pipeline->shader_module, NULL);
        free(pipeline);
    }
}

void vkcl_pipeline_exec(vkcl_pipeline *pipeline, uint32_t x, uint32_t y, uint32_t z)
{
    vkcl_context *ctx = pipeline->ctx;
    VkCommandBufferBeginInfo commandBufferBeginInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        0,
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        0
    };
    VK_CHK(vkBeginCommandBuffer(pipeline->commandBuffer, &commandBufferBeginInfo));
    vkCmdBindPipeline(pipeline->commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->pipeline);
    vkCmdBindDescriptorSets(pipeline->commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->pipelineLayout, 0, pipeline->sets, pipeline->descriptorSets, 0, 0);
    vkCmdDispatch(pipeline->commandBuffer, x, y, z);
    VK_CHK(vkEndCommandBuffer(pipeline->commandBuffer));

    VkSubmitInfo submitInfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,
        0,
        0,
        0,
        0,
        1,
        &pipeline->commandBuffer,
        0,
        0
    };
    VK_CHK(vkQueueSubmit(ctx->queue, 1, &submitInfo, 0));
    VK_CHK(vkQueueWaitIdle(ctx->queue));
}

int main(int argc, char** argv) {

    vkcl_context ctx;
    vkcl_init(&ctx);
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

        vkcl_descset *set = vkcl_descset_create(&ctx, 0);
        vkcl_buffer *in_buffer = vkcl_buffer_create(&ctx, set, bufferSize, mem, 0);
        vkcl_buffer *out_buffer = vkcl_buffer_create(&ctx, set, bufferSize, mem, bufferSize);
        vkcl_pipeline *pipeline = vkcl_pipeline_create(&ctx, &set, 1, argv[1]);

        vkcl_pipeline_exec(pipeline, bufferSize / sizeof(int32_t), 1, 1);

        payload = vkcl_memory_map(mem);
        for (uint32_t k = 0, e = bufferSize / sizeof(int32_t); k < e; k++) {
            VK_CHK(payload[k + e] == payload[k] ? VK_SUCCESS : VK_ERROR_VALIDATION_FAILED_EXT);
        }
        vkcl_memory_unmap(mem);

        vkcl_pipeline_destroy(pipeline);
        vkcl_buffer_destroy(out_buffer);
        vkcl_buffer_destroy(in_buffer);
        vkcl_memory_free(mem);
        vkcl_end(&ctx);
    }

    printf("DONE\n");
    return 0;
}
