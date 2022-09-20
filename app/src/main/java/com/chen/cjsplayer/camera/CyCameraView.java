package com.chen.cjsplayer.camera;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.util.AttributeSet;
import android.view.Surface;
import android.view.WindowManager;

import com.chen.cjsplayer.opengl.CyEGLSurfaceView;

/**
 * @author Created by CHEN on 2019/6/21
 * @email 188669@163.comgetTextureId
 */
public class CyCameraView extends CyEGLSurfaceView {
    private CyCameraRender cyCameraRender;
    private CyCamera cyCamera;

    private int textureId = -1;
    private int cameraId = Camera.CameraInfo.CAMERA_FACING_BACK;

    public CyCameraView(Context context) {
        this(context, null);
    }

    public CyCameraView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public CyCameraView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        cyCameraRender = new CyCameraRender(context);
        cyCamera = new CyCamera(context);
        setRender(cyCameraRender);
        previewAngle(context);
        cyCameraRender.setOnSurfaceCreateListener(new CyCameraRender.OnSurfaceCreateListener() {
            @Override
            public void onSurfaceCreate(SurfaceTexture surfaceTexture ,int tid) {
                cyCamera.initCamera(surfaceTexture, cameraId);
                textureId = tid;
            }
        });
    }

    private void previewAngle(Context context) {
        int angle = ((WindowManager) context.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay().getRotation();
        cyCameraRender.resetMatrix();
        switch (angle) {
            case Surface.ROTATION_0:
                if (cameraId == Camera.CameraInfo.CAMERA_FACING_BACK){
                    cyCameraRender.setAngle(90, 0, 0, 1);
                    cyCameraRender.setAngle(180, 1, 0, 0);
                }else {
                    cyCameraRender.setAngle(90f, 0f, 0f, 1f);
                }
                break;
            case Surface.ROTATION_90:
                if (cameraId == Camera.CameraInfo.CAMERA_FACING_BACK){
                    cyCameraRender.setAngle(180, 0, 0, 1);
                    cyCameraRender.setAngle(180, 0, 1, 0);
                }else {
                    cyCameraRender.setAngle(90f, 0f, 0f, 1f);
                }
                break;
            case Surface.ROTATION_180:
                if (cameraId == Camera.CameraInfo.CAMERA_FACING_BACK){
                    cyCameraRender.setAngle(90f, 0.0f, 0f, 1f);
                    cyCameraRender.setAngle(180f, 0.0f, 1f, 0f);
                }else {
                    cyCameraRender.setAngle(-90, 0f, 0f, 1f);
                }
                break;
            case Surface.ROTATION_270:
                if (cameraId == Camera.CameraInfo.CAMERA_FACING_BACK){
                    cyCameraRender.setAngle(180f, 0.0f, 1f, 0f);
                }else {
                    cyCameraRender.setAngle(0f, 0f, 0f, 1f);
                }
                break;
        }
    }
    public int getTextureId() {
        return textureId;
    }
    public void onDestory() {
        if (cyCamera != null) {
            cyCamera.stopPreview();
        }
    }
}
