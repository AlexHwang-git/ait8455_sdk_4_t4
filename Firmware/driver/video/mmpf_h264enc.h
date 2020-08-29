//==============================================================================
//
//  File        : mmpf_h264enc.h
//  Description : Header function of h.264 codec
//  Author      : Julian Hsiao
//  Revision    : 1.0
//
//==============================================================================

#ifndef _MMPF_H264ENC_H_
#define _MMPF_H264ENC_H_

#include "mmpf_mp4venc.h"

/// @}

/** @addtogroup MMPF_H264ENC
 *  @{
 */
//==============================================================================
//
//                              COMPILER OPTION
//
//==============================================================================
#define SUPPORT_POC_TYPE_1      (0)
#define SUPPORT_VUI_INFO        (1)
    #define INS_SEI_BUF_PERIOD          (1)
    #define INS_SEI_PIC_TIMING          (1)
    #define INS_SEI_ITU_T_T35           (0)

#define VUI_HRD_MAX_CPB_CNT     (1)

#define SUPPORT_LYNC_SIMULCAST  (1) // give different stream id in pps header

//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================
#define BASELINE_PROFILE 66      //!< YUV 4:2:0/8  "Baseline"
#define MAIN_PROFILE     77      //!< YUV 4:2:0/8  "Main"
#define EXTENDED         88      //!< YUV 4:2:0/8  "Extended"
#define FREXT_HP        100      //!< YUV 4:2:0/8 "High"
#define FREXT_Hi10P     110      //!< YUV 4:2:0/10 "High 10"
#define FREXT_Hi422     122      //!< YUV 4:2:2/10 "High 4:2:2"
#define FREXT_Hi444     244      //!< YUV 4:4:4/14 "High 4:4:4"
#define FREXT_CAVLC444   44      //!< YUV 4:4:4/14 "CAVLC 4:4:4"
#define CONSTRAINED_FLAG_0      0x01
#define CONSTRAINED_FLAG_1      0x02
#define CONSTRAINED_FLAG_2      0x04
#define CONSTRAINED_FLAG_3      0x08
#define CONSTRAINED_FLAG_4      0x10
#define CONSTRAINED_FLAG_5      0x20
#define CONSTRAINED_FLAG_6      0x40
#define H264E_MAX_CBR_OFFSET    (12)
#define H264E_MIN_CBR_OFFSET    (-12)

#define H264E_STARTCODE_LEN     (4)

#define H264E_MIN_MB_QP_8x8     (9)
#if (STRICT_AVG_FRAME_SIZE == 1)
#define H264E_MAX_MB_QP         (MAX_MB_QP)
#define H264E_MIN_MB_QP         (MIN_MB_QP)
#else
#define H264E_MAX_MB_QP         (46)
#define H264E_MIN_MB_QP         (10)
#define H264E_CABAC_MIN_MB_I_QP   (28) // for H264 rt-mode cabac only
#define H264E_CABAC_MIN_MB_P_QP   (22) // for H264 rt-mode cabac only
#endif
#define H264E_VLC_SYNC_SIZE     (64)

#define VIDENC_WORKBUF_SET_NUM  (3)
//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef enum _MMPF_H264ENC_INS_FLAG {
    MMPF_H264ENC_INS_NONE = 0x0000,
    MMPF_H264ENC_INS_SPS = 0x0001,
    MMPF_H264ENC_INS_PPS = 0x0002
} MMPF_H264ENC_INS_FLAG;

typedef enum _MMPF_H264ENC_POC_TYPE {
    MMPF_H264ENC_POC_TYPE_0 = 0,
    MMPF_H264ENC_POC_TYPE_2 = 2
} MMPF_H264ENC_POC_TYPE;

typedef enum _MMPF_H264ENC_ENTROPY_MODE {
    MMPF_H264ENC_ENTROPY_CAVLC = 0,
    MMPF_H264ENC_ENTROPY_CABAC
} MMPF_H264ENC_ENTROPY_MODE;

typedef enum _MMPF_H264ENC_SLICE_TYPE {
    MMPF_H264ENC_SLICE_TYPE_P  = 5,
    MMPF_H264ENC_SLICE_TYPE_B  = 6,
    MMPF_H264ENC_SLICE_TYPE_I  = 7,
    MMPF_H264ENC_SLICE_TYPE_SP = 8,
    MMPF_H264ENC_SLICE_TYPE_SI = 9
} MMPF_H264ENC_SLICE_TYPE;

typedef enum _MMPF_H264ENC_BYTESTREAM_TYPE {
    MMPF_H264ENC_BYTESTREAM_ANNEXB= 0,
    MMPF_H264ENC_BYTESTREAM_NALU_EBSP,
    MMPF_H264ENC_BYTESTREAM_NALU_RBSP
} MMPF_H264ENC_BYTESTREAM_TYPE;

