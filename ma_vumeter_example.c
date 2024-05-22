
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
    ma_engine_config engineConfig;
    ma_vumeter_node_config vumeterNodeConfig;
	ma_vumeter_node vumeter;
	ma_engine engine1;
    ma_sound sound;
    ma_splitter_node splitter;
    
    deviceConfig = ma_device_config_init(ma_device_type_duplex);
    deviceConfig.capture.pDeviceID  = NULL; 
    deviceConfig.capture.format     = FORMAT;
    deviceConfig.capture.channels   = CHANNELS;
    deviceConfig.capture.shareMode  = ma_share_mode_shared;
    deviceConfig.playback.pDeviceID = NULL;
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
        printf("Error Failed to initi audio engine.\n");
        return 1;
    }
    ma_vumeter_node_config vumeterConfig = ma_vumeter_node_config_init(CHANNELS, SAMPLE_RATE);
    result = ma_vumeter_node_init(ma_engine_get_node_graph(&engine1), &vumeterConfig, NULL, &vumeter);
    if (result != MA_SUCCESS) {
         printf("ERROR: Failed to init vumeter node.\n");
         return result;
    }
    result = ma_node_attach_output_bus(&vumeter, 0, ma_engine_get_endpoint(&engine1), 0);
    if (result != MA_SUCCESS) {
    	printf("ERROR:  Failed to attach vumeter node.\n");
        return result;
    }
    ma_splitter_node_config splitterConfig = ma_splitter_node_config_init(CHANNELS);
    splitterConfig.outputBusCount= 2;
    result = ma_splitter_node_init(ma_engine_get_node_graph(&engine1), &splitterConfig, NULL, &splitter);
    if (result != MA_SUCCESS) {
        printf("ERROR: Failed to init splitter node.\n");
        return result;
    }
    result = ma_node_attach_output_bus(&splitter, 0, ma_engine_get_endpoint(&engine1), 0);
    if (result != MA_SUCCESS) {
        printf("Failed to attach splitter to endpoint.\n");
        return result;
    }
    result = ma_node_attach_output_bus(&splitter, 1, &vumeter, 0);
    if (result != MA_SUCCESS) {
        printf("Failed to attach splitter to vumeter.\n");
        return result;
    }
    result = ma_sound_init_from_file(&engine1, "test.flac", 0, NULL, NULL, &sound);
    if (result != MA_SUCCESS) {
        printf("Failed to load sound.\n");
        return result;
    }
    result = ma_node_attach_output_bus(&sound, 0, &splitter, 0);
    if (result != MA_SUCCESS) {
        printf("Failed to attach sound.\n");
        return result;
    }
	ma_sound_set_looping(&sound, MA_TRUE);
	ma_sound_set_volume(&sound, 0.9f);
    result = ma_engine_start(&engine1);
	if (result != MA_SUCCESS) {
        printf("Error Failed to start audio engine.\n");
        return 1;
    }
	ma_sound_start(&sound);

	char c = '\0';
	while (c != 'q') {
	    printf("Press q to quit, another key to show level...\n");
	    c = getchar();
		printf("level : %f\n", ma_vumeter_node_get_level(&vumeter));
	}    
	ma_engine_stop(&engine1);    
    ma_sound_uninit(&sound);
    ma_vumeter_node_uninit(&vumeter, NULL);
	ma_engine_uninit(&engine1);
    ma_device_uninit(&device1);

    (void)argc;
    (void)argv;
    return 0;
}
