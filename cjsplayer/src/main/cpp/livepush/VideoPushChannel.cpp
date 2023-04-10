//
// Created by 哦 on 2023/4/9.
//

#include "VideoPushChannel.h"

VideoPushChannel::VideoPushChannel() {
    pthread_mutex_init(&mutex, nullptr);
}

VideoPushChannel::~VideoPushChannel() {
    pthread_mutex_destroy(&mutex);
}

void VideoPushChannel::initVideoEncoder(int width, int height, int fps, int bitrate) {
    pthread_mutex_lock(&mutex);
    mWidth = width;
    mHeight = height;
    mFps = fps;
    mBitrate = bitrate;

    y_len = width * height;
    uv_len = y_len / 4;

    // 1.初始化
    if (videoCodec) {
        x264_encoder_close(videoCodec);
        videoCodec = nullptr;
    }

    if (pic_in) {
        x264_picture_clean(pic_in);
        delete pic_in;
        pic_in = nullptr;
    }

    // 2.初始化编码器
    x264_param_t param;
    // 设置编码器参数
    /**
     *  ultrafast 最快
     *  zerolatency 零延迟
     */
    x264_param_default_preset(&param, "ultrafast", "zerolatency");
    // 设置编码器的编码规格
    param.i_level_idc = 32;

    // 输入数据格式是 YUV420P 平面模式 YYYY YYYY VVVVV UUUU   相机数据是 NV21 YYYY VUVU VUVU
    param.i_csp = X264_CSP_I420;
    // 设置编码器的宽高
    param.i_width = width;
    param.i_height = height;

    // 推流不需要b帧 b帧效率低
    param.i_bframe = 0;

    // 设置码率控制方式
    /**
     * CQP 固定质量
     * CRF 固定码率
     * ABR 平均码率
     */
    param.rc.i_rc_method = X264_RC_CRF; // 平均码率
    // 设置码率
    param.rc.i_bitrate = bitrate / 1000; // 码率

    param.rc.i_vbv_max_bitrate = bitrate / 1000 * 1.2; // 码率上限

    // 码率控制
    param.b_vfr_input = 0; // 不可变帧率

    // 设置帧率
    // 分数形式设置 fps = 25  fps = 1/25
    param.i_fps_num = fps;
    param.i_fps_den = 1;

    // 设置关键帧间隔
    param.i_keyint_max = fps * 2; // 2秒一个关键帧

    // sps 序列参数集 pps 图像参数集
    param.b_repeat_headers = 1; // 每个关键帧前面都会有 sps pps

    // 编码线程数
    param.i_threads = 1;

    // 提交参数 profile  or baseline
    x264_param_apply_profile(&param, "baseline");

    // 输入的图像 初始化
    pic_in = new x264_picture_t;
    x264_picture_alloc(pic_in, param.i_csp, param.i_width, param.i_height);

    // 打开编码器
    videoCodec = x264_encoder_open(&param);
    if (!videoCodec) {
        LOGE("初始化编码器失败")
    } else{
        LOGE("初始化编码器成功")
    }
    pthread_mutex_unlock(&mutex);
}

void VideoPushChannel::setVideoCallback(VideoPushChannel::VideoCallback callback) {
    videoCallback = callback;
}

/**
 * 对相机数据进行编码
 * @param data
 */

/**
 * 相机 nv21 数据格式
 * y1  y2   y3  y4
 * y5  y6   y7  y8
 * y9  y10  y11 y12
 * y13 y14  y15 y16
 * v1  u1   v2  u2
 * v3  u3   v4  u4
 *
 *
 * i420 数据格式
 * y1  y2   y3  y4
 * y5  y6   y7  y8
 * y9  y10  y11 y12
 * y13 y14  y15 y16
 * u1  u2   u3  u4
 * v1  v2   v3  v4
 */
void VideoPushChannel::encodeData(signed char *data) {
    pthread_mutex_lock(&mutex);
    if (!pic_in){
        pthread_mutex_unlock(&mutex);
        return;
    }

    // 1.将相机数据 y分量 拷贝到 i420 的 y分量
    memcpy(pic_in->img.plane[0], data, y_len); // Y

    // 2.将相机数据 u分量 拷贝到 i420 的 v分量
    for (int i = 0; i < uv_len; ++i) {
        // u分量 拷贝到 i420 的 u分量
        *(pic_in->img.plane[1] + i) = *(data + y_len + i * 2 + 1); // U
        // v分量 拷贝到 i420 的 v分量
        *(pic_in->img.plane[2] + i) = *(data + y_len + i * 2); // V
    }

    // 3.编码
    x264_nal_t *pp_nal = nullptr; // 编码后的数据 nal 数组
    int pi_nal = 0; // 编码后的数据个数
    x264_picture_t *pic_out = nullptr; // 编码后的图片
    // 编码
    int ret = x264_encoder_encode(videoCodec, &pp_nal, &pi_nal, pic_in, pic_out);
    // ret x264_encoder_encode 返回 nal 的字节数，如果没有 nal 单元，则返回0或者负数
    if (ret < 0) {
        LOGE("编码失败")
        pthread_mutex_unlock(&mutex);
        return;
    }

    // 4.获取 sps pps
    int sps_len , pps_len = 0;  // sps pps 长度
    uint8_t sps[100] = {0}; // 接收 sps 序列参数集
    uint8_t pps[100] = {0}; // 接收 pps 图像参数集

    // 获取 sps pps
    for (int i = 0; i < pi_nal; ++i) {
        if (pp_nal[i].i_type == NAL_SPS){
            sps_len = pp_nal[i].i_payload - 4; // sps 长度 (要去掉起始码 00 00 00 01)
            memcpy(sps, pp_nal[i].p_payload + 4, sps_len); // 拷贝 sps
        } else if (pp_nal[i].i_type == NAL_PPS){
            pps_len = pp_nal[i].i_payload - 4; // pps 长度 (要去掉起始码 00 00 00 01)
            memcpy(pps, pp_nal[i].p_payload + 4, pps_len); // 拷贝 pps

            // 发送 sps pps
            sendSpsPps(sps, sps_len, pps, pps_len);
        } else{
            // 发送关键帧或者非关键帧
            sendFrame(pp_nal[i].i_type, pp_nal[i].p_payload, pp_nal[i].i_payload);
        }

    }

    pthread_mutex_unlock(&mutex);
}

/**
 * 发送 sps pps
 * @param sps
 * @param sps_len
 * @param pps
 * @param pps_len
 */
void VideoPushChannel::sendSpsPps(uint8_t *sps, int sps_len, uint8_t *pps, int pps_len) {

}

/**
 *
 * @param type
 * @param pPayload
 * @param payload
 */
void VideoPushChannel::sendFrame(int type, uint8_t *pPayload, int payload) {

}