typedef enum _MMPF_H264ENC_HDRCODING_MODE {
    MMPF_H264ENC_HDRCODING_FULL = 0,
    MMPF_H264ENC_HDRCODING_COHDR,
    MMPF_H264ENC_HDRCODING_MAX
} MMPF_H264ENC_HDRCODING_MODE;

typedef enum _MMPF_H264ENC_COHDR_OPTION {
    MMPF_H264ENC_COHDR_NONE  = 0x0000,
    MMPF_H264ENC_COHDR_SEI_0 = 0x0001,
    MMPF_H264ENC_COHDR_SEI_1 = 0x0002,
    MMPF_H264ENC_COHDR_PNALU = 0x0004,
    MMPF_H264ENC_COHDR_SLICE = 0x0008
} MMPF_H264ENC_COHDR_OPTION;

typedef enum _MMPF_H264ENC_NALU_TYPE {
    NALU_TYPE_SLICE    = 1,
    NALU_TYPE_DPA      = 2,
    NALU_TYPE_DPB      = 3,
    NALU_TYPE_DPC      = 4,
    NALU_TYPE_IDR      = 5,
    NALU_TYPE_SEI      = 6,
    NALU_TYPE_SPS      = 7,
    NALU_TYPE_PPS      = 8,
    NALU_TYPE_AUD      = 9,
    NALU_TYPE_EOSEQ    = 10,
    NALU_TYPE_EOSTREAM = 11,
    NALU_TYPE_FILL     = 12,
    NALU_TYPE_SPSEXT   = 13,
    NALU_TYPE_PREFIX   = 14,
    NALU_TYPE_SUBSPS   = 15
} MMPF_H264ENC_NALU_TYPE;

typedef enum _MMPF_H264ENC_NAL_REF_IDC{
    NALU_PRIORITY_HIGHEST     = 3,
    NALU_PRIORITY_HIGH        = 2,
    NALU_PRIORITY_LOW         = 1,
    NALU_PRIORITY_DISPOSABLE  = 0
} MMPF_H264ENC_NAL_REF_IDC;

#if (SUPPORT_VUI_INFO == 1)
typedef enum _MMPF_H264ENC_SEI_TYPE {
    MMPF_H264ENC_SEI_TYPE_BUF_PERIOD    = (1 << 0),
    MMPF_H264ENC_SEI_TYPE_PIC_TIMING    = (1 << 1),
    MMPF_H264ENC_SEI_TYPE_ITU_T_T35     = (1 << 4),
    MMPF_H264ENC_SEI_TYPE_MAX           = (1 << 5)
} MMPF_H264ENC_SEI_TYPE;
#endif

#if (SUPPORT_VUI_INFO == 1)
typedef struct _MMPF_H264ENC_SEI_PARAM {
    struct {
        MMP_ULONG init_cpb_removal_delay[2][VUI_HRD_MAX_CPB_CNT];
        MMP_ULONG init_cpb_removal_delay_offset[2][VUI_HRD_MAX_CPB_CNT];
    } BUF_PERIOD;
    struct {
        MMP_ULONG cpb_removal_delay;
        MMP_ULONG dpb_output_delay; 
    } PIC_TIMING;
    struct {
        MMP_UBYTE county_code;
        MMP_UBYTE ext_byte_en;
        MMP_UBYTE ext_byte;
        MMP_UBYTE payload_len;
        MMP_UBYTE *payload;
    } ITU_T_T35;
} MMPF_H264ENC_SEI_PARAM;
#endif

typedef struct _MMPF_H264ENC_SYNTAX_ELEMENT {
    MMP_LONG    type;           //!< type of syntax element for data part.
    MMP_LONG    value1;         //!< numerical value of syntax element
    MMP_LONG    value2;         //!< for blocked symbols, e.g. run/level
    MMP_LONG    len;            //!< length of code
    MMP_LONG    inf;            //!< info part of UVLC code
    MMP_ULONG   bitpattern;     //!< UVLC bitpattern
    MMP_LONG    context;        //!< CABAC context

  //!< for mapping of syntaxElement to UVLC
  //void    (*mapping)(int value1, int value2, int* len_ptr, int* info_ptr);
} MMPF_H264ENC_SYNTAX_ELEMENT;

typedef struct _MMPF_H264ENC_BS_PARSE_INFO {
    MMP_ULONG   *tail;
	MMP_ULONG   *start;
    MMP_ULONG   bufA;
    MMP_ULONG   bufB;
    MMP_ULONG   initpos;
    MMP_ULONG   buf_length;
	MMP_ULONG   bitpos;
    MMP_ULONG   bits_count;
    //MMP_ULONG   eof;
    MMP_ULONG   golomb_zeros;
} MMPF_H264ENC_BS_PARSE_INFO;

