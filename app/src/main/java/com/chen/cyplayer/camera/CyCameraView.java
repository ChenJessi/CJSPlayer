package com.chen.cyplayer.camera;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.util.AttributeSet;

import com.chen.cyplayer.opengl.CyEGLSurfaceView;

/**
 * @author Created by CHEN on 2019/6/21
 * @email 188669@163.com
 */
public class CyCameraView extends CyEGLSurfaceView {
    private CyCameraRender render;
    private CyCamera cyCamera;

    private int cameraId = Camera.CameraInfo.CAMERA_FACING_BACK;
    public CyCameraView(Context context) {
        this(context, null);
    }

    public CyCameraView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public CyCameraView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        render = new CyCameraRender(context);
        cyCamera = new CyCamera();
        setRender(render);

        render.setOnSurfaceCreateListener(new CyCameraRender.OnSurfaceCreateListener() {
            @Override
            public void onSurfaceCreate(SurfaceTexture surfaceTexture) {
                cyCamera.initCamera(surfaceTexture, cameraId);
            }
        });
    }

    public void onDestory(){
        if (cyCamera != null){
            cyCamera.stopPreview();
        }
    }
}
