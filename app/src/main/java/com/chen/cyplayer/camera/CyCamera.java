package com.chen.cyplayer.camera;


import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;

import java.io.IOException;

/**
 * @author Created by CHEN on 2019/6/21
 * @email 188669@163.com
 */
public class CyCamera {
    private Camera camera;
    private SurfaceTexture surfaceTexture;

    public void initCamera(SurfaceTexture surfaceTexture, int cameraId)
    {
        this.surfaceTexture = surfaceTexture;
        setCameraParm(cameraId);

    }

    private void setCameraParm(int cameraId)
    {
        try {
            camera = Camera.open(cameraId);
            camera.setPreviewTexture(surfaceTexture);
            Camera.Parameters parameters = camera.getParameters();

            parameters.setFlashMode("off");
            parameters.setPreviewFormat(ImageFormat.NV21);

            parameters.setPictureSize(parameters.getSupportedPictureSizes().get(0).width,
                    parameters.getSupportedPictureSizes().get(0).height);
            parameters.setPreviewSize(parameters.getSupportedPreviewSizes().get(0).width,
                    parameters.getSupportedPreviewSizes().get(0).height);

            camera.setParameters(parameters);
            camera.startPreview();

        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void stopPreview()
    {
        if(camera != null)
        {
            camera.startPreview();
            camera.release();
            camera = null;
        }
    }

    public void changeCamera(int cameraId)
    {
        if(camera != null)
        {
            stopPreview();
        }
        setCameraParm(cameraId);
    }

}