typedef struct _MMPF_H264ENC_BS_INFO {
    //MMP_LONG buffer_size;       ///< BS buffer size      
    MMP_LONG byte_pos;          ///< current position in bitstream;
    MMP_LONG bits_to_go;        ///< current bitcounter

    //MMP_LONG stored_byte_pos;   ///< storage for position in bitstream;
    //MMP_LONG stored_bits_to_go; ///< storage for bitcounter
    //MMP_LONG byte_pos_skip;     ///< storage for position in bitstream;
    //MMP_LONG bits_to_go_skip;   ///< storage for bitcounter
    //MMP_LONG write_flag;        ///< Bitstream contains data and needs to be written

    MMP_UBYTE byte_buf;         ///< current buffer for last written byte
    //MMP_UBYTE stored_byte_buf;  ///< storage for buffer of last written byte
    //MMP_UBYTE byte_buf_skip;    ///< current buffer for last written byte
    MMP_UBYTE *streamBuffer;    ///< actual buffer for written bytes
} MMPF_H264ENC_BS_INFO;

typedef struct _MMPF_H264ENC_HRD_INFO {
    MMP_ULONG cpb_cnt_minus1;                                   // ue(v)
    MMP_ULONG bit_rate_scale;                                   // u(4)
    MMP_ULONG cpb_size_scale;                                   // u(4)
        MMP_ULONG bit_rate_value_minus1 [VUI_HRD_MAX_CPB_CNT];  // ue(v)
        MMP_ULONG cpb_size_value_minus1 [VUI_HRD_MAX_CPB_CNT];  // ue(v)
        MMP_ULONG cbr_flag              [VUI_HRD_MAX_CPB_CNT];  // u(1)
    MMP_ULONG initial_cpb_removal_delay_length_minus1;          // u(5)
    MMP_ULONG cpb_removal_delay_length_minus1;                  // u(5)
    MMP_ULONG dpb_output_delay_length_minus1;                   // u(5)
    MMP_ULONG time_offset_length;                               // u(5)
} MMPF_H264ENC_HRD_INFO;

typedef struct _MMPF_H264ENC_VUI_INFO {
    MMP_BOOL      aspect_ratio_info_present_flag;               // u(1)
        MMP_ULONG aspect_ratio_idc;                             // u(8)
            MMP_ULONG sar_width;                                // u(16)
            MMP_ULONG sar_height;                               // u(16)
    MMP_BOOL      overscan_info_present_flag;                   // u(1)
        MMP_BOOL      overscan_appropriate_flag;                // u(1)
    MMP_BOOL      video_signal_type_present_flag;                   // u(1)
        MMP_ULONG video_format;                                   // u(3)
        MMP_BOOL      video_full_range_flag;                          // u(1)
        MMP_BOOL      colour_description_present_flag;                // u(1)
            MMP_ULONG colour_primaries;                             // u(8)
            MMP_ULONG transfer_characteristics;                     // u(8)
            MMP_ULONG matrix_coefficients;                          // u(8)
    MMP_BOOL      chroma_location_info_present_flag;                // u(1)
        MMP_ULONG  chroma_sample_loc_type_top_field;               // ue(v)
        MMP_ULONG  chroma_sample_loc_type_bottom_field;            // ue(v)
    MMP_BOOL      timing_info_present_flag;                         // u(1)
        MMP_ULONG num_units_in_tick;                              // u(32)
        MMP_ULONG time_scale;                                     // u(32)
        MMP_BOOL      fixed_frame_rate_flag;                          // u(1)
    MMP_BOOL      nal_hrd_parameters_present_flag;                  // u(1)
        MMPF_H264ENC_HRD_INFO nal_hrd_parameters;                      // hrd_paramters_t
    MMP_BOOL      vcl_hrd_parameters_present_flag;                  // u(1)
        MMPF_H264ENC_HRD_INFO vcl_hrd_parameters;                      // hrd_paramters_t
    // if ((nal_hrd_parameters_present_flag || (vcl_hrd_parameters_present_flag))
        MMP_BOOL      low_delay_hrd_flag;                             // u(1)
    MMP_BOOL      pic_struct_present_flag;                        // u(1)
    MMP_BOOL      bitstream_restriction_flag;                       // u(1)
        MMP_BOOL      motion_vectors_over_pic_boundaries_flag;        // u(1)
        MMP_ULONG max_bytes_per_pic_denom;                        // ue(v)
        MMP_ULONG max_bits_per_mb_denom;                          // ue(v)
        MMP_ULONG log2_max_mv_length_vertical;                    // ue(v)
        MMP_ULONG log2_max_mv_length_horizontal;                  // ue(v)
        MMP_ULONG num_reorder_frames;                             // ue(v)
        MMP_ULONG max_dec_frame_buffering;                        // ue(v)
} MMPF_H264ENC_VUI_INFO;

