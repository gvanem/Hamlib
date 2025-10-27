/*
 * hamlib - (C) Frank Singleton 2000 (javabear at users.sourceforge.net)
 *
 * ftx1.c - (C) Jeremy Miller KO4SSD 2025 (ko4ssd at ko4ssd.com)
 *        
 * The FTX-1 is very much like the FT991 except it has some different frequency ranges and features
 * So the code was built using the FT991 code as a template.
 * 
 * This shared library provides an API for communicating
 * via serial interface to an FTX-1 using the "CAT" interface
 *
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <stdlib.h>
#include <string.h>
#include "hamlib/rig.h"
#include "misc.h"
#include "cache.h"
#include "newcat.h"
#include "yaesu.h"
#include "ftx1.h"

static const char cat_term = ';';             /* Yaesu command terminator */

/* Prototypes */
static int ftx1_init(RIG *rig);
static int ftx1_set_vfo(RIG *rig, vfo_t vfo);
static int ftx1_get_vfo(RIG *rig, vfo_t *vfo);
static int ftx1_get_split_mode(RIG *rig, vfo_t vfo, rmode_t *tx_mode,
                                pbwidth_t *tx_width);
static int ftx1_set_split_mode(RIG *rig, vfo_t vfo, rmode_t tx_mode,
                                pbwidth_t tx_width);
static int ftx1_set_split_freq(RIG *rig, vfo_t vfo, freq_t tx_freq);
static int ftx1_get_split_freq(RIG *rig, vfo_t vfo, freq_t *tx_freq);
static void debug_ftx1info_data(const ftx1info *rdata);
static int ftx1_set_ctcss_tone(RIG *rig, vfo_t vfo, tone_t tone);
static int ftx1_get_ctcss_tone(RIG *rig, vfo_t vfo, tone_t *tone);
static int ftx1_set_dcs_code(RIG *rig, vfo_t vfo, tone_t code);
static int ftx1_get_dcs_code(RIG *rig, vfo_t vfo, tone_t *code);
static int ftx1_set_ctcss_sql(RIG *rig, vfo_t vfo, tone_t tone);
static int ftx1_get_ctcss_sql(RIG *rig, vfo_t vfo, tone_t *tone);
static int ftx1_get_dcs_sql(RIG *rig, vfo_t vfo, tone_t *code);
static int ftx1_set_dcs_sql(RIG *rig, vfo_t vfo, tone_t code);

/* Configuration parameters */
const struct confparams ftx1_cfg_params[] = {
    { RIG_CONF_END, NULL, }
};

const struct confparams ftx1_ext_levels[] =
{
    {
        TOK_KEYER,
        "KEYER",
        "Keyer",
        "Keyer on/off",
        NULL,
        RIG_CONF_CHECKBUTTON,
    },
    {
        TOK_APF_FREQ,
        "APF_FREQ",
        "APF frequency",
        "Audio peak filter frequency",
        NULL,
        RIG_CONF_NUMERIC,
        { .n = { .min = -250, .max = 250, .step = 10 } },
    },
    {
        TOK_APF_WIDTH,
        "APF_WIDTH",
        "APF width",
        "Audio peak filter width",
        NULL,
        RIG_CONF_COMBO,
        { .c = { .combostr = { "Narrow", "Medium", "Wide", NULL } } },
    },
    {
        TOK_CONTOUR,
        "CONTOUR",
        "Contour",
        "Contour on/off",
        NULL,
        RIG_CONF_CHECKBUTTON,
    },
    {
        TOK_CONTOUR_FREQ,
        "CONTOUR_FREQ",
        "Contour frequency",
        "Contour frequency",
        NULL,
        RIG_CONF_NUMERIC,
        { .n = { .min = 10, .max = 3200, .step = 1 } },
    },
    {
        TOK_CONTOUR_LEVEL,
        "CONTOUR_LEVEL",
        "Contour level",
        "Contour level (dB)",
        NULL,
        RIG_CONF_NUMERIC,
        { .n = { .min = -40, .max = 20, .step = 1 } },
    },
    {
        TOK_CONTOUR_WIDTH,
        "CONTOUR_WIDTH",
        "Contour width",
        "Contour width",
        NULL,
        RIG_CONF_NUMERIC,
        { .n = { .min = 1, .max = 11, .step = 1 } },
    },
    {
        TOK_MAXPOWER_HF,
        "MAXPOWER_HF",
        "Maxpower HF",
        "Maxpower HF",
        NULL,
        RIG_CONF_INT,
        { .n = { .min = 5, .max = 100, .step = 1 } },
    },
    {
        TOK_MAXPOWER_6M,
        "MAXPOWER_6M",
        "Maxpower 6m",
        "Maxpower 6m",
        NULL,
        RIG_CONF_INT,
        { .n = { .min = 5, .max = 100, .step = 1 } },
    },
    {
        TOK_MAXPOWER_VHF,
        "MAXPOWER_VHF",
        "Maxpower VHF",
        "Maxpower VHF",
        NULL,
        RIG_CONF_INT,
        { .n = { .min = 5, .max = 50, .step = 1 } },
    },
    {
        TOK_MAXPOWER_UHF,
        "MAXPOWER_UHF",
        "Maxpower UHF",
        "Maxpower UHF",
        NULL,
        RIG_CONF_NUMERIC,
        { .n = { .min = 5, .max = 50, .step = 1 } },
    },

    { RIG_CONF_END, NULL, }
};

int ftx1_ext_tokens[] =
{
    TOK_KEYER, TOK_APF_FREQ, TOK_APF_WIDTH,
    TOK_CONTOUR, TOK_CONTOUR_FREQ, TOK_CONTOUR_LEVEL, TOK_CONTOUR_WIDTH,
    TOK_MAXPOWER_HF, TOK_MAXPOWER_6M, TOK_MAXPOWER_UHF, TOK_MAXPOWER_VHF,
    TOK_BACKEND_NONE
};

/*
 * FTX-1 rig capabilities
 */
struct rig_caps ftx1_caps =
{
    RIG_MODEL(RIG_MODEL_FTX1),
    .model_name =         "FTX-1",
    .mfg_name =           "Yaesu",
    .version =            NEWCAT_VER ".1",
    .copyright =          "LGPL",
    .status =             RIG_STATUS_ALPHA,
    .rig_type =           RIG_TYPE_TRANSCEIVER,
    .ptt_type =           RIG_PTT_RIG,
    .dcd_type =           RIG_DCD_NONE,
    .port_type =          RIG_PORT_SERIAL,
    .serial_rate_min =    4800,         /* Default rate per manual */
    .serial_rate_max =    38400,
    .serial_data_bits =   8,
    .serial_stop_bits =   2,            /* Assumed since manual makes no mention */
    .serial_parity =      RIG_PARITY_NONE,
    .serial_handshake =   RIG_HANDSHAKE_HARDWARE,
    .write_delay =        FTX1_WRITE_DELAY,
    .post_write_delay =   FTX1_POST_WRITE_DELAY,
    .timeout =            2000,
    .retry =              3,
    .has_get_func =       FTX1_FUNCS,
    .has_set_func =       FTX1_FUNCS,
    .has_get_level =      FTX1_LEVELS,
    .has_set_level =      RIG_LEVEL_SET(FTX1_LEVELS),
    .has_get_parm =       RIG_PARM_BANDSELECT,
    .has_set_parm =       RIG_PARM_BANDSELECT,
    .level_gran = {
#define NO_LVL_MICGAIN
#define NO_LVL_SQL
#define NO_LVL_MONITOR_GAIN
#define NO_LVL_RFPOWER
#include "level_gran_yaesu.h"
#undef NO_LVL_MICGAIN
#undef NO_LVL_SQL
#undef NO_LVL_MONITOR_GAIN
#undef NO_LVL_RFPOWER
        [LVL_MICGAIN] = { .min = { .f = 0 }, .max = { .f = 1.0 }, .step = { .f = 1.0f / 100.0f } },
        [LVL_SQL] = { .min = { .f = 0 }, .max = { .f = 1.0 }, .step = { .f = 1.0f / 100.0f } },
        [LVL_MONITOR_GAIN] = { .min = { .f = 0 }, .max = { .f = 1.0 }, .step = { .f = 1.0f / 100.0f } },
        [LVL_RFPOWER] = { .min = { .f = .05 }, .max = { .f = 1.0 }, .step = { .f = 1.0f / 100.0f } },
    },
    .parm_gran =  {
        [PARM_BANDSELECT] = {.step = {.s = "BAND160M,BAND80M,BANDUNUSED,BAND40M,BAND30M,BAND20M,BAND17M,BAND15M,BAND12M,BAND10M,BAND6M,BANDGEN,BANDMW,BANDUNUSED,BANDAIR,BAND70CM,BAND33CM"}}
    },

