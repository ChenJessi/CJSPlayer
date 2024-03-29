//
// Created by 哦 on 2023/4/9.
//

#include "VideoPushChannel.h"

VideoPushChannel::VideoPushChannel() {
    pthread_mutex_init(&mutex, nullptr);
}

VideoPushChannel::~VideoPushChannel() {
    pthread_mutex_destroy(&mutex);
    if (videoCodec) {
        x264_encoder_close(videoCodec);
        videoCodec = nullptr;
    }
    if (pic_in) {
        x264_picture_clean(pic_in);
        delete pic_in;
        pic_in = nullptr;
    }
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
    x264_picture_t pic_out ; // 编码后的图片

    // 编码
    int ret = x264_encoder_encode(videoCodec, &pp_nal, &pi_nal, pic_in, &pic_out);
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
    // 1.组装 sps pps
    // sps pps 结构体长度
    int body_size = 5 + 8 + sps_len + 3 + pps_len;

    // 组装 sps pps
    RTMPPacket *packet = new RTMPPacket;
    RTMPPacket_Alloc(packet, body_size);// 分配内存堆区

    int i = 0; // 记录当前组装的位置
    // 2.组装 flv tag header
    packet->m_body[i++] = 0x17; // 0x17 代表 sps pps

    packet->m_body[i++] = 0x00; // 0x00 代表 AVC // sps pps
    packet->m_body[i++] = 0x00; // 0x00 代表 AVC
    packet->m_body[i++] = 0x00; // 0x00 代表 AVC
    packet->m_body[i++] = 0x00; // 0x00 代表 AVC

    packet->m_body[i++] = 0x01; // 版本号

    packet->m_body[i++] = sps[1]; // profile
    packet->m_body[i++] = sps[2]; // 兼容性
    packet->m_body[i++] = sps[3]; // 级别

    packet->m_body[i++] = 0xff; // sps 字节数

    packet->m_body[i++] = 0xe1; // sps 个数

    // sps 长度 用2个字节表示
    packet->m_body[i++] = (sps_len >> 8) & 0xff;
    packet->m_body[i++] = sps_len & 0xff;

    // sps 数据
    memcpy(&packet->m_body[i], sps, sps_len);
    i += sps_len;
    packet->m_body[i++] = 0x01; // pps 个数

    // pps 长度 用2个字节表示
    packet->m_body[i++] = (pps_len >> 8) & 0xff;
    packet->m_body[i++] = pps_len & 0xff;

    // pps 数据
    memcpy(&packet->m_body[i], pps, pps_len);
    i+= pps_len;

    // 封包处理
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO; // 包类型 视频
    packet->m_nBodySize = body_size; // 包体大小
    packet->m_nChannel = 0x14; // 通道
    packet->m_nTimeStamp = 0; // 时间戳  sps pps没有时间戳
    packet->m_hasAbsTimestamp = 0; // 时间戳绝对值
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM; // 头部类型

    // 3.发送 sps pps
    videoCallback(packet);
}

/**
 *
 * @param type 帧类型
 * @param pPayload 帧数据
 * @param payload  帧数据长度
 */
void VideoPushChannel::sendFrame(int type, uint8_t *pPayload, int payload) {
    // 去掉起始码 起始码有两种
    if (pPayload[2] == 0x00){ // 00 00 00 01
        pPayload += 4;      // 挪动指针 跳过起始码
        payload -= 4;       // 长度减去起始码长度
    } else if (pPayload[2] == 0x01){ // 00 00 01
        pPayload += 3;
        payload -= 3;
    }

    int body_size = 5 + 4 + payload; // flv tag header + nalu 数据

    // 组装 flv tag header
    auto *packet = new RTMPPacket;
    RTMPPacket_Alloc(packet, body_size); // 分配内存堆区


    if (type == NAL_SLICE_IDR){
        packet->m_body[0] = 0x17; // 0x17 代表关键帧
    } else{
        packet->m_body[0] = 0x27; // 0x27 代表非关键帧
    }

    packet->m_body[1] = 0x01; // 0x01 代表 数据帧  0x02 代表 sps pps
    packet->m_body[2] = 0x00; // 0x00 代表 AVC
    packet->m_body[3] = 0x00; // 0x00 代表 AVC
    packet->m_body[4] = 0x00; // 0x00 代表 AVC

    // 四字节表示数据长度
    packet->m_body[5] = (payload >> 24) & 0xff;
    packet->m_body[6] = (payload >> 16) & 0xff;
    packet->m_body[7] = (payload >> 8) & 0xff;
    packet->m_body[8] = payload & 0xff;


    memcpy(&packet->m_body[9], pPayload, payload); // 拷贝数据

    // 封包处理
    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO; // 包类型 视频
    packet->m_nBodySize = body_size; // 包体大小
    packet->m_nChannel = 0x14; // 通道
    packet->m_nTimeStamp = -1; // 时间戳
    packet->m_hasAbsTimestamp = 0; // 时间戳绝对值
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE; // 头部类型

    videoCallback(packet);
}