#define MAX_SLICE_GROUP_MINUS1 8
typedef struct _MMPF_H264ENC_PPS_INFO {
    MMP_BOOL  Valid; // indicates the parameter set is valid
    MMP_ULONG pic_parameter_set_id;                               // ue(v)
    MMP_ULONG seq_parameter_set_id;                               // ue(v)
    MMP_BOOL  entropy_coding_mode_flag;                           // u(1)
  
    MMP_BOOL  transform_8x8_mode_flag;                            // u(1)
  
    //MMP_BOOL  pic_scaling_matrix_present_flag;                    // u(1)
    //MMP_LONG  pic_scaling_list_present_flag[12];                  // u(1)
  
    // if( pic_order_cnt_type < 2 )  in the sequence parameter set
    MMP_BOOL  pic_order_present_flag;                           // u(1)
    MMP_ULONG num_slice_groups_minus1;                          // ue(v)
    #if 0 // only support 1 slice group
        MMP_ULONG slice_group_map_type;                      // ue(v)
    // if( slice_group_map_type = = 0 )
        MMP_ULONG run_length_minus1[MAX_SLICE_GROUP_MINUS1]; // ue(v)
    // else if( slice_group_map_type = = 2 )
        MMP_ULONG top_left[MAX_SLICE_GROUP_MINUS1];         // ue(v)
        MMP_ULONG bottom_right[MAX_SLICE_GROUP_MINUS1];     // ue(v)
    // else if( slice_group_map_type = = 3 || 4 || 5
        MMP_BOOL   slice_group_change_direction_flag;            // u(1)
        MMP_ULONG slice_group_change_rate_minus1;               // ue(v)
    // else if( slice_group_map_type = = 6 )
        MMP_ULONG pic_size_in_map_units_minus1;                 // ue(v)
        MMP_UBYTE slice_group_id[1];                            // complete MBAmap u(v)
    #endif
  
    MMP_LONG  num_ref_idx_l0_active_minus1;                     // ue(v)
    MMP_LONG  num_ref_idx_l1_active_minus1;                     // ue(v)
    MMP_BOOL  weighted_pred_flag;                               // u(1)
    MMP_ULONG weighted_bipred_idc;                              // u(2)
    MMP_LONG  pic_init_qp_minus26;                              // se(v)
    MMP_LONG  pic_init_qs_minus26;                              // se(v)
    MMP_LONG  chroma_qp_index_offset;                           // se(v)
  
    MMP_LONG  cb_qp_index_offset;                               // se(v)
    MMP_LONG  cr_qp_index_offset;                               // se(v)
  
    MMP_BOOL  deblocking_filter_control_present_flag;           // u(1)
    MMP_BOOL  constrained_intra_pred_flag;                      // u(1)
    MMP_BOOL  redundant_pic_cnt_present_flag;                   // u(1)
} MMPF_H264ENC_PPS_INFO;

#define MAX_REF_FRAME_IN_POC_CYCLE 256
typedef struct _MMPF_H264ENC_SPS_INFO {
    MMP_BOOL    Valid; // indicates the parameter set is valid

    MMP_ULONG   profile_idc;                                      // u(8)
    MMP_BOOL    constrained_set0_flag;                            // u(1)
    MMP_BOOL    constrained_set1_flag;                            // u(1)
    MMP_BOOL    constrained_set2_flag;                            // u(1)
    MMP_BOOL    constrained_set3_flag;                            // u(1)
    MMP_BOOL    constrained_set4_flag;                            // u(1)
    MMP_BOOL    constrained_set5_flag;                            // u(1)
    MMP_BOOL    constrained_set6_flag;                            // u(1)
    MMP_ULONG   level_idc;                                        // u(8)
    MMP_ULONG   seq_parameter_set_id;                             // ue(v)
    MMP_ULONG   chroma_format_idc;                                // ue(v)

    MMP_BOOL    seq_scaling_matrix_present_flag;                // u(1) => always 0
        //MMP_LONG   seq_scaling_list_present_flag[12];              // u(1)

    MMP_ULONG   bit_depth_luma_minus8;                          // ue(v)
    MMP_ULONG   bit_depth_chroma_minus8;                        // ue(v)
    MMP_ULONG   log2_max_frame_num_minus4;                      // ue(v)
    MMP_ULONG   pic_order_cnt_type;
    // if( pic_order_cnt_type == 0 )
    MMP_ULONG   log2_max_pic_order_cnt_lsb_minus4;              // ue(v)
    // else if( pic_order_cnt_type == 1 )
    #if (SUPPORT_POC_TYPE_1 == 1)
    MMP_BOOL delta_pic_order_always_zero_flag;                  // u(1)
        MMP_LONG    offset_for_non_ref_pic;                     // se(v)
        MMP_LONG    offset_for_top_to_bottom_field;             // se(v)
        MMP_ULONG   num_ref_frames_in_pic_order_cnt_cycle;      // ue(v)
        // for( i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
            MMP_LONG    offset_for_ref_frame[MAX_REF_FRAME_IN_POC_CYCLE];   // se(v)
    #endif //(SUPPORT_POC_TYPE_1 == 1)
    MMP_ULONG   num_ref_frames;                                   // ue(v)
    MMP_BOOL    gaps_in_frame_num_value_allowed_flag;             // u(1)
    MMP_ULONG   pic_width_in_mbs_minus1;                          // ue(v)
    MMP_ULONG   pic_height_in_map_units_minus1;                   // ue(v)
    MMP_BOOL    frame_mbs_only_flag;                              // u(1)
    // if( !frame_mbs_only_flag )
        MMP_BOOL    mb_adaptive_frame_field_flag;                   // u(1)
    MMP_BOOL    direct_8x8_inference_flag;                        // u(1)
    MMP_BOOL    frame_cropping_flag;                              // u(1)
        MMP_ULONG   frame_cropping_rect_left_offset;                // ue(v)
        MMP_ULONG   frame_cropping_rect_right_offset;               // ue(v)
        MMP_ULONG   frame_cropping_rect_top_offset;                 // ue(v)
        MMP_ULONG   frame_cropping_rect_bottom_offset;              // ue(v)
    MMP_BOOL    vui_parameters_present_flag;                      // u(1)
    #if (SUPPORT_VUI_INFO == 1)
        MMPF_H264ENC_VUI_INFO vui_seq_parameters;
    #endif
} MMPF_H264ENC_SPS_INFO;