    .ctcss_list =         common_ctcss_list,
    .dcs_list =           common_dcs_list,
    .preamp =             { 10, 20, RIG_DBLST_END, },
    .attenuator =         { 12, RIG_DBLST_END, },
    .max_rit =            Hz(9999),
    .max_xit =            Hz(9999),
    .max_ifshift =        Hz(1200),
    .agc_level_count = 5,
    .agc_levels = { RIG_AGC_OFF, RIG_AGC_FAST, RIG_AGC_MEDIUM, RIG_AGC_SLOW, RIG_AGC_AUTO },
    .vfo_ops =            FTX1_VFO_OPS,
    .scan_ops =           RIG_SCAN_VFO,
    .targetable_vfo =     RIG_TARGETABLE_FREQ,
    .transceive =         RIG_TRN_OFF,        /* May enable later as the 950 has an Auto Info command */
    .bank_qty =           0,
    .chan_desc_sz =       0,
    .rfpower_meter_cal =  FTX1_RFPOWER_METER_CAL,
    .str_cal =            FTX1_STR_CAL,
    .id_meter_cal =       FTX1_ID_CAL,
    .vd_meter_cal =       FTX1_VD_CAL,
    .comp_meter_cal =     FTX1_COMP_CAL,
    .chan_list =          {
        {   1,  99, RIG_MTYPE_MEM,  NEWCAT_MEM_CAP },
        {   100,  117, RIG_MTYPE_MEM,  NEWCAT_MEM_CAP }, // P1L-P9U PMS channels
        {   118,  127, RIG_MTYPE_MEM,  NEWCAT_MEM_CAP }, // 5xx 5 MHz band
        {   1,      5, RIG_MTYPE_VOICE },
        {   1,      5, RIG_MTYPE_MORSE },
        RIG_CHAN_END,
    },

    // Rig only has 1 model
    .rx_range_list1 =     {
        {kHz(30), MHz(56), FTX1_ALL_RX_MODES, -1, -1, FTX1_VFO_ALL, FTX1_ANTS, "Operating"},
        {MHz(118), MHz(164), FTX1_ALL_RX_MODES, -1, -1, FTX1_VFO_ALL, FTX1_ANTS, "Operating"},
        {MHz(420), MHz(470), FTX1_ALL_RX_MODES, -1, -1, FTX1_VFO_ALL, FTX1_ANTS, "Operating"},
        RIG_FRNG_END,
    },

    .tx_range_list1 =     {
        {MHz(1.8), MHz(54), FTX1_OTHER_TX_MODES, W(5), W(100), FTX1_VFO_ALL, FTX1_ANTS, "Operating"},
        {MHz(1.8), MHz(54), FTX1_AM_TX_MODES, W(2), W(25), FTX1_VFO_ALL, FTX1_ANTS, "Operating"}, /* AM class */
        {MHz(144), MHz(148), FTX1_OTHER_TX_MODES, W(5), W(50), FTX1_VFO_ALL, FTX1_ANTS, "Operating"},
        {MHz(144), MHz(148), FTX1_AM_TX_MODES, W(2), W(25), FTX1_VFO_ALL, FTX1_ANTS, "Operating"}, /* AM class */
        {MHz(430), MHz(450), FTX1_OTHER_TX_MODES, W(5), W(50), FTX1_VFO_ALL, FTX1_ANTS, "Operating"},
        {MHz(430), MHz(450), FTX1_AM_TX_MODES, W(2), W(25), FTX1_VFO_ALL, FTX1_ANTS, "Operating"}, /* AM class */
        RIG_FRNG_END,
    },

    .tuning_steps =       {
        {FTX1_SSB_CW_RX_MODES, Hz(10)},    /* Normal */
        {FTX1_SSB_CW_RX_MODES, Hz(100)},   /* Fast */

        {FTX1_AM_RX_MODES,     Hz(100)},   /* Normal */
        {FTX1_AM_RX_MODES,     kHz(1)},    /* Fast */

        {FTX1_FM_RX_MODES,     Hz(100)},   /* Normal */
        {FTX1_FM_RX_MODES,     kHz(1)},    /* Fast */

        RIG_TS_END,

    },

