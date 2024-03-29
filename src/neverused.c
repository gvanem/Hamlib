// These function are not referenced anywhere inside hamlib
// according to cppcheck
// Some are marked with XXREMOVEDXX in the code are are
// subject to being deprecated in a future release

void used_externally()
{
    uh_close_wkey(); // used by external programs
    uh_open_wkey();  // used by external programs
    rig_mem_count(); // referenced by rig.swg and tcl
}

void never_used()
{
    adat_mode_anr2rnr();
    adat_parse_vfo();
    adat_ptt_rnr2anr();
    agcToHamlib();
    agcToNative();
    amp_ext_level_foreach();
    amp_ext_lookup_tok();
    amp_ext_parm_foreach();
    amp_ext_token_lookup();
    amp_probe_all();
    amp_set_powerstat();
    amp_unregister();
    cm108_dcd_get();
    CtrlHandler();
    elad_close();
    elad_get_channel();
    elad_get_ctcss_sql();
    elad_get_ctcss_tone();
    elad_get_ext_parm();
    elad_get_info();
    elad_get_level();
    elad_get_mem();
    elad_get_mem_if();
    elad_get_mode_if();
    elad_open();
    elad_send_morse();
    elad_set_ant_no_ack();
    elad_set_channel();
    elad_set_ctcss_sql();
    elad_set_ctcss_tone();
    elad_set_ctcss_tone_tn();
    elad_set_ext_parm();
    elad_set_level();
    elad_set_mem();
    elad_set_ptt_safe();
    elad_set_split();
    elad_vfo_op();
    flushBuffer();
    foreach_opened_amp();
    foreach_opened_rot();
    getCalLevel();
    getFilterBW();
    getopt_long_only();
    get_output_freq();
    gp2000_get_func();
    gp2000_set_func();
    hzToDDS();
    hzToPBS();
    i2c_read();
    ic10_set_ptt();
    ic9100_get_level();
    ic9100_set_level();
    icmarine_cleanup();
    icmarine_get_conf();
    icmarine_get_dcd();
    icmarine_get_func();
    icmarine_get_level();
    icmarine_get_mode();
    icmarine_get_ptt();
    icmarine_get_split_vfo();
    icmarine_get_tx_freq();
    icmarine_init();
    icmarine_open();
    icmarine_set_conf();
    icmarine_set_freq();
    icmarine_set_func();
    icmarine_set_level();
    icmarine_set_mode();
    icmarine_set_ptt();
    icmarine_set_split_vfo();
    icmarine_vfo_op();
    icmarine_vfo_op();
    icom_get_ctcss_sql();
    icom_get_ctcss_tone();
    icom_get_dcs_code();
    icom_get_dcs_sql();
    icom_get_ext_func();
    icom_get_ext_level();
    icom_get_ext_parm();
    icom_get_freq_range();
    icom_get_func();
    icom_get_level_raw();
    icom_get_mode_with_data();
    icom_get_parm();
    icom_get_powerstat();
    icom_get_ptt();
    icom_get_rit_new();
    icom_get_rptr_offs();
    icom_get_rptr_shift();
    icom_get_split_freq_mode();
    icom_get_vfo();
    icom_mem_get_split_vfo();
    icom_mW2power();
    icom_power2mW();
    icom_send_morse();
    icom_send_voice_mem();
    icom_stop_morse();
    icom_set_bank();
    icom_set_ctcss_sql();
    icom_set_ctcss_tone();
    icom_set_dcs_code();
    icom_set_dcs_sql();
    icom_set_dsp_flt();
    icom_set_ext_func();
    icom_set_ext_level();
    icom_set_ext_parm();
    icom_set_func();
    icom_set_level_raw();
    icom_set_mode_with_data();
    icom_set_parm();
    icom_set_powerstat();
    icom_set_ptt();
    icom_set_rit_new();
    icom_set_rptr_offs();
    icom_set_rptr_shift();
    icom_set_split_freq_mode();
    icom_set_xit_new();
    int2BCD();
    is_uh_radio_fd();
    lockRx();
    lt_dladdsearchdir();
    lt_dlclose();
    lt_dlerror();
    lt_dlexit();
    lt_dlinit();
    lt_dlopen();
    lt_dlopenext();
    lt_dlsym();
    miniVNA_get_info();
    modeToHamlib();
    modeToNative();
    newcat_decode_event();
    newcat_get_dcd();
    newcat_get_dcs_code();
    newcat_get_dcs_sql();
    newcat_get_ext_level();
    newcat_get_ext_parm();
    newcat_get_parm();
    newcat_get_rptr_offs();
    newcat_get_split_freq();
    newcat_get_split_mode();
    newcat_get_tone();
    newcat_get_tone_sql();
    newcat_recv_dtmf();
    newcat_reset();
    newcat_scan();
    newcat_send_dtmf();
    newcat_send_morse();
    newcat_set_bank();
    newcat_set_dcs_code();
    newcat_set_dcs_sql();
    newcat_set_ext_level();
    newcat_set_ext_parm();
    newcat_set_parm();
    newcat_set_rptr_offs();
    newcat_set_split_freq();
    newcat_set_split_mode();
    newcat_set_tone();
    newcat_set_tone_sql();
    optoscan_close();
    optoscan_get_ctcss_tone();
    optoscan_get_dcs_code();
    optoscan_get_ext_parm();
    optoscan_get_info();
    optoscan_get_level();
    optoscan_open();
    optoscan_recv_dtmf();
    optoscan_scan();
    optoscan_set_ext_parm();
    optoscan_set_level();
    pageSize();
    pbsToHz();
    pll_locked_p();
    ra37xx_cleanup();
    ra37xx_close();
    ra37xx_get_ant();
    ra37xx_get_conf();
    ra37xx_get_freq();
    ra37xx_get_func();
    ra37xx_get_info();
    ra37xx_get_level();
    ra37xx_get_mem();
    ra37xx_get_mode();
    ra37xx_init();
    ra37xx_open();
    ra37xx_scan();
    ra37xx_set_ant();
    ra37xx_set_conf();
    ra37xx_set_freq();
    ra37xx_set_func();
    ra37xx_set_level();
    ra37xx_set_mem();
    ra37xx_set_mode();
    ra37xx_vfo_op();
    read3Bytes();
    read_info();
    readInt();
    readShort();
    readSignal();
    rig_copy_channel();
    rig_copyright();
    rig_ext_token_lookup();
    rig_get_mem_all();
    rig_get_mem_all_cb();
    rig_license();
    rig_parse_mtype();
    rig_probe_all();
    rig_send_voice_mem();
    rig_set_bank();
    rig_set_debug_callback();
    rig_set_debug_file();
    rig_set_ext_func();
    rig_set_mem_all();
    rig_set_mem_all_cb();
    rig_set_pltune_callback();
    rig_set_twiddle();
    rig_unregister();
    rig_version();
    rot_probe_all();
    rot_unregister();
    sendIRCode();
    set_parm_all_cb_generic();
    sprintf_level();
    sprintf_level_ext();
    sprintf_parm();
    tentec_cleanup();
    tentec_get_freq();
    tentec_get_info();
    tentec_get_level();
    tentec_get_mode();
    tentec_init();
    tentec_set_level();
    tentec_set_mode();
    tentec_trx_open();
    thd72_get_chan_all_cb();
    thd74_get_chan_all_cb();
    tt550_cleanup();
    tt550_decode_event();
    tt550_get_freq();
    tt550_get_func();
    tt550_get_info();
    tt550_get_level();
    tt550_get_mode();
    tt550_get_ptt();
    tt550_get_rit();
    tt550_get_split_vfo();
    tt550_get_tuning_step();
    tt550_get_tx_freq();
    tt550_get_tx_mode();
    tt550_get_xit();
    tt550_init();
    tt550_set_freq();
    tt550_set_func();
    tt550_set_mode();
    tt550_set_rit();
    tt550_set_split_vfo();
    tt550_set_tuning_step();
    tt550_set_xit();
    tt550_trx_open();
    tt550_vfo_op();
    wj_cleanup();
    wj_get_conf();
    wj_get_freq();
    wj_get_level();
    wj_get_mode();
    wj_init();
    wj_set_conf();
    wj_set_freq();
    wj_set_level();
    wj_set_mode();
    write3Bytes();
    write_both_dacs();
    writeInt();
    writeShort();
}

main()
{
    used_externally();
    never_used();
}