typedef struct _MMPF_H264ENC_NALU_INFO {
    //MMP_ULONG   startcodeprefix_len;
    MMPF_H264ENC_NALU_TYPE nal_unit_type;
    MMPF_H264ENC_NAL_REF_IDC nal_ref_idc;
    //MMP_ULONG   forbidden_bit;  ///< should always 0
    MMP_UBYTE   temporal_id;    ///< SVC extension
} MMPF_H264ENC_NALU_INFO;

typedef struct _MMPF_H264ENC_SLICE_INFO {
    MMPF_H264ENC_SLICE_TYPE type;
    MMP_LONG    first_mb_num;
    MMP_LONG    qp_delta;
    MMP_LONG    header_bits;
    //MMP_LONG    aligned_bytes;     //!< including startcode len
} MMPF_H264ENC_SLICE_INFO;

typedef struct _MMPF_H264ENC_REFLIST_INFO {
    MMP_USHORT  usMaxNumRefFrame;       ///< max num ref frames
    MMP_USHORT  usActiveNumRefFrame;    ///< ref frame(long+short) in list
    MMP_USHORT  usActiveNumLTRefFrame;  ///< long term ref frame in list
} MMPF_H264ENC_REFLIST_INFO;

typedef struct _MMPF_H264ENC_ENC_INFO {
    // Sequence Level
    MMP_ULONG   eid;
    MMPF_H264ENC_SPS_INFO sps;
    MMPF_H264ENC_PPS_INFO pps;
    MMP_USHORT  profile;
    MMP_UBYTE   constraint_flag;
    MMP_USHORT  mb_w;
    MMP_USHORT  mb_h;
    MMP_USHORT  usResvMaxWidth;
    MMP_USHORT  usResvMaxHeight;
    MMP_USHORT  level;
    MMP_USHORT  succ_b_count;
    MMP_USHORT  b_frame_num;
    MMP_USHORT  gop_size;
    MMPF_VIDENC_SYNCFRAME_TYPE GopType;
    MMP_ULONG   mv_addr;
    MMP_BOOL    rc_skippable;           ///< false, rc should not skip ant frames
    MMP_ULONG   stream_bitrate;         ///< total bitrate for all layers
    MMP_ULONG   layer_bitrate[MAX_NUM_TMP_LAYERS]; ///< per-layer bitrate
    MMP_ULONG   layer_lb_size[MAX_NUM_TMP_LAYERS]; ///< leakybucket size per-layer in ms
    MMP_ULONG   layer_fps_ratio[MAX_NUM_TMP_LAYERS]; ///< fps ratio for each layer
    RC_CONFIG_PARAM rc_config[MAX_NUM_TMP_LAYERS]; ///< config for rc temporal layers
    void        *layer_rc_hdl[MAX_NUM_TMP_LAYERS]; ///< rc handler per-layer
    MMP_BOOL    bGlobalRc;              ///< True if all layer share the same RC
    MMP_UBYTE   priority_id[MAX_NUM_TMP_LAYERS]; ///< priority_id config by temporal layer
    MMP_UBYTE   total_layers;

    MMP_BOOL    dblk_disable_idc;
    MMP_SHORT   dblk_alpha_div2;
    MMP_SHORT   dblk_beta_div2;
    MMPF_H264ENC_ENTROPY_MODE entropy_mode;
    MMPF_H264ENC_POC_TYPE   poc_type;
    MMPF_VIDENC_CROPPING crop;

    MMP_BOOL    bIdrPicFlag;
    MMP_BOOL    num_ref_idx_override_flag;
    MMP_USHORT  num_ref_idx_l0_active_minus1;
    MMP_USHORT  num_ref_idx_l1_active_minus1;

    // runtime, picture level
    // encode parameters
    MMP_ULONG   total_frames;           ///< total encoded frames
    MMP_ULONG   init_idr_repeat_count;  ///< initial idr frame numbers
    // hw dependent
    MMP_ULONG   idr_id;                 ///< id for idr frames
    MMP_LONG    frame_num;
    MMP_LONG    poc;
    MMP_ULONG   enc_frame_num;          ///< count since previous I frame
    MMP_ULONG   prev_ref_num;           ///< increase when cur frame can be ref
    MMPF_VIDENC_CURBUF_MODE     CurBufMode;
    MMPF_VIDENC_CURBUF_MODE     CurRTModeSwitchingSig;
    MMPF_VIDENC_BUFCFG          CurBufFrmConfig;
    MMPF_VIDENC_BUFCFG          CurBufRTConfig;
    MMPF_VIDENC_RTFCTL_MODE     CurRTFctlMode;
    MMP_BOOL    ref_rot_mode;
    MMP_UBYTE   qp_tune_mode;           ///< mb, row, slice, frame
    MMP_UBYTE   qp_tune_size;           ///< unit size
    MMP_USHORT  inter_cost_th;          ///< reset 1024
    MMP_USHORT  intra_cost_adj;         ///< reset 18
    MMP_USHORT  inter_cost_bias;        ///< reset 0
    MMP_USHORT  cohdr_sei_rbsp_bits;
    MMP_UBYTE   cohdr_pnalu_bits;
    MMP_UBYTE   cohdr_slicehdr_bits;
    MMPF_H264ENC_COHDR_OPTION   cohdr_option;
    MMP_ULONG   temporal_id;            ///< I:0, P:1(ref),2(non-ref)
    MMP_ULONG   timestamp;
    #if (SUPPORT_VUI_INFO == 1)
    MMPF_H264ENC_SEI_PARAM  sei_param;  ///< parameter for supported sei message
    MMP_BOOL                sei_en;     ///< inser sei message or not
    #endif
    MMP_BOOL    video_full_range;       ///< indicate video input is 16-255
    // flow control
    MMP_USHORT  init_buffered_frm;      ///< init to b_frame_num+1
    MMP_BOOL    drop_next;              ///< true if base layer drop
    MMP_BOOL    bRefListReorder;
    MMP_LONG    qp_max[3];              ///< max qp
    MMP_LONG    qp_min[3];              ///< min qp
    MMP_LONG    CurRcQP[MAX_NUM_TMP_LAYERS][3];
    MMP_LONG    LayerCbrQpIdxOffset[MAX_NUM_TMP_LAYERS][3];
    MMP_LONG    LayerCrQpIdxOffset[MAX_NUM_TMP_LAYERS][3];

    MMP_ULONG   ulParamQueueRdWrapIdx;  ///< MS3Byte wrap, LSByte idx
    MMP_ULONG   ulParamQueueWrWrapIdx;  ///< MS3Byte wrap, LSByte idx
    MMPF_VIDENC_PARAM_CTL   ParamQueue[MAX_NUM_PARAM_CTL];
    MMPF_VIDENC_PICCTL  pic_ctl_flag;   ///< pic control
    MMPF_3GPMGR_FRAME_TYPE  OpFrameType;
    MMP_BOOL    OpIdrPic;
    MMP_BOOL    OpInsParset;
    MMP_UBYTE   CurRTSrcPipeId;
    MMPF_VIDENC_COLORMODE InputColormode;

    MMPF_VIDMGR_HANDLE  *MgrHandle;

    // runtime, slice level 
    MMPF_H264ENC_NALU_INFO      nalu;
    MMPF_H264ENC_SLICE_INFO     slice;
    MMPF_H264ENC_REFLIST_INFO   RefListInfo;
    MMPF_VIDENC_REFGENBUF_MODE  RefGenBufMode;

    MMPF_3GPMGR_FRAME_TYPE cur_frm_type;
    MMPF_VIDENC_FRAME cur_frm;
    MMPF_VIDENC_FRAME ref_frm;
    MMPF_VIDENC_FRAME rec_frm;
    MMPF_VIDENC_FRAME CurRTModeLineBuf[2];
    MMPF_VIDENC_FRAMEBUF_BD RefBufBound;
    MMPF_VIDENC_FRAMEBUF_BD GenBufBound;

    MMPF_VIDENC_QUEUE cur_frm_queue;    ///< in display order
    MMP_UBYTE   enc_frm_buf;            ///< buffer idx for encode

    MMP_ULONG   ulMaxFpsRes;            ///< host controled max fps
    MMP_ULONG   ulMaxFpsInc;            ///< host controled max fps
    MMP_ULONG   ulFpsInputRes;          ///< input framerate resolution
    MMP_ULONG   ulFpsInputInc;          ///< input framerate increament
    MMP_ULONG   ulFpsInputAcc;          ///< input framerate accumulate
    MMP_ULONG   ulFpsOutputRes;         ///< enc framerate resolution
    MMP_ULONG   ulFpsOutputInc;         ///< enc framerate increament
    MMP_ULONG   ulFpsOutputAcc;         ///< enc framerate accumulate

    MMP_ULONG   cur_frm_bs_addr;        ///< bs addr for current enc frame
    MMP_ULONG   cur_frm_bs_high_bd;     ///< bs addr for current enc frame
    MMP_ULONG   cur_frm_bs_low_bd;      ///< bs addr for current enc frame
    MMP_ULONG   cur_frm_wr_size;        ///< accumulating size write to bs buf
    MMP_ULONG   cur_frm_rd_size;        ///< accumulating size read from bs buf
    MMP_ULONG   HwCurFrmParsetNum;     ///< hw sps/pps slice num
    MMP_ULONG   cur_frm_slice_num;      ///< hw total slice num(incluing parset)
    MMP_ULONG   cur_frm_slice_idx;      ///< 

    MMP_UBYTE   slice_mode;
    MMP_ULONG   slice_size;

    // buffer addr and buffer for modify header
    MMP_ULONG   cur_slice_wr_idx;       ///< 
    MMP_ULONG   cur_slice_rd_idx;       ///< 
    MMP_ULONG   cur_slice_xhdr_sum;     ///< generated slice header size sum
    MMP_ULONG   cur_slice_parsed_sum;   ///< parsed slice header size sum
    #if (MGR_PROC_EN == 1)
    
    #ifdef MAX_SLICE_HDR_NUM
    MMP_UBYTE   cur_slice_xhdr_buf[VIDENC_WORKBUF_SET_NUM][MAX_SLICE_HDR_NUM][MAX_XHDR_SIZE];
    MMP_ULONG   cur_slice_xhdr_len[VIDENC_WORKBUF_SET_NUM][MAX_SLICE_HDR_NUM];
    #else
    MMP_UBYTE   cur_slice_xhdr_buf[VIDENC_WORKBUF_SET_NUM][MAX_SLICE_NUM][MAX_XHDR_SIZE];
    MMP_ULONG   cur_slice_xhdr_len[VIDENC_WORKBUF_SET_NUM][MAX_SLICE_NUM];
    #endif
    MMP_ULONG   cur_slice_data_addr[VIDENC_WORKBUF_SET_NUM][MAX_SLICE_NUM];
    MMP_ULONG   cur_slice_data_len[VIDENC_WORKBUF_SET_NUM][MAX_SLICE_NUM];
    MMP_ULONG   FrmWorkBufSetWrIdx;
    MMP_ULONG   FrmWorkBufSetWrWrap;
    MMP_ULONG   FrmWorkBufSetRdIdx;
    MMP_ULONG   FrmWorkBufSetRdWrap;
    #endif

    #if (H264_SW_CODING_EN == 1)
    MMPF_H264ENC_INS_FLAG SwParsetInsFlag;
    MMP_ULONG   sps_len;
    MMP_ULONG   pps_len;
    MMP_UBYTE   sps_buf[MAX_SLICE_HDR_SIZE*2];
    MMP_UBYTE   pps_buf[MAX_SLICE_HDR_SIZE*2];
    MMP_UBYTE   sei_buf[MAX_SLICE_HDR_SIZE*4];
    #endif
    MMP_UBYTE   prefix_nal_buf[MAX_SLICE_HDR_SIZE];
    MMP_UBYTE   slice_hdr_ebsp_buf[MAX_SLICE_HDR_SIZE];
#if H264_CBR_PADDING_EN==1
    MMP_BOOL    cbr_padding_en ;
#endif
    VidEncEndCallBackFunc *EncStartCallback;
    VidEncEndCallBackFunc *EncReStartCallback;
    VidEncEndCallBackFunc *EncEndCallback;
} MMPF_H264ENC_ENC_INFO;

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================