    /* mode/filter list, .remember =  order matters! */
    .filters =            {
        {RIG_MODE_SSB, Hz(2400)},    /* Normal SSB */
        {RIG_MODE_SSB, Hz(1500)},    /* Narrow SSB */
        {RIG_MODE_SSB, Hz(3200)},    /* Wide SSB */
        {RIG_MODE_CW, Hz(500)},      /* Normal CW */
        {RIG_MODE_CW, Hz(300)},      /* Narrow CW */
        {RIG_MODE_CW, Hz(2400)},     /* Wide CW */
        {RIG_MODE_FM, Hz(16000)},    /* Normal FM */
        {RIG_MODE_FMN, Hz(9000)},    /* Narrow FM */
        {RIG_MODE_AM, Hz(9000)},     /* Normal AM */
        {RIG_MODE_AMN, Hz(6000)},    /* Narrow AM */
        {FTX1_RTTY_DATA_RX_MODES, Hz(500)},     /* Normal RTTY, DATA */
        {FTX1_RTTY_DATA_RX_MODES, Hz(300)},     /* Narrow RTTY, DATA */
        {FTX1_RTTY_DATA_RX_MODES, Hz(3000)},    /* Wide   RTTY, DATA */
        {FTX1_RTTY_DATA_RX_MODES, Hz(2400)},    /*        RTTY, DATA */
        {FTX1_RTTY_DATA_RX_MODES, Hz(2000)},    /*        RTTY, DATA */
        {FTX1_RTTY_DATA_RX_MODES, Hz(1700)},    /*        RTTY, DATA */
        {FTX1_RTTY_DATA_RX_MODES, Hz(1400)},    /*        RTTY, DATA */
        {FTX1_RTTY_DATA_RX_MODES, Hz(1200)},    /*        RTTY, DATA */
        {FTX1_RTTY_DATA_RX_MODES, Hz(800)},     /*        RTTY, DATA */
        {FTX1_RTTY_DATA_RX_MODES, Hz(450)},     /*        RTTY, DATA */
        {FTX1_RTTY_DATA_RX_MODES, Hz(400)},     /*        RTTY, DATA */
        {FTX1_RTTY_DATA_RX_MODES, Hz(350)},     /*        RTTY, DATA */
        {FTX1_RTTY_DATA_RX_MODES, Hz(250)},     /*        RTTY, DATA */
        {FTX1_RTTY_DATA_RX_MODES, Hz(200)},     /*        RTTY, DATA */
        {FTX1_RTTY_DATA_RX_MODES, Hz(150)},     /*        RTTY, DATA */
        {FTX1_RTTY_DATA_RX_MODES, Hz(100)},     /*        RTTY, DATA */
        {FTX1_RTTY_DATA_RX_MODES, Hz(50)},      /*        RTTY, DATA */
        {FTX1_CW_RX_MODES, Hz(2000)},    /*        CW */
        {FTX1_CW_RX_MODES, Hz(1700)},    /*        CW */
        {FTX1_CW_RX_MODES, Hz(1400)},    /*        CW */
        {FTX1_CW_RX_MODES, Hz(1200)},    /*        CW */
        {FTX1_CW_RX_MODES, Hz(800)},     /*        CW */
        {FTX1_CW_RX_MODES, Hz(450)},     /*        CW */
        {FTX1_CW_RX_MODES, Hz(400)},     /*        CW */
        {FTX1_CW_RX_MODES, Hz(350)},     /*        CW */
        {FTX1_CW_RX_MODES, Hz(300)},     /*        CW */
        {FTX1_CW_RX_MODES, Hz(250)},     /*        CW */
        {FTX1_CW_RX_MODES, Hz(200)},     /*        CW */
        {FTX1_CW_RX_MODES, Hz(150)},     /*        CW */
        {FTX1_CW_RX_MODES, Hz(100)},     /*        CW */
        {FTX1_CW_RX_MODES, Hz(50)},      /*        CW */
        {RIG_MODE_SSB, Hz(3000)},    /*        SSB */
        {RIG_MODE_SSB, Hz(2900)},    /*        SSB */
        {RIG_MODE_SSB, Hz(2800)},    /*        SSB */
        {RIG_MODE_SSB, Hz(2700)},    /*        SSB */
        {RIG_MODE_SSB, Hz(2600)},    /*        SSB */
        {RIG_MODE_SSB, Hz(2500)},    /*        SSB */
        {RIG_MODE_SSB, Hz(2300)},    /*        SSB */
        {RIG_MODE_SSB, Hz(2200)},    /*        SSB */
        {RIG_MODE_SSB, Hz(2100)},    /*        SSB */
        {RIG_MODE_SSB, Hz(1950)},    /*        SSB */
        {RIG_MODE_SSB, Hz(1650)},    /*        SSB */
        {RIG_MODE_SSB, Hz(1350)},    /*        SSB */
        {RIG_MODE_SSB, Hz(1100)},    /*        SSB */
        {RIG_MODE_SSB, Hz(850)},     /*        SSB */
        {RIG_MODE_SSB, Hz(600)},     /*        SSB */
        {RIG_MODE_SSB, Hz(400)},     /*        SSB */
        {RIG_MODE_SSB, Hz(200)},     /*        SSB */

        RIG_FLT_END,
    },

    .ext_tokens =         ftx1_ext_tokens,
    .extlevels =          ftx1_ext_levels,
    .cfgparams =          ftx1_cfg_params,

    .priv =               NULL,           /* private data FIXME: */

    .rig_init =           ftx1_init,
    .rig_cleanup =        newcat_cleanup,
    .rig_open =           newcat_open,     /* port opened */
    .rig_close =          newcat_close,    /* port closed */

    .set_freq =           ftx1_set_freq,
    .get_freq =           newcat_get_freq,
    .set_mode =           newcat_set_mode,
    .get_mode =           newcat_get_mode,
    .set_vfo =            ftx1_set_vfo,
    .get_vfo =            ftx1_get_vfo,
    .set_ptt =            newcat_set_ptt,
    .get_ptt =            newcat_get_ptt,
    .set_split_vfo =      newcat_set_split_vfo,
    .get_split_vfo =      newcat_get_split_vfo,
    .set_split_freq =     ftx1_set_split_freq,
    .get_split_freq =     ftx1_get_split_freq,
    .get_split_mode =     ftx1_get_split_mode,
    .set_split_mode =     ftx1_set_split_mode,
    .set_rit =            ftx1_set_rit,
    .get_rit =            ftx1_get_rit,
    .set_xit =            ftx1_set_xit,
    .get_xit =            ftx1_get_xit,
    .get_func =           newcat_get_func,
    .set_func =           newcat_set_func,
    .get_parm =           newcat_get_parm,
    .set_parm =           newcat_set_parm,
    .get_level =          ftx1_get_power_control,
    .set_level =          ftx1_set_power_control,
    .get_mem =            newcat_get_mem,
    .set_mem =            newcat_set_mem,
    .vfo_op =             newcat_vfo_op,
    .get_info =           newcat_get_info,
    .power2mW =           newcat_power2mW,
    .mW2power =           newcat_mW2power,
    .set_rptr_shift =     newcat_set_rptr_shift,
    .get_rptr_shift =     newcat_get_rptr_shift,
    .set_rptr_offs =      newcat_set_rptr_offs,
    .get_rptr_offs =      newcat_get_rptr_offs,
    .set_ctcss_tone =     ftx1_set_ctcss_tone,
    .get_ctcss_tone =     ftx1_get_ctcss_tone,
    .set_dcs_code =       ftx1_set_dcs_code,
    .get_dcs_code =       ftx1_get_dcs_code,
    .set_ctcss_sql  =     ftx1_set_ctcss_sql,
    .get_ctcss_sql  =     ftx1_get_ctcss_sql,
    .set_dcs_sql =        ftx1_set_dcs_sql,
    .get_dcs_sql =        ftx1_get_dcs_sql,
    .set_powerstat =      newcat_set_powerstat,
    .get_powerstat =      newcat_get_powerstat,
    .set_ts =             ftx1_set_ts,
    .get_ts =             ftx1_get_ts,
    .set_trn =            newcat_set_trn,
    .get_trn =            newcat_get_trn,
    .set_channel =        newcat_set_channel,
    .get_channel =        newcat_get_channel,
    .set_ext_level =      newcat_set_ext_level,
    .get_ext_level =      newcat_get_ext_level,
    .send_morse =         newcat_send_morse,
    .wait_morse =         rig_wait_morse,
    .scan =               newcat_scan,
    .send_voice_mem =     newcat_send_voice_mem,
    .set_clock =          newcat_set_clock,
    .get_clock =          newcat_get_clock,
    .morse_qsize =        50,
    .hamlib_check_rig_caps = HAMLIB_CHECK_RIG_CAPS
};


static int
ftx1_get_tx_split(RIG *rig, split_t *in_split)
{
    vfo_t cur_tx_vfo;
    int rval;

    rig_debug(RIG_DEBUG_VERBOSE, "%s called\n", __func__);

    if (!rig || !in_split)
    {
        return (-RIG_EINVAL);
    }

    rval = newcat_get_tx_vfo(rig, &cur_tx_vfo);

    if (rval != RIG_OK)
    {
        return (rval);
    }

    if (cur_tx_vfo == RIG_VFO_B || cur_tx_vfo == RIG_VFO_MEM)
    {
        *in_split = RIG_SPLIT_ON;
    }
    else if (cur_tx_vfo == RIG_VFO_A)
    {
        *in_split = RIG_SPLIT_OFF;
    }
    else
    {
        return (-RIG_EINVAL);
    }

    return (rval);
}

