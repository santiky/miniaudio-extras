
/*
void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    MA_ASSERT(pDevice->capture.format == pDevice->playback.format);
    MA_ASSERT(pDevice->capture.channels == pDevice->playback.channels);
    ma_audio_buffer_ref_set_data(&g_exciteData, pInput, frameCount);
    ma_node_graph_read_pcm_frames(&g_nodeGraph, pOutput, frameCount, NULL);
}

    result = ma_audio_buffer_ref_init(device.capture.format, device.capture.channels, NULL, 0, &g_exciteData);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize audio buffer for source.");

    exciteNodeConfig = ma_data_source_node_config_init(&g_exciteData);

    result = ma_data_source_node_init(&g_nodeGraph, &exciteNodeConfig, NULL, &g_exciteNode);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize source node.");
        goto done2;
    }

    ma_node_attach_output_bus(&g_exciteNode, 0, &g_vocoderNode, 1);
*/