//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

MMP_ERR MMPF_H264ENC_Initilize(MMP_UBYTE id);
MMP_ERR MMPF_H264ENC_InitResolution(MMP_UBYTE id, MMP_USHORT width, MMP_USHORT height);
MMP_ERR MMPF_H264ENC_InitRefList(MMP_ULONG *ulCurAddr, MMPF_H264ENC_ENC_INFO *pEnc);
MMP_ERR MMPF_H264ENC_InitMVBuf(MMP_ULONG addr, MMPF_H264ENC_ENC_INFO *EncHandle);
MMP_ERR MMPF_H264ENC_InitSpsConfig(MMP_USHORT sps_id, MMPF_H264ENC_ENC_INFO *enc);
MMP_ERR MMPF_H264ENC_InitPpsConfig(MMP_USHORT pps_id, MMP_LONG CbrIndexOffset, MMP_LONG CrIndexOffset,
                                    MMPF_H264ENC_ENC_INFO *enc);
MMP_ULONG MMPF_H264ENC_GenerateSPS (MMP_UBYTE *nalu_buf, MMPF_H264ENC_SPS_INFO *sps);
MMP_ULONG MMPF_H264ENC_GeneratePPS (MMP_UBYTE *nalu_buf, 
            MMPF_H264ENC_PPS_INFO *pps, MMPF_H264ENC_SPS_INFO *sps);