int
ftx1_set_split_freq(RIG *rig, vfo_t vfo, freq_t tx_freq)
{
    int rval;
    split_t is_split;

    rig_debug(RIG_DEBUG_VERBOSE, "%s called\n", __func__);

    rval = ftx1_get_tx_split(rig, &is_split);

    if (rval != RIG_OK)
    {
        return (rval);
    }

    if (CACHE(rig)->freqMainB == tx_freq)
    {
        rig_debug(RIG_DEBUG_TRACE, "%s: freq %.0f already set on VFOB\n", __func__,
                  tx_freq);
        return RIG_OK;
    }

    if (is_split == RIG_SPLIT_OFF)
    {
        rval = newcat_set_tx_vfo(rig, RIG_VFO_B);

        if (rval != RIG_OK)
        {
            return (rval);
        }
    }

    rval = newcat_set_freq(rig, RIG_VFO_B, tx_freq);
    rig_debug(RIG_DEBUG_VERBOSE,
              "%s newcat_set_freq() rval = %d freq = %f\n",
              __func__, rval, tx_freq);
    return (rval);
}

int
ftx1_get_split_freq(RIG *rig, vfo_t vfo, freq_t *tx_freq)
{
    int rval;
    split_t is_split;

    rig_debug(RIG_DEBUG_VERBOSE, "%s called\n", __func__);

    rval = ftx1_get_tx_split(rig, &is_split);

    if (rval != RIG_OK)
    {
        return (rval);
    }

    if (is_split == RIG_SPLIT_OFF)
    {
        *tx_freq = 0.0;
        return (rval);
    }

    rval = newcat_get_freq(rig, RIG_VFO_B, tx_freq);
    rig_debug(RIG_DEBUG_VERBOSE,
              "%s newcat_get_freq() rval = %d freq = %f\n",
              __func__, rval, *tx_freq);

    return (rval);
}

/*
 * rig_get_split_mode*
 *
 * Get the 'X1 split TX mode
 *
 * Parameter    | Type      | Accepted/expected values
 * ------------------------------------------------------------------
 * *rig         | input     | pointer to private data
 * vfo          | input     | currVFO, VFOA, VFOB, MEM
 * *tx_mode     | output    | supported modes
 * *tx_width    | output    | supported widths
 * ------------------------------------------------------------------
 * Returns RIG_OK on success or an error code on failure
 *
 * Comments:    Checks to see if the X1 is in split mode, if so it
 *              checks which VFO is set for TX and then gets the
 *              mode and passband of that VFO and stores it into *tx_mode
 *              and tx_width respectively.  If not in split mode returns
 *              RIG_MODE_NONE and 0 Hz.
 *
 */

static int ftx1_get_split_mode(RIG *rig, vfo_t vfo, rmode_t *tx_mode,
                                pbwidth_t *tx_width)
{
    struct newcat_priv_data *priv;
    int err;
    ftx1info *rdata;

    rig_debug(RIG_DEBUG_VERBOSE, "%s called\n", __func__);

    if (!rig || !tx_mode || !tx_width)
    {
        return -RIG_EINVAL;
    }

    priv = (struct newcat_priv_data *)STATE(rig)->priv;
    rdata = (ftx1info *)priv->ret_data;

    SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "OI;");

    if (RIG_OK != (err = newcat_get_cmd(rig)))
    {
        return err;
    }

    debug_ftx1info_data(rdata);

    *tx_mode = newcat_rmode(rdata->mode);
    rig_debug(RIG_DEBUG_VERBOSE, "%s opposite mode %s\n", __func__,
              rig_strrmode(*tx_mode));
    *tx_width = RIG_PASSBAND_NORMAL;

    return RIG_OK;
}

static void debug_ftx1info_data(const ftx1info *rdata)
{

    rig_debug(RIG_DEBUG_VERBOSE, "%s command         %2.2s\n",
              __func__, rdata->command);
    rig_debug(RIG_DEBUG_VERBOSE, "%s memory_ch       %3.3s\n",
              __func__, rdata->memory_ch);
    rig_debug(RIG_DEBUG_VERBOSE, "%s vfo_freq        %9.9s\n",
              __func__, rdata->vfo_freq);
    rig_debug(RIG_DEBUG_VERBOSE, "%s clarifier       %5.5s\n",
              __func__, rdata->clarifier);
    rig_debug(RIG_DEBUG_VERBOSE, "%s rx_clarifier    %c\n",
              __func__, rdata->rx_clarifier);
    rig_debug(RIG_DEBUG_VERBOSE, "%s tx_clarifier    %c\n",
              __func__, rdata->tx_clarifier);
    rig_debug(RIG_DEBUG_VERBOSE, "%s mode            %c\n",
              __func__, rdata->mode);
    rig_debug(RIG_DEBUG_VERBOSE, "%s vfo_memory      %c\n",
              __func__, rdata->vfo_memory);
    rig_debug(RIG_DEBUG_VERBOSE, "%s tone_mode       %c\n",
              __func__, rdata->tone_mode);
    rig_debug(RIG_DEBUG_VERBOSE, "%s fixed           %2.2s\n",
              __func__, rdata->fixed);
    rig_debug(RIG_DEBUG_VERBOSE, "%s repeater_offset %c\n",
              __func__, rdata->repeater_offset);
    rig_debug(RIG_DEBUG_VERBOSE, "%s terminator      %c\n",
              __func__, rdata->terminator);

}

/*
 * rig_set_split_mode
 *
 * Set the 'X1 split TX mode
 *
 * Parameter    | Type      | Accepted/expected values
 * ------------------------------------------------------------------
 * *rig         | input     | pointer to private data
 * vfo          | input     | currVFO, VFOA, VFOB, MEM
 * tx_mode      | input     | supported modes
 * tx_width     | input     | supported widths
 * ------------------------------------------------------------------
 * Returns RIG_OK on success or an error code on failure
 *
 * Comments: Pass band is not set here nor does it make sense as the
 *              FTX-1 cannot receive on VFO B. The FTX-1 cannot set
 *              VFO B mode directly so we'll just set A and swap A
 *              into B but we must preserve the VFO A mode and VFO B
 *              frequency.
 *              [TODO] This is inaccurate and based on copying the FT991A code to build this.  Remove the above and implement it correctly.
 */

