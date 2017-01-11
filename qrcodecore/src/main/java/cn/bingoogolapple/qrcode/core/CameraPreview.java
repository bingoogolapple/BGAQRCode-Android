package cn.bingoogolapple.qrcode.core;

import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.Point;
import android.hardware.Camera;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;


public class CameraPreview extends SurfaceView implements SurfaceHolder.Callback {
    private static final String TAG = CameraPreview.class.getSimpleName();
    private Camera mCamera;
    private boolean mPreviewing = true;
    private boolean mAutoFocus = true;
    private boolean mSurfaceCreated = false;
    private CameraConfigurationManager mCameraConfigurationManager;

    public CameraPreview(Context context) {
        super(context);
    }

    public void setCamera(Camera camera) {
        mCamera = camera;
        if (mCamera != null) {
            mCameraConfigurationManager = new CameraConfigurationManager(getContext());
            mCameraConfigurationManager.initFromCameraParameters(mCamera);

            getHolder().addCallback(this);
            if (mPreviewing) {
                requestLayout();
            } else {
                showCameraPreview();
            }
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {
        mSurfaceCreated = true;
    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i2, int i3) {
        if (surfaceHolder.getSurface() == null) {
            return;
        }
        stopCameraPreview();

        post(new Runnable() {
            public void run() {
                showCameraPreview();
            }
        });
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
        mSurfaceCreated = false;
        stopCameraPreview();
    }

    public void showCameraPreview() {
        if (mCamera != null) {
            try {
                mPreviewing = true;
                mCamera.setPreviewDisplay(getHolder());

                mCameraConfigurationManager.setDesiredCameraParameters(mCamera);
                mCamera.startPreview();
                if (mAutoFocus) {
                    mCamera.autoFocus(autoFocusCB);
                }
            } catch (Exception e) {
                Log.e(TAG, e.toString(), e);
            }
        }
    }

    public void stopCameraPreview() {
        if (mCamera != null) {
            try {
                removeCallbacks(doAutoFocus);

                mPreviewing = false;
                mCamera.cancelAutoFocus();
                mCamera.setOneShotPreviewCallback(null);
                mCamera.stopPreview();
            } catch (Exception e) {
                Log.e(TAG, e.toString(), e);
            }
        }
    }

    public void openFlashlight() {
        if (flashLightAvailable()) {
            mCameraConfigurationManager.openFlashlight(mCamera);
        }
    }

    public void closeFlashlight() {
        if (flashLightAvailable()) {
            mCameraConfigurationManager.closeFlashlight(mCamera);
        }
    }

    @Override
    public void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int width = getDefaultSize(getSuggestedMinimumWidth(), widthMeasureSpec);
        int height = getDefaultSize(getSuggestedMinimumHeight(), heightMeasureSpec);
        if (mCameraConfigurationManager != null
                && mCameraConfigurationManager.getCameraResolution() != null) {
            Point cameraResolution = mCameraConfigurationManager.getCameraResolution();
            // 取出来的cameraResolution高宽值与屏幕的高宽顺序是相反的
            int cameraPreviewWidth = cameraResolution.y;
            int cameraPreviewHeight = cameraResolution.x;
            if (width * 1f / height < cameraPreviewWidth * 1f / cameraPreviewHeight) {
                float ratio = cameraPreviewHeight * 1f / cameraPreviewWidth;
                width = (int) (height / ratio + 0.5f);
            } else {
                float ratio = cameraPreviewWidth * 1f / cameraPreviewHeight;
                height = (int) (width / ratio + 0.5f);
            }
        }
        super.onMeasure(MeasureSpec.makeMeasureSpec(width, MeasureSpec.EXACTLY),
                MeasureSpec.makeMeasureSpec(height, MeasureSpec.EXACTLY));
    }


    private boolean flashLightAvailable() {
        return mCamera != null && mPreviewing && mSurfaceCreated && getContext().getPackageManager().hasSystemFeature(PackageManager.FEATURE_CAMERA_FLASH);
    }

    private Runnable doAutoFocus = new Runnable() {
        public void run() {
            if (mCamera != null && mPreviewing && mAutoFocus && mSurfaceCreated) {
                try {
                    mCamera.autoFocus(autoFocusCB);
                } catch (Exception e) {
                }
            }
        }
    };

    Camera.AutoFocusCallback autoFocusCB = new Camera.AutoFocusCallback() {
        public void onAutoFocus(boolean success, Camera camera) {
            postDelayed(doAutoFocus, 1000);
        }
    };

}