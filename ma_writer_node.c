#include "ma_writer_node.h"

MA_API ma_writer_node_config ma_writer_node_config_init(ma_uint32 channels, ma_uint32 bufferSizeInFrames, ma_pcm_rb *rb)
{
    ma_writer_node_config config;

    //MA_ZERO_OBJECT(&config);
    config.nodeConfig = ma_node_config_init();
    config.channels   = channels;
    config.bufferSizeInFrames = bufferSizeInFrames;
    config.pBuffer = rb;

    return config;
}

static void ma_writer_node_process_pcm_frames(ma_node* pNode, const float** ppFramesIn, ma_uint32* pFrameCountIn, float** ppFramesOut, ma_uint32* pFrameCountOut)
{
    ma_writer_node* pWriteNode = (ma_writer_node*)pNode;

    //MA_ASSERT(pWriteNode != NULL);
    //MA_ASSERT(ma_node_get_input_bus_count(&pWriteNode->baseNode) == 2);

    if (*pFrameCountIn > 0) {
        void *pWriteBuffer = NULL;
        ma_pcm_rb_acquire_write(pWriteNode->pBuffer, pFrameCountIn, &pWriteBuffer);
        if (pWriteBuffer != NULL) {
            ma_copy_pcm_frames(pWriteBuffer, ppFramesIn[0], *pFrameCountIn, ma_format_f32, pWriteNode->channels);
            ma_pcm_rb_commit_write(pWriteNode->pBuffer, *pFrameCountIn);
        }
    }
    ma_copy_pcm_frames(ppFramesOut[0], ppFramesIn[0], *pFrameCountOut, ma_format_f32, pWriteNode->channels);
}

static ma_node_vtable g_ma_writer_node_vtable =
{
    ma_writer_node_process_pcm_frames,
    NULL,
    2,
    1,
    0
};

MA_API ma_result ma_writer_node_init(ma_node_graph* pNodeGraph, const ma_writer_node_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ma_writer_node* pWriteNode)
{
    ma_result result;
    ma_node_config baseConfig;
    ma_uint32 inputChannels[2];
    ma_uint32 outputChannels[1];

    if (pWriteNode == NULL || pConfig == NULL || pConfig->pBuffer == NULL \
                           || (pConfig->channels > MA_MAX_NODE_BUS_COUNT) ) {
        return MA_INVALID_ARGS;
    }

    //MA_ZERO_OBJECT(pWriteNode);
    inputChannels[0]  = pConfig->channels;
    inputChannels[1]  = pConfig->channels;
    outputChannels[0] = pConfig->channels;
    baseConfig = pConfig->nodeConfig;
    baseConfig.vtable          = &g_ma_writer_node_vtable;
    baseConfig.pInputChannels  = inputChannels;
    baseConfig.pOutputChannels = outputChannels;

    result = ma_node_init(pNodeGraph, &baseConfig, pAllocationCallbacks, &pWriteNode->baseNode);
    if (result != MA_SUCCESS) {
        return result;
    }
	pWriteNode->bufferSizeInFrames = pConfig->bufferSizeInFrames;
	pWriteNode->pBuffer = pConfig->pBuffer;
	pWriteNode->channels = pConfig->channels;
	return MA_SUCCESS;
}

MA_API void ma_writer_node_uninit(ma_writer_node* pWriteNode, const ma_allocation_callbacks* pAllocationCallbacks)
{
    ma_node_uninit(&pWriteNode->baseNode, pAllocationCallbacks);
}

/*
 * Data Source Ring Buffer
 */

ma_result ma_data_source_rb_read(ma_data_source* pDataSource, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead)
{
     ma_data_source_rb* ds = (ma_data_source_rb*)pDataSource;

    ma_uint32 pcmFramesAvailableInRB = 0;
    ma_uint32 pcmFramesProcessed = 0;
    while (pcmFramesProcessed < frameCount) {
        pcmFramesAvailableInRB = ma_pcm_rb_available_read(ds->rb);
        if (pcmFramesAvailableInRB == 0) {
            break;
        }
        ma_uint32 framesToRead = frameCount - pcmFramesProcessed;
        if (framesToRead > pcmFramesAvailableInRB) {
            framesToRead = pcmFramesAvailableInRB;
        }
        void* pReadBuffer = NULL;
        ma_pcm_rb_acquire_read(ds->rb, &framesToRead, &pReadBuffer);
        if (pReadBuffer != NULL) {
            ma_copy_pcm_frames(pFramesOut, pReadBuffer, framesToRead, ma_format_f32, 2);
            ma_pcm_rb_commit_read(ds->rb, framesToRead);
            pcmFramesProcessed += framesToRead;
        }
        else {
            break;
        }
    }
    *pFramesRead += pcmFramesProcessed;
    return MA_SUCCESS;
}

ma_result ma_data_source_rb_seek(ma_data_source* pDataSource, ma_uint64 frameIndex)
{
    (void)pDataSource;
    (void)frameIndex;
    return MA_NOT_IMPLEMENTED;
}

ma_result ma_data_source_rb_get_data_format(ma_data_source* pDataSource, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate, ma_channel* pChannelMap, size_t channelMapCap)
{
    (void)pDataSource;
    *pFormat = ma_format_f32;
    *pChannels = 2;
    *pSampleRate = ma_standard_sample_rate_48000;
    return MA_SUCCESS;
}