static int ftx1_set_split_mode(RIG *rig, vfo_t vfo, rmode_t tx_mode,
                                pbwidth_t tx_width)
{
    struct newcat_priv_data *priv;
    struct rig_state *state;
    int err;
    char restore_commands[NEWCAT_DATA_LEN];
    split_t is_split;

    rig_debug(RIG_DEBUG_VERBOSE, "%s called\n", __func__);

    if (!rig)
    {
        return -RIG_EINVAL;
    }

    if (CACHE(rig)->modeMainB == tx_mode)
    {
        rig_debug(RIG_DEBUG_TRACE, "%s: mode %s already set on VFOB\n", __func__,
                  rig_strrmode(tx_mode));
        return RIG_OK;
    }

    err = ftx1_get_tx_split(rig, &is_split);

    if (err != RIG_OK)
    {
        return (err);
    }

    if (is_split == RIG_SPLIT_ON)
    {
        err = newcat_set_tx_vfo(rig, RIG_VFO_B);

        if (err != RIG_OK)
        {
            return (err);
        }
    }


    state = STATE(rig);

    rig_debug(RIG_DEBUG_TRACE, "%s: passed vfo = %s\n", __func__,
              rig_strvfo(vfo));
    rig_debug(RIG_DEBUG_TRACE, "%s: passed mode = %s\n", __func__,
              rig_strrmode(tx_mode));
    rig_debug(RIG_DEBUG_TRACE, "%s: passed width = %d Hz\n", __func__,
              (int)tx_width);

    priv = (struct newcat_priv_data *)state->priv;

    /* append VFO A mode restore command first as we want to minimize
       any Rx glitches */
    SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "MD0;");
    rig_debug(RIG_DEBUG_TRACE, "cmd_str = %s\n", priv->cmd_str);

    if (RIG_OK != (err = newcat_get_cmd(rig)))
    {
        return err;
    }

    SNPRINTF(restore_commands, sizeof(restore_commands), "AB;%.*s",
             (int)sizeof(restore_commands) - 4, priv->ret_data);

    /* append VFO B frequency restore command */
    SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "FB;");
    rig_debug(RIG_DEBUG_TRACE, "cmd_str = %s\n", priv->cmd_str);

    if (RIG_OK != (err = newcat_get_cmd(rig)))
    {
        return err;
    }

    size_t len = strlen(restore_commands);
    SNPRINTF(restore_commands + len, sizeof(restore_commands) - len, "%.*s",
         (int)(sizeof(restore_commands) - len - 1), priv->ret_data);

    /* Change mode on VFOA */
    if (RIG_OK != (err = newcat_set_mode(rig, RIG_VFO_A, tx_mode,
                                         RIG_PASSBAND_NOCHANGE)))
    {
        return err;
    }

    /* Send the copy VFO A to VFO B and restore commands */
    SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "%s", restore_commands);
    return newcat_set_cmd(rig);
}

static int ftx1_init(RIG *rig)
{
    int ret;

    rig_debug(RIG_DEBUG_VERBOSE, "%s called, version %s\n", __func__,
              rig->caps->version);

    ret = newcat_init(rig);

    if (ret != RIG_OK) { return ret; }

    STATE(rig)->current_vfo = RIG_VFO_A;
    return RIG_OK;
}

static int ftx1_find_current_vfo(RIG *rig, vfo_t *vfo, tone_t *enc_dec_mode,
                                  rmode_t *mode)
{
    struct newcat_priv_data *priv = (struct newcat_priv_data *)STATE(rig)->priv;
    ftx1info *info = (ftx1info *)priv->ret_data;
    int err;

    rig_debug(RIG_DEBUG_TRACE, "%s called\n", __func__);

    SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "IF;");

    /* Get info */
    if (RIG_OK != (err = newcat_get_cmd(rig)))
    {
        return err;
    }

    debug_ftx1info_data(info);

    if (enc_dec_mode != NULL)
    {
        *enc_dec_mode = info->tone_mode;
    }

    if (mode != NULL)
    {
        *mode = newcat_rmode(info->mode);
    }

    switch (info->vfo_memory)
    {
    case '1':    // Memory
    case '2':    // Memory Tune
    case '3':    // Quick Memory
    case '4':    // Quick Memory Tune
        *vfo = RIG_VFO_MEM;
        break;

    case '0':    // VFO
        *vfo = RIG_VFO_A;
        break;

    default:
        rig_debug(RIG_DEBUG_BUG, "%s: unexpected vfo returned 0x%X\n",
                  __func__, info->vfo_memory);
        return -RIG_EINTERNAL;
    }

    return RIG_OK;
}

static int ftx1_get_enabled_ctcss_dcs_mode(RIG *rig)
{
    struct newcat_priv_data *priv = (struct newcat_priv_data *)STATE(rig)->priv;
    int err;

    rig_debug(RIG_DEBUG_TRACE, "%s called\n", __func__);

    SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "CT0;");

    /* Get enabled mode */
    if (RIG_OK != (err = newcat_get_cmd(rig)))
    {
        return err;
    }

    return priv->ret_data[3];
}

static int ftx1_set_ctcss_tone(RIG *rig, vfo_t vfo, tone_t tone)
{
    struct newcat_priv_data *priv = (struct newcat_priv_data *)STATE(rig)->priv;
    int i;
    ncboolean tone_match;

    rig_debug(RIG_DEBUG_TRACE, "%s called\n", __func__);

    for (i = 0, tone_match = FALSE; rig->caps->ctcss_list[i] != 0; i++)
    {
        if (tone == rig->caps->ctcss_list[i])
        {
            tone_match = TRUE;
            break;
        }
    }

    rig_debug(RIG_DEBUG_TRACE, "%s: tone = %u, tone_match = %d, i = %d\n",
              __func__, tone, tone_match, i);

    if (tone_match == FALSE && tone != 0)
    {
        return -RIG_EINVAL;
    }

    if (tone == 0)     /* turn off ctcss */
    {
        SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "CT00;");
    }
    else
    {
        SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "CN00%3.3d;CT02;", i);
    }

    return newcat_set_cmd(rig);
}

static int ftx1_get_ctcss_tone(RIG *rig, vfo_t vfo, tone_t *tone)
{
    struct newcat_priv_data *priv = (struct newcat_priv_data *)STATE(rig)->priv;
    int ret;
    int t;
    int ret_data_len;
    tone_t enc_dec_mode;
    rmode_t rmode;
    char *retlvl;

    rig_debug(RIG_DEBUG_TRACE, "%s called with vfo %s\n",
              __func__, rig_strvfo(vfo));

    *tone = 0;

    ret = ftx1_find_current_vfo(rig, &vfo, &enc_dec_mode, &rmode);

    if (ret < 0)
    {
        return ret;
    }

    rig_debug(RIG_DEBUG_TRACE, "%s current vfo is %s\n",
              __func__, rig_strvfo(vfo));

    if (rmode != RIG_MODE_FM && rmode != RIG_MODE_FMN && rmode != RIG_MODE_C4FM)
    {
        return RIG_OK;
    }

    if ((enc_dec_mode == '0') ||      // CTCSS and DCS Disabled
            (enc_dec_mode == '3') ||  // DCS Encode and Decode Enabled
            (enc_dec_mode == '4'))    // DCS Encode only
    {
        return RIG_OK;                // Any of the above not CTCSS return 0
    }

    /* Get CTCSS TONE */
    SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "CN00;");

    if (RIG_OK != (ret = newcat_get_cmd(rig)))
    {
        return ret;
    }

    ret_data_len = strlen(priv->ret_data);

    /* skip command */
    retlvl = priv->ret_data + strlen(priv->cmd_str) - 1;
    /* chop term */
    priv->ret_data[ret_data_len - 1] = '\0';

    t = atoi(retlvl);   /*  tone index */

    rig_debug(RIG_DEBUG_TRACE, "%s ctcss code %d\n", __func__, t);

    if (t < 0 || t > 49)
    {
        return -RIG_EINVAL;
    }

    *tone = rig->caps->ctcss_list[t];

    return RIG_OK;
}

