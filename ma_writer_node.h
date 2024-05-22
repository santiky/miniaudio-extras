/* Include ma_writer_node.h after miniaudio.h */
#ifndef ma_writer_node_h
#define ma_writer_node_h

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>
#include "miniaudio.h"

/*
 * writer
 */

typedef struct
{
    ma_node_config nodeConfig;
    ma_uint32 channels;
    ma_uint32 bufferSizeInFrames;
    ma_pcm_rb *pBuffer;
} ma_writer_node_config;

MA_API ma_writer_node_config ma_writer_node_config_init(ma_uint32 channels, ma_uint32 bufferSizeInFrames, ma_pcm_rb *rb);

typedef struct
{
    ma_node_base baseNode;
    ma_uint32 bufferSizeInFrames;
    ma_pcm_rb *pBuffer;
	ma_uint32 channels;
} ma_writer_node;

MA_API ma_result ma_writer_node_init(ma_node_graph* pNodeGraph, const ma_writer_node_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ma_writer_node* pWriteNode);
MA_API void ma_writer_node_uninit(ma_writer_node* pWriteNode, const ma_allocation_callbacks* pAllocationCallbacks);
/**
 * data source ring buffer
 */

typedef struct
{
    ma_data_source_base base;
    ma_pcm_rb *rb;
} ma_data_source_rb;

ma_result ma_data_source_rb_read(ma_data_source* pDataSource, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead);
ma_result ma_data_source_rb_seek(ma_data_source* pDataSource, ma_uint64 frameIndex);
ma_result ma_data_source_rb_get_data_format(ma_data_source* pDataSource, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate, ma_channel* pChannelMap, size_t channelMapCap);
ma_result ma_data_source_rb_get_cursor(ma_data_source* pDataSource, ma_uint64* pCursor);
ma_result ma_data_source_rb_get_length(ma_data_source* pDataSource, ma_uint64* pLength);
ma_result ma_data_source_rb_init(ma_data_source_rb* pMyDataSource, ma_pcm_rb *ringBuffer);
void ma_data_source_rb_uninit(ma_data_source_rb* pMyDataSource);


/*
 * VU meter
 */
typedef struct
{
    ma_node_config nodeConfig;
    ma_uint32 channels;
    ma_uint32 sampleRate;
} ma_vumeter_node_config;

MA_API ma_vumeter_node_config ma_vumeter_node_config_init(ma_uint32 channels, ma_uint32 sampleRate);

typedef struct
{
    ma_node_base baseNode;
    ma_uint32 channels;
    ma_uint32 sampleRate;
    float level;
    float TC;
    float alpha;
} ma_vumeter_node;

MA_API ma_result ma_vumeter_node_init(ma_node_graph* pNodeGraph, const ma_vumeter_node_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ma_vumeter_node* pVumeterNode);
MA_API void ma_vumeter_node_uninit(ma_vumeter_node* pVumeterNode, const ma_allocation_callbacks* pAllocationCallbacks);
MA_API float ma_vumeter_node_get_level(ma_vumeter_node* pVumeterNode);

#ifdef __cplusplus
}
#endif
#endif  /* ma_writer_node_h */
