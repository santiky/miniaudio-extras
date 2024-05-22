
#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_ENABLE_JACK
#define MA_ENABLE_PULSE
#define MA_NO_GENERATION
#define MA_DEBUG_OUTPUT
#define MA_LOG_LEVEL_DEBUG DEBUG
#define MINIAUDIO_IMPLEMENTATION
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "ma_writer_node.h"
#include <unistd.h>
#include <stdio.h>
#include <math.h>

#define FORMAT      ma_format_f32
#define CHANNELS    2
#define SAMPLE_RATE 48000

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    //MA_ASSERT(pDevice->capture.format == pDevice->playback.format);
    //MA_ASSERT(pDevice->capture.channels == pDevice->playback.channels);
    ma_engine_read_pcm_frames((ma_engine*)pDevice->pUserData, pOutput, frameCount, NULL);
}

int main(int argc, char *argv[])
{
    ma_result result;
    ma_device_config deviceConfig;
    ma_device device1;
	ma_device device2;	
    ma_engine_config engineConfig;
    ma_writer_node_config writerNodeConfig;
	ma_engine engine1;
	ma_engine engine2;
    ma_data_source_rb   dataSourceRB;
    ma_data_source_node dataSupplyNode;
    ma_writer_node writer;
    ma_pcm_rb buffer;
    ma_sound sound;
    ma_context context;
    ma_device_info* pPlaybackDeviceInfos;
    ma_uint32 playbackDeviceCount;
    
    result = ma_context_init(NULL, 0, NULL, &context);
    if (result != MA_SUCCESS) {
        printf("ERROR: Failed to initialize audio context.\n");
	    return result;
    }
    result = ma_context_get_devices(&context, &pPlaybackDeviceInfos, &playbackDeviceCount, NULL, NULL);
    if (result != MA_SUCCESS) {
        printf("ERROR: Failed to enumerate audio devices.\n");
        ma_context_uninit(&context);
        return result;
    }
    printf("Audio devices available:\n");
    for (ma_uint32 iAvailableDevice = 0; iAvailableDevice < playbackDeviceCount; iAvailableDevice += 1) {
        printf("%u : %s\n", iAvailableDevice, pPlaybackDeviceInfos[iAvailableDevice].name);
    }
    deviceConfig = ma_device_config_init(ma_device_type_duplex);
    deviceConfig.capture.pDeviceID  = &pPlaybackDeviceInfos[4].id; 
    deviceConfig.capture.format     = FORMAT;
    deviceConfig.capture.channels   = CHANNELS;
    deviceConfig.capture.shareMode  = ma_share_mode_shared;
    deviceConfig.playback.pDeviceID = &pPlaybackDeviceInfos[4].id; // change this four your device id
    deviceConfig.playback.format    = FORMAT;
    deviceConfig.playback.channels  = CHANNELS;
    deviceConfig.sampleRate			= SAMPLE_RATE;
    deviceConfig.dataCallback       = data_callback;
    deviceConfig.pUserData          = &engine1;
    result = ma_device_init(NULL, &deviceConfig, &device1);
    if (result != MA_SUCCESS) {
        return result;
    }
    engineConfig                  = ma_engine_config_init();
	engineConfig.pDevice          = &device1;
    engineConfig.noAutoStart      = MA_TRUE;
    result = ma_engine_init(&engineConfig, &engine1);
    if (result != MA_SUCCESS) {
        printf("Error Failed to initialize audio engine.\n");
        return 1;
    }
    deviceConfig.capture.pDeviceID  = &pPlaybackDeviceInfos[3].id; // change this four your device id
    deviceConfig.playback.pDeviceID  = &pPlaybackDeviceInfos[3].id; // change this four your device id
    deviceConfig.pUserData          = &engine2;    
    result = ma_device_init(NULL, &deviceConfig, &device2);
    if (result != MA_SUCCESS) {
        return result;
    }
	engineConfig.pDevice          = &device2;
    result = ma_engine_init(&engineConfig, &engine2);
    if (result != MA_SUCCESS) {
        printf("ERROR: Failed to initialize audio engine.\n");
        return 1;
    }
    result = ma_pcm_rb_init(FORMAT, CHANNELS, SAMPLE_RATE, NULL, NULL, &buffer);
    if (result != MA_SUCCESS) {
           printf("ERROR: to initialize ring buffer.\n");
           return result;
     }
     ma_silence_pcm_frames(buffer.rb.pBuffer, SAMPLE_RATE, FORMAT, CHANNELS);
     ma_writer_node_config writerConfig = ma_writer_node_config_init(CHANNELS, SAMPLE_RATE, &buffer);
     result = ma_writer_node_init(ma_engine_get_node_graph(&engine1), &writerConfig, NULL, &writer);
     if (result != MA_SUCCESS) {
          printf("ERROR: Failed to init writer node.\n");
          return result;
     }
     result = ma_node_attach_output_bus(&writer, 0, ma_engine_get_endpoint(&engine1), 0);
     if (result != MA_SUCCESS) {
           printf("ERROR:  Failed to attach writer node.\n");
           return result;
     }
     result = ma_data_source_rb_init(&dataSourceRB, &buffer);
     if (result != MA_SUCCESS) {
           printf("ERROR: Failed to init data source ring buffer.\n");
           return result;
     }
     ma_data_source_node_config dataSupplyNodeConfig = ma_data_source_node_config_init(&dataSourceRB);
     result = ma_data_source_node_init(ma_engine_get_node_graph(&engine2), &dataSupplyNodeConfig, NULL, &dataSupplyNode);
     if (result != MA_SUCCESS) {
            printf("Error: Failed to init data source node.\n");
            return result;
     }
     result = ma_node_attach_output_bus(&dataSupplyNode, 0, ma_engine_get_endpoint(&engine2), 0);
     if (result != MA_SUCCESS) {
             printf("Error: Failed to attach data source rb node.\n");
             return result;
     }
    ma_node_attach_output_bus(&writer, 0, ma_engine_get_endpoint(&engine1), 0);
    result = ma_sound_init_from_file(&engine1, "test.flac", 0, NULL, NULL, &sound);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize sound.\n");
        return result;
    }
    ma_node_attach_output_bus(&sound, 0, &writer, 0);
	ma_sound_set_looping(&sound, MA_TRUE);
	ma_sound_set_volume(&sound, 0.4f);
    result = ma_engine_start(&engine1);
	if (result != MA_SUCCESS) {
        printf("Error Failed to start audio engine.\n");
        return 1;
    }
    result = ma_engine_start(&engine2);
    if (result != MA_SUCCESS) {
        printf("ERROR: Failed to start audio engine.\n");
        return 1;
    }
	ma_sound_start(&sound);

    printf("Press Enter to quit...\n");
    getchar();

    ma_engine_stop(&engine2);
	ma_engine_stop(&engine1);    
    ma_sound_uninit(&sound);
    ma_data_source_node_uninit(&dataSupplyNode, NULL);    
    ma_writer_node_uninit(&writer, NULL);
    ma_pcm_rb_uninit(&buffer);
	ma_engine_uninit(&engine2);
	ma_engine_uninit(&engine1);
    ma_device_uninit(&device2);
    ma_device_uninit(&device1);
	ma_context_uninit(&context);

    (void)argc;
    (void)argv;
    return 0;
}