static int ftx1_set_ctcss_sql(RIG *rig, vfo_t vfo, tone_t tone)
{
    struct newcat_priv_data *priv = (struct newcat_priv_data *)STATE(rig)->priv;
    int err;
    rmode_t rmode;

    rig_debug(RIG_DEBUG_TRACE, "%s called\n", __func__);

    err = ftx1_find_current_vfo(rig, &vfo, NULL, &rmode);

    if (err != RIG_OK)
    {
        return err;
    }

    if (rmode != RIG_MODE_FM && rmode != RIG_MODE_FMN && rmode != RIG_MODE_C4FM)
    {
        return -RIG_EINVAL;  // Invalid mode for setting ctcss
    }

    if (tone == 0)
    {
        SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "CT00;");
    }
    else
    {
        int i;
        ncboolean tone_match;

        for (i = 0, tone_match = FALSE; rig->caps->ctcss_list[i] != 0; i++)
        {
            if (tone == rig->caps->ctcss_list[i])
            {
                tone_match = TRUE;
                break;
            }
        }

        if (tone_match == FALSE)
        {
            return -RIG_EINVAL;   // Tone not on the list
        }

        SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "CN0%3.3d;CT01;", i);
    }

    return newcat_set_cmd(rig);
}

static int ftx1_get_ctcss_sql(RIG *rig, vfo_t vfo, tone_t *tone)
{
    struct newcat_priv_data *priv = (struct newcat_priv_data *)STATE(rig)->priv;
    int ret;
    int t;
    int ret_data_len;
    char *retlvl;

    rig_debug(RIG_DEBUG_TRACE, "%s called\n", __func__);

    *tone = 0;

    ret = ftx1_get_enabled_ctcss_dcs_mode(rig);

    if (ret < 0)
    {
        return ret;
    }

    if (ret != '1') // If not CTCSS Encode and Decode return tone of zero.
    {
        return RIG_OK;
    }

    /* Get CTCSS TONE */
    SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "CN00;");

    if (RIG_OK != (ret = newcat_get_cmd(rig)))
    {
        return ret;
    }

    ret_data_len = strlen(priv->ret_data);

    /* skip command */
    retlvl = priv->ret_data + strlen(priv->cmd_str) - 1;
    /* chop term */
    priv->ret_data[ret_data_len - 1] = '\0';

    t = atoi(retlvl);   /*  tone index */

    rig_debug(RIG_DEBUG_TRACE, "%s ctcss code %d\n", __func__, t);

    if (t < 0 || t > 49)
    {
        return -RIG_EINVAL;
    }

    *tone = rig->caps->ctcss_list[t];

    return RIG_OK;
}

static int ftx1_get_dcs_code(RIG *rig, vfo_t vfo, tone_t *code)
{
    struct newcat_priv_data *priv = (struct newcat_priv_data *)STATE(rig)->priv;
    int err;
    int t;
    tone_t enc_dec_mode;
    rmode_t rmode;
    int ret_data_len;
    char *retlvl;

    rig_debug(RIG_DEBUG_TRACE, "%s called\n", __func__);

    *code = 0;

    err = ftx1_find_current_vfo(rig, &vfo, &enc_dec_mode, &rmode);

    if (err < 0)
    {
        return err;
    }

    rig_debug(RIG_DEBUG_TRACE, "%s current vfo is %s\n",
              __func__, rig_strvfo(vfo));

    if (rmode != RIG_MODE_FM && rmode != RIG_MODE_FMN && rmode != RIG_MODE_C4FM)
    {
        return RIG_OK;
    }

    if ((enc_dec_mode == '0') ||     // Encode off
            (enc_dec_mode == '1') || // CTCSS Encode and Decode
            (enc_dec_mode == '2'))   // CTCSS Encode Only
    {
        return RIG_OK;               // Any of the above not DCS return 0
    }

    SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "CN01;");

    /* Get DCS code */
    if (RIG_OK != (err = newcat_get_cmd(rig)))
    {
        return err;
    }

    ret_data_len = strlen(priv->ret_data);

    /* skip command */
    retlvl = priv->ret_data + strlen(priv->cmd_str) - 1;
    /* chop term */
    priv->ret_data[ret_data_len - 1] = '\0';

    t = atoi(retlvl);   /*  code index */

    if (t < 0 || t > 103)
    {
        return -RIG_EINVAL;
    }

    *code = rig->caps->dcs_list[t];

    rig_debug(RIG_DEBUG_TRACE, "%s dcs code %u\n", __func__, *code);

    return RIG_OK;
}

static int ftx1_set_dcs_code(RIG *rig, vfo_t vfo, tone_t code)
{
    struct newcat_priv_data *priv = (struct newcat_priv_data *)STATE(rig)->priv;
    int i;
    ncboolean code_match;

    rig_debug(RIG_DEBUG_TRACE, "%s called\n", __func__);

    for (i = 0, code_match = FALSE; rig->caps->dcs_list[i] != 0; i++)
    {
        if (code == rig->caps->dcs_list[i])
        {
            code_match = TRUE;
            break;
        }
    }

    rig_debug(RIG_DEBUG_TRACE, "%s: code = %u, code_match = %d, i = %d\n",
              __func__, code, code_match, i);

    if (code_match == FALSE && code != 0)
    {
        return -RIG_EINVAL;
    }

    if (code == 0)     /* turn off dcs */
    {
        SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "CT00;");
    }
    else
    {
        SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "CN01%3.3d;CT04;", i);
    }

    return newcat_set_cmd(rig);
}

static int ftx1_set_dcs_sql(RIG *rig, vfo_t vfo, tone_t code)
{
    struct newcat_priv_data *priv = (struct newcat_priv_data *)STATE(rig)->priv;
    int i;
    ncboolean code_match;

    rig_debug(RIG_DEBUG_TRACE, "%s called\n", __func__);

    for (i = 0, code_match = FALSE; rig->caps->dcs_list[i] != 0; i++)
    {
        if (code == rig->caps->dcs_list[i])
        {
            code_match = TRUE;
            break;
        }
    }

    rig_debug(RIG_DEBUG_TRACE, "%s: code = %u, code_match = %d, i = %d\n",
              __func__, code, code_match, i);

    if (code_match == FALSE && code != 0)
    {
        return -RIG_EINVAL;
    }

    if (code == 0)     /* turn off dcs */
    {
        SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "CT00;");
    }
    else
    {
        SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "CN01%3.3d;CT03;", i);
    }

    return newcat_set_cmd(rig);
}

static int ftx1_get_dcs_sql(RIG *rig, vfo_t vfo, tone_t *code)
{
    struct newcat_priv_data *priv = (struct newcat_priv_data *)STATE(rig)->priv;
    int codeindex;
    int ret;
    int ret_data_len;
    char *retlvl;

    rig_debug(RIG_DEBUG_TRACE, "%s called\n", __func__);

    *code = 0;

    ret = ftx1_get_enabled_ctcss_dcs_mode(rig);

    if (ret < 0)
    {
        return ret;
    }

    if (ret != '3')
    {
        return RIG_OK;   // If not DCS Encode and Decode return zero.
    }

    /* Get DCS CODE */
    SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "CN01;");

    if (RIG_OK != (ret = newcat_get_cmd(rig)))
    {
        return ret;
    }

    ret_data_len = strlen(priv->ret_data);

    /* skip command */
    retlvl = priv->ret_data + strlen(priv->cmd_str) - 1;
    /* chop term */
    priv->ret_data[ret_data_len - 1] = '\0';

    codeindex = atoi(retlvl);   /*  code index */

    rig_debug(RIG_DEBUG_TRACE, "%s dcs code %d\n", __func__, codeindex);

    if (codeindex < 0 || codeindex > 103)
    {
        return -RIG_EINVAL;
    }

    *code = rig->caps->dcs_list[codeindex];

    return RIG_OK;
}

// VFO functions so rigctld can be used without --vfo argument
static int ftx1_set_vfo(RIG *rig, vfo_t vfo)
{
    STATE(rig)->current_vfo = vfo;
    RETURNFUNC2(RIG_OK);
}