ma_result ma_data_source_rb_get_cursor(ma_data_source* pDataSource, ma_uint64* pCursor)
{
    (void)pDataSource;
    *pCursor  = 0;
    return MA_NOT_IMPLEMENTED;
}

ma_result ma_data_source_rb_get_length(ma_data_source* pDataSource, ma_uint64* pLength)
{
    (void)pDataSource;
    *pLength  = 0;
    return MA_NOT_IMPLEMENTED;
}

ma_data_source_vtable g_ma_data_source_rb_vtable =
{
    ma_data_source_rb_read,
    ma_data_source_rb_seek,
    ma_data_source_rb_get_data_format,
    ma_data_source_rb_get_cursor,
    ma_data_source_rb_get_length
};

ma_result ma_data_source_rb_init(ma_data_source_rb* pMyDataSource, ma_pcm_rb *ringBuffer)
{
    ma_result result;
    ma_data_source_config baseConfig;

    baseConfig = ma_data_source_config_init();
    baseConfig.vtable = &g_ma_data_source_rb_vtable;

    result = ma_data_source_init(&baseConfig, &pMyDataSource->base);
    if (result != MA_SUCCESS) {
        return result;
    }

    pMyDataSource->rb = ringBuffer;

    return MA_SUCCESS;
}

void ma_data_source_rb_uninit(ma_data_source_rb* pMyDataSource)
{
    ma_data_source_uninit(&pMyDataSource->base);
}

/*
 * vumeter
 */

MA_API ma_vumeter_node_config ma_vumeter_node_config_init(ma_uint32 channels, ma_uint32 sampleRate)
{
    ma_vumeter_node_config config;

    //MA_ZERO_OBJECT(&config);
    config.nodeConfig = ma_node_config_init();
    config.channels   = channels;
    config.sampleRate = sampleRate;

    return config;
}

static void ma_vumeter_node_process_pcm_frames(ma_node* pNode, const float** ppFramesIn, ma_uint32* pFrameCountIn, float** ppFramesOut, ma_uint32* pFrameCountOut)
{
    ma_vumeter_node* pVumeterNode = (ma_vumeter_node*)pNode;

    //MA_ASSERT(pVumeterNode != NULL);
    //MA_ASSERT(ma_node_get_input_bus_count(&pVumeterNode->baseNode) == 1);

    for (ma_uint32 i = 0; i < *pFrameCountIn; i++) {
        float input = fabsf(ppFramesIn[0][i]);
        pVumeterNode->level += pVumeterNode->alpha * (input - pVumeterNode->level);
    }
    //ma_copy_pcm_frames(ppFramesOut[0], ppFramesIn[0], *pFrameCountOut, ma_format_f32, pVumeterNode->channels);
    //*pFrameCountIn = 0; // uncoment this and framesIn will not be refreshed
    //*pFrameCountOut = 0; // uncoment this and the engine does not sound
    (void)ppFramesOut;
}

static ma_node_vtable g_ma_vumeter_node_vtable =
{
    ma_vumeter_node_process_pcm_frames,
    NULL,
    1,
    1,
//    0
//    MA_NODE_FLAG_CONTINUOUS_PROCESSING
    MA_NODE_FLAG_SILENT_OUTPUT
};

MA_API ma_result ma_vumeter_node_init(ma_node_graph* pNodeGraph, const ma_vumeter_node_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ma_vumeter_node* pVumeterNode)
{
    ma_result result;
    ma_node_config baseConfig;
    ma_uint32 inputChannels[1];
    ma_uint32 outputChannels[1];

    if (pVumeterNode == NULL || pConfig == NULL \
                           || (pConfig->channels > MA_MAX_NODE_BUS_COUNT) ) {
        return MA_INVALID_ARGS;
    }

    //MA_ZERO_OBJECT(pVumeterNode);
    inputChannels[0]  = pConfig->channels;
    outputChannels[0] = pConfig->channels;
    baseConfig = pConfig->nodeConfig;
    baseConfig.vtable          = &g_ma_vumeter_node_vtable;
    baseConfig.pInputChannels  = inputChannels;
    baseConfig.pOutputChannels = outputChannels;

    result = ma_node_init(pNodeGraph, &baseConfig, pAllocationCallbacks, &pVumeterNode->baseNode);
    if (result != MA_SUCCESS) {
            return result;
    }
    pVumeterNode->sampleRate = pConfig->sampleRate;
    pVumeterNode->channels = pConfig->channels;
    pVumeterNode->level = 0;
    pVumeterNode->TC = 0.010f;
    pVumeterNode->alpha = 1.0 - expf( (-2.0 * M_PI) / (pVumeterNode->TC * 48000.0f) );
    return MA_SUCCESS;
}

MA_API void ma_vumeter_node_uninit(ma_vumeter_node* pVumeterNode, const ma_allocation_callbacks* pAllocationCallbacks)
{
    ma_node_uninit(&pVumeterNode->baseNode, pAllocationCallbacks);
}

MA_API float ma_vumeter_node_get_level(ma_vumeter_node* pVumeterNode) { return pVumeterNode->level; };