MMP_ULONG MMPF_H264ENC_PreCalculateEp3Num (MMP_UBYTE *rbsp, MMP_ULONG rbsp_size);
MMP_UBYTE MMPF_H264ENC_GenerateNaluHeader (MMPF_H264ENC_NALU_INFO *nalu_inf);
#if (SUPPORT_VUI_INFO == 1)
MMP_ULONG MMPF_H264ENC_GenerateSEI(MMP_UBYTE *nalu_buf, MMPF_H264ENC_SEI_PARAM *pSeiParam,
            MMPF_H264ENC_SEI_TYPE PayloadType, MMPF_H264ENC_BYTESTREAM_TYPE ByteStrType,
            MMPF_H264ENC_ENC_INFO *pEnc);
#endif
MMP_ULONG MMPF_H264ENC_GenerateSliceHeader (MMP_UBYTE               *header_buf,
                                            MMP_UBYTE               *padding_bits,
                                            MMPF_H264ENC_HDRCODING_MODE HdrCodingMode,
                                            MMPF_H264ENC_ENC_INFO   *enc);
MMP_ULONG MMPF_H264ENC_GetAudNaluLen (void);
MMP_ULONG MMPF_H264ENC_GenerateAudHeader (MMP_UBYTE *header_buf,
            MMPF_3GPMGR_FRAME_TYPE FrameType);