static int ftx1_get_vfo(RIG *rig, vfo_t *vfo)
{
    *vfo = STATE(rig)->current_vfo;
    RETURNFUNC2(RIG_OK);
}





/* RIT/XIT functions */
static int ftx1_get_rit(RIG *rig, vfo_t vfo, shortfreq_t *rit)
{
    struct newcat_priv_data *priv = (struct newcat_priv_data *)STATE(rig)->priv;
    int err;
    ftx1info *rdata;

    rig_debug(RIG_DEBUG_VERBOSE, "%s called\n", __func__);

    if (!rig || !rit)
    {
        return -RIG_EINVAL;
    }

    rdata = (ftx1info *)priv->ret_data;

    SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "IF;");

    if (RIG_OK != (err = newcat_get_cmd(rig)))
    {
        return err;
    }

    debug_ftx1info_data(rdata);

    if (rdata->rx_clarifier == '1')
    {
        *rit = atoi(rdata->clarifier);
        if (rdata->clarifier[0] == '-')
        {
            *rit = -*rit;
        }
    }
    else
    {
        *rit = 0;
    }

    return RIG_OK;
}

static int ftx1_set_rit(RIG *rig, vfo_t vfo, shortfreq_t rit)
{
    struct newcat_priv_data *priv = (struct newcat_priv_data *)STATE(rig)->priv;
    int err;

    rig_debug(RIG_DEBUG_VERBOSE, "%s called\n", __func__);

    if (rit == 0)
    {
        SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "RC0;");
    }
    else if (rit > 0)
    {
        SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "RC+%04ld;", rit);
    }
    else
    {
        SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "RC%05ld;", rit);
    }

    if (RIG_OK != (err = newcat_set_cmd(rig)))
    {
        return err;
    }

    return RIG_OK;
}

static int ftx1_get_xit(RIG *rig, vfo_t vfo, shortfreq_t *xit)
{
    struct newcat_priv_data *priv = (struct newcat_priv_data *)STATE(rig)->priv;
    int err;
    ftx1info *rdata;

    rig_debug(RIG_DEBUG_VERBOSE, "%s called\n", __func__);

    if (!rig || !xit)
    {
        return -RIG_EINVAL;
    }

    rdata = (ftx1info *)priv->ret_data;

    SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "IF;");

    if (RIG_OK != (err = newcat_get_cmd(rig)))
    {
        return err;
    }

    debug_ftx1info_data(rdata);

    if (rdata->tx_clarifier == '1')
    {
        *xit = atoi(rdata->clarifier);
        if (rdata->clarifier[0] == '-')
        {
            *xit = -*xit;
        }
    }
    else
    {
        *xit = 0;
    }

    return RIG_OK;
}

static int ftx1_set_xit(RIG *rig, vfo_t vfo, shortfreq_t xit)
{
    struct newcat_priv_data *priv = (struct newcat_priv_data *)STATE(rig)->priv;
    int err;

    rig_debug(RIG_DEBUG_VERBOSE, "%s called\n", __func__);

    if (xit == 0)
    {
        SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "TC0;");
    }
    else if (xit > 0)
    {
        SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "TC+%04ld;", xit);
    }
    else
    {
        SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "TC%05ld;", xit);
    }

    if (RIG_OK != (err = newcat_set_cmd(rig)))
    {
        return err;
    }

    return RIG_OK;
}



/* Tuning step functions */
static int ftx1_get_ts(RIG *rig, vfo_t vfo, shortfreq_t *ts)
{
    struct newcat_priv_data *priv = (struct newcat_priv_data *)STATE(rig)->priv;
    int err;
    char response[32];

    rig_debug(RIG_DEBUG_VERBOSE, "%s called\n", __func__);

    if (!rig || !ts)
    {
        return -RIG_EINVAL;
    }

    // Get current mode to determine which tuning step setting to query
    rmode_t mode;
    pbwidth_t width;
    if (RIG_OK != (err = rig_get_mode(rig, vfo, &mode, &width)))
    {
        return err;
    }

    // Determine which tuning step setting to query based on mode
    int setting_num;
    if (mode == RIG_MODE_FM || mode == RIG_MODE_FMN)
    {
        setting_num = 3;  // FM DIAL STEP
    }
    else if (mode == RIG_MODE_RTTY || mode == RIG_MODE_RTTYR || 
             mode == RIG_MODE_PKTLSB || mode == RIG_MODE_PKTUSB)
    {
        setting_num = 2;  // RTTY/PSK DIAL STEP
    }
    else
    {
        setting_num = 1;  // SSB/CW DIAL STEP
    }

    // Send EX0306[setting_num]; to query current tuning step
    // The radio returns the current value in the response
    SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "EX0306%02d;", setting_num);

    if (RIG_OK != (err = newcat_get_cmd(rig)))
    {
        return err;
    }

    // Parse response like "EX0306012;" or "EX0306021;" or "EX0306033;"
    strncpy(response, priv->ret_data, sizeof(response) - 1);
    response[sizeof(response) - 1] = '\0';

    if (strlen(response) >= 9 && strncmp(response, "EX0306", 6) == 0)
    {
        // Extract the step value from the response (last digit before semicolon)
        char step_str[2];
        int len = strlen(response);
        if (len >= 9) {
            step_str[0] = response[len - 2];  // Get the digit before semicolon
            step_str[1] = '\0';
        } else {
            step_str[0] = '1';  // Default to 1 (10Hz)
            step_str[1] = '\0';
        }
        
        int step_value = atoi(step_str);
        
        // Convert step value to frequency based on mode
        if (mode == RIG_MODE_FM || mode == RIG_MODE_FMN)
        {
            // FM DIAL STEP values: 0:5kHz, 1:6.25kHz, 2:10kHz, 3:12.5kHz, 4:20kHz, 5:25kHz, 6:Auto
            switch (step_value) {
                case 0: *ts = 5000; break;
                case 1: *ts = 6250; break;
                case 2: *ts = 10000; break;
                case 3: *ts = 12500; break;
                case 4: *ts = 20000; break;
                case 5: *ts = 25000; break;
                case 6: *ts = 0; break;  // Auto - return 0 to indicate auto mode
                default: *ts = 10000; break;
            }
        }
        else if (mode == RIG_MODE_RTTY || mode == RIG_MODE_RTTYR || 
                 mode == RIG_MODE_PKTLSB || mode == RIG_MODE_PKTUSB)
        {
            // RTTY/PSK DIAL STEP values: 0:5Hz, 1:10Hz, 2:20Hz
            switch (step_value) {
                case 0: *ts = 5; break;
                case 1: *ts = 10; break;
                case 2: *ts = 20; break;
                default: *ts = 10; break;
            }
        }
        else
        {
            // SSB/CW DIAL STEP values: 0:5Hz, 1:10Hz, 2:20Hz
            switch (step_value) {
                case 0: *ts = 5; break;
                case 1: *ts = 10; break;
                case 2: *ts = 20; break;
                default: *ts = 10; break;
            }
        }
        
        rig_debug(RIG_DEBUG_TRACE, "%s: response=%s, setting=%d, step_value=%d, ts=%ld\n", 
                 __func__, response, setting_num, step_value, *ts);
        
        return RIG_OK;
    }

    // If parsing failed, return default
    *ts = 10;
    return RIG_OK;
}

