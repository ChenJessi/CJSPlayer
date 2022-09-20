package com.chen.cjsplayer.util;

import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.util.Log;

import java.util.HashMap;
import java.util.Map;

/**
 * @author Created by CHEN on 2019/3/9
 * @email 188669@163.com
 * 检测是否支持硬解码
 */
public class CyVideoSupportUitl {
    private static Map<String,String> codecMap = new HashMap<String,String>();

    static {
        codecMap.put("h264", "video/avc");
    }

    public static String  findCodecName(String ffcodename){
        if (codecMap.containsKey(ffcodename)){
            return codecMap.get(ffcodename);
        }
        return "";
    }
    public static boolean isSupportCodec(String ffcodename){
        boolean supportvideo = false;
        int count = MediaCodecList.getCodecCount();
        for (int i = 0; i < count; i++) {
            String[] types = MediaCodecList.getCodecInfoAt(i).getSupportedTypes();
            for (int j = 0; j < types.length; j++) {
                if (types[j].equals(findCodecName(ffcodename))){
                    supportvideo = true;
                    break;
                };
            }
            if (supportvideo){
                break;
            }
        }
        return supportvideo;
    }
}