MMP_ULONG MMPF_H264ENC_GenerateSVCHeader (MMP_UBYTE *header_buf,
            MMPF_H264ENC_NALU_TYPE nalu_type, MMP_UBYTE priority_id, MMP_UBYTE temporal_id);
MMP_ERR MMPF_H264ENC_ParseSliceHeader (MMP_LONG *header_bits, MMP_LONG *start_mb_num,
            MMP_LONG *qp_delta, MMP_UBYTE *header_buf, MMPF_H264ENC_ENC_INFO *enc);
MMP_ERR MMPF_H264ENC_PreProcSlice (MMPF_H264ENC_ENC_INFO *pEnc);
MMP_ERR MMPF_H264ENC_ProcSlice (MMP_ULONG *ulSliceIdx, MMP_ULONG *ulSliceLenBuf,
            MMPF_H264ENC_ENC_INFO *pEnc);
MMPF_H264ENC_ENC_INFO *MMPF_H264ENC_GetHandle(MMP_UBYTE ubEncId);
MMP_ULONG MMPF_H264ENC_CalculateTemporalLayerId (MMP_ULONG ulCodedFrameNum, MMP_ULONG ulTotalLayer);
MMP_ERR MMPF_H264ENC_SetSeiEnable (MMP_UBYTE ubEncId, MMP_BOOL bEnable);
MMP_ERR MMPF_H264ENC_SetProfile(MMP_USHORT usProfile, MMPF_H264ENC_ENC_INFO *enc);
MMP_ERR MMPF_H264ENC_SetLevel (MMP_USHORT usLevel, MMPF_H264ENC_ENC_INFO *enc);
MMP_ERR MMPF_H264ENC_SetEntropyMode (MMPF_H264ENC_ENTROPY_MODE EntropyMode, MMPF_H264ENC_ENC_INFO *enc);
MMP_ERR MMPF_H264ENC_SetCropping(MMPF_H264ENC_ENC_INFO *pEnc, MMP_BOOL bEnable);
MMP_ERR MMPF_H264ENC_SetRealtimeLineBuf(MMPF_H264ENC_ENC_INFO *pEnc, MMPF_VIDENC_FRAME CurLine[]);
MMP_ERR MMPF_H264ENC_RestoreRefList(MMPF_H264ENC_ENC_INFO *enc);
void MMPF_H264ENC_SetRCFineTune(MMPF_H264ENC_ENC_INFO *pEnc);
MMP_ERR MMPF_H264ENC_RestoreTransform (MMP_BOOL bTrans8x8, MMP_BOOL bIntra8x8);
MMP_ERR MMPF_H264ENC_RestoreProfile (MMP_USHORT usProfile, MMPF_H264ENC_ENTROPY_MODE Entropy);
MMP_ERR MMPF_H264ENC_RestoreReg(MMPF_H264ENC_ENC_INFO *pEnc);
MMP_ERR MMPF_H264ENC_UpdateRefList(MMPF_H264ENC_ENC_INFO *EncHandle);
MMP_ERR MMPF_H264ENC_UpdateModeDecision(MMPF_H264ENC_ENC_INFO *pEnc);
MMP_ERR MMPF_H264ENC_SartInstance(MMP_UBYTE ubEncID,VidEncEndCallBackFunc *startCB);
MMP_ERR MMPF_H264ENC_EnablePrefixNALU(MMP_UBYTE ubEncID,MMP_BOOL en);
/// @}

#endif	// _MMPF_H264ENC_H_