static int ftx1_set_ts(RIG *rig, vfo_t vfo, shortfreq_t ts)
{
    struct newcat_priv_data *priv = (struct newcat_priv_data *)STATE(rig)->priv;
    int step_value, setting_num;
    rmode_t mode;
    pbwidth_t width;
    int err;

    rig_debug(RIG_DEBUG_VERBOSE, "%s called with ts=%ld\n", __func__, ts);

    // Get current mode to determine which tuning step setting to modify
    if (RIG_OK != (err = rig_get_mode(rig, vfo, &mode, &width)))
    {
        return err;
    }

    // Determine which tuning step setting to modify based on mode
    if (mode == RIG_MODE_FM || mode == RIG_MODE_FMN)
    {
        setting_num = 3;  // FM DIAL STEP
        // Convert frequency to FM step value: 0:5kHz, 1:6.25kHz, 2:10kHz, 3:12.5kHz, 4:20kHz, 5:25kHz, 6:Auto
        if (ts == 0) step_value = 6;  // Auto mode
        else if (ts <= 5000) step_value = 0;
        else if (ts <= 6250) step_value = 1;
        else if (ts <= 10000) step_value = 2;
        else if (ts <= 12500) step_value = 3;
        else if (ts <= 20000) step_value = 4;
        else step_value = 5;
    }
    else if (mode == RIG_MODE_RTTY || mode == RIG_MODE_RTTYR || 
             mode == RIG_MODE_PKTLSB || mode == RIG_MODE_PKTUSB)
    {
        setting_num = 2;  // RTTY/PSK DIAL STEP
        // Convert frequency to RTTY step value: 0:5Hz, 1:10Hz, 2:20Hz
        if (ts <= 5) step_value = 0;
        else if (ts <= 10) step_value = 1;
        else step_value = 2;
    }
    else
    {
        setting_num = 1;  // SSB/CW DIAL STEP
        // Convert frequency to SSB/CW step value: 0:5Hz, 1:10Hz, 2:20Hz
        if (ts <= 5) step_value = 0;
        else if (ts <= 10) step_value = 1;
        else step_value = 2;
    }

    // Format: EX0306[setting_num][step_value]; 
    SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "EX0306%02d%d;", setting_num, step_value);

    rig_debug(RIG_DEBUG_TRACE, "%s: cmd_str = %s (setting=%d, step_value=%d)\n", 
             __func__, priv->cmd_str, setting_num, step_value);

    return newcat_set_cmd(rig);
}

/*
 * FTX-1 specific power control function
 * The FTX-1 uses PC command with format: PC[amp][power] where:
 * amp: 1 = radio only, 2 = amplifier
 * power: 3-digit percentage (005-010 for radio, 005-100 for amp)
 */
int ftx1_set_power_control(RIG *rig, vfo_t vfo, setting_t level, value_t val)
{
    struct newcat_priv_data *priv = (struct newcat_priv_data *)STATE(rig)->priv;
    int amp_setting = 2; // Default to amp on (2)
    int power_percent;
    
    ENTERFUNC;
    
    // Only handle RIG_LEVEL_RFPOWER
    if (level != RIG_LEVEL_RFPOWER)
    {
        // For other levels, use the standard newcat function
        return newcat_set_level(rig, vfo, level, val);
    }
    
    // Convert power value (0.0-1.0) to percentage (0-100)
    power_percent = (int)(val.f * 100.0f + 0.5f);
    
    // Ensure power is within valid range for amp (5-100)
    if (power_percent < 5) power_percent = 5;
    if (power_percent > 100) power_percent = 100;
    
    // Format: PC[amp][power] where amp=2 for amplifier, power=3 digits
    SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "PC%d%03d%c", amp_setting, power_percent, cat_term);
    
    rig_debug(RIG_DEBUG_TRACE, "%s: cmd_str = %s\n", __func__, priv->cmd_str);
    
    RETURNFUNC(newcat_set_cmd(rig));
}

/*
 * FTX-1 specific power reading function
 * The FTX-1 returns PC command responses in format: PC[amp][power] where:
 * amp: 1 = radio only, 2 = amplifier
 * power: 3-digit percentage (005-010 for radio, 005-100 for amp)
 */
int ftx1_get_power_control(RIG *rig, vfo_t vfo, setting_t level, value_t *val)
{
    struct newcat_priv_data *priv = (struct newcat_priv_data *)STATE(rig)->priv;
    int amp_setting, power_percent;
    char response[16];
    int err;
    
    ENTERFUNC;
    
    // Only handle RIG_LEVEL_RFPOWER
    if (level != RIG_LEVEL_RFPOWER)
    {
        // For other levels, use the standard newcat function
        return newcat_get_level(rig, vfo, level, val);
    }
    
    // Send PC; command to get current power setting
    SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "PC%c", cat_term);
    
    if (RIG_OK != (err = newcat_get_cmd(rig)))
    {
        RETURNFUNC(err);
    }
    
    // Parse response like "PC2075;" or "PC1004;"
    // Extract the numeric part (2075 or 1004)
    strncpy(response, priv->ret_data, sizeof(response) - 1);
    response[sizeof(response) - 1] = '\0';
    
    // Remove "PC" prefix and ";" suffix
    if (strlen(response) >= 4 && strncmp(response, "PC", 2) == 0)
    {
        char *numeric_part = response + 2; // Skip "PC"
        char *semicolon = strchr(numeric_part, ';');
        if (semicolon) *semicolon = '\0';
        
        if (strlen(numeric_part) == 4)
        {
            amp_setting = numeric_part[0] - '0';  // First digit (1 or 2)
            power_percent = atoi(numeric_part + 1); // Last 3 digits
            
            // Convert to 0.0-1.0 range
            val->f = (float)power_percent / 100.0f;
            
            rig_debug(RIG_DEBUG_TRACE, "%s: response=%s, amp=%d, power=%d, val=%.3f\n", 
                     __func__, response, amp_setting, power_percent, val->f);
            
            RETURNFUNC(RIG_OK);
        }
    }
    
    // If parsing failed, return error
    RETURNFUNC(-RIG_EPROTO);
}

/*
 * FTX-1 specific frequency setting function
 * The FTX-1 requires FA command with format: FA[11 digits]; where:
 * frequency is formatted as 11 digits with leading zeros
 * e.g., FA007200000; for 7.2 MHz
 */
int ftx1_set_freq(RIG *rig, vfo_t vfo, freq_t freq)
{
    struct newcat_priv_data *priv = (struct newcat_priv_data *)STATE(rig)->priv;
    char c;

    ENTERFUNC;

    // Determine VFO letter (A or B)
    if (vfo == RIG_VFO_A || vfo == RIG_VFO_MAIN)
    {
        c = 'A';
    }
    else if (vfo == RIG_VFO_B || vfo == RIG_VFO_SUB)
    {
        c = 'B';
    }
    else
    {
        // For other VFOs, default to A
        c = 'A';
    }

    // Format: F[VFO][10 digits]; where 10 digits include leading zeros
    // Radio returns: FA028100000; (10 digits after FA)
    // We should send: FA028200000; (10 digits after FA, not 11)
    // The frequency needs to be formatted as 10 digits with proper leading zeros
    char freq_str[16];
    SNPRINTF(freq_str, sizeof(freq_str), "%09"PRIll, (int64_t)freq);
    SNPRINTF(priv->cmd_str, sizeof(priv->cmd_str), "F%c%s%c", c, freq_str, cat_term);

    rig_debug(RIG_DEBUG_TRACE, "%s: cmd_str = %s\n", __func__, priv->cmd_str);

    RETURNFUNC(newcat_set_cmd(rig));
}