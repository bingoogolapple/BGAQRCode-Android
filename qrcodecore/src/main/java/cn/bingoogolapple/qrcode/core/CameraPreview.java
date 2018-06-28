package cn.bingoogolapple.qrcode.core;

import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.Point;
import android.hardware.Camera;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class CameraPreview extends SurfaceView implements SurfaceHolder.Callback {
    /**
     * 自动对焦成功后，再次对焦的延迟
     */
    public static final long DEFAULT_AUTO_FOCUS_SUCCESS_DELAY = 1000L;

    /**
     * 自动对焦失败后，再次对焦的延迟
     */
    public static final long DEFAULT_AUTO_FOCUS_FAILURE_DELAY = 500L;

    private long mAutoFocusSuccessDelay = DEFAULT_AUTO_FOCUS_SUCCESS_DELAY;
    private long mAutoFocusFailureDelay = DEFAULT_AUTO_FOCUS_FAILURE_DELAY;
    private Camera mCamera;
    private boolean mPreviewing = true;
    private boolean mSurfaceCreated = false;
    private CameraConfigurationManager mCameraConfigurationManager;

    public CameraPreview(Context context) {
        super(context);
    }

    void setCamera(Camera camera) {
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
    public void surfaceChanged(SurfaceHolder surfaceHolder, int format, int width, int height) {
        if (surfaceHolder.getSurface() == null) {
            return;
        }
        stopCameraPreview();
        showCameraPreview();
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
        mSurfaceCreated = false;
        stopCameraPreview();
    }

    public void reactNativeShowCameraPreview() {
        if (getHolder() == null || getHolder().getSurface() == null) {
            return;
        }

        stopCameraPreview();
        showCameraPreview();
    }

    private void showCameraPreview() {
        post(new Runnable() {
            @Override
            public void run() {
                if (mCamera != null) {
                    try {
                        mPreviewing = true;
                        mCamera.setPreviewDisplay(getHolder());

                        mCameraConfigurationManager.setDesiredCameraParameters(mCamera);
                        mCamera.startPreview();

                        mCamera.autoFocus(autoFocusCB);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            }
        });
    }

    void stopCameraPreview() {
        if (mCamera != null) {
            try {
                removeCallbacks(doAutoFocus);

                mPreviewing = false;
                mCamera.cancelAutoFocus();
                mCamera.setOneShotPreviewCallback(null);
                mCamera.stopPreview();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    void openFlashlight() {
        if (flashLightAvailable()) {
            mCameraConfigurationManager.openFlashlight(mCamera);
        }
    }

    void closeFlashlight() {
        if (flashLightAvailable()) {
            mCameraConfigurationManager.closeFlashlight(mCamera);
        }
    }

    @Override
    public void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int width = getDefaultSize(getSuggestedMinimumWidth(), widthMeasureSpec);
        int height = getDefaultSize(getSuggestedMinimumHeight(), heightMeasureSpec);
        if (mCameraConfigurationManager != null && mCameraConfigurationManager.getCameraResolution() != null) {
            Point cameraResolution = mCameraConfigurationManager.getCameraResolution();
            // 取出来的cameraResolution高宽值与屏幕的高宽顺序是相反的
            int cameraPreviewWidth = cameraResolution.x;
            int cameraPreviewHeight = cameraResolution.y;
            if (width * 1f / height < cameraPreviewWidth * 1f / cameraPreviewHeight) {
                float ratio = cameraPreviewHeight * 1f / cameraPreviewWidth;
                width = (int) (height / ratio + 0.5f);
            } else {
                float ratio = cameraPreviewWidth * 1f / cameraPreviewHeight;
                height = (int) (width / ratio + 0.5f);
            }
        }
        super.onMeasure(MeasureSpec.makeMeasureSpec(width, MeasureSpec.EXACTLY), MeasureSpec.makeMeasureSpec(height, MeasureSpec.EXACTLY));
    }


    private boolean flashLightAvailable() {
        return mCamera != null && mPreviewing && mSurfaceCreated && getContext().getPackageManager().hasSystemFeature(PackageManager.FEATURE_CAMERA_FLASH);
    }

    private Runnable doAutoFocus = new Runnable() {
        public void run() {
            if (mCamera != null && mPreviewing && mSurfaceCreated) {
                try {
                    mCamera.autoFocus(autoFocusCB);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    };

    Camera.AutoFocusCallback autoFocusCB = new Camera.AutoFocusCallback() {
        public void onAutoFocus(boolean success, Camera camera) {
            if (success) {
                postDelayed(doAutoFocus, getAutoFocusSuccessDelay());
            } else {
                postDelayed(doAutoFocus, getAutoFocusFailureDelay());
            }
        }
    };

    /**
     * 自动对焦成功后，再次对焦的延迟
     */
    public long getAutoFocusSuccessDelay() {
        return mAutoFocusSuccessDelay;
    }

    /**
     * 自动对焦成功后，再次对焦的延迟
     */
    public void setAutoFocusSuccessDelay(long autoFocusSuccessDelay) {
        mAutoFocusSuccessDelay = autoFocusSuccessDelay;
    }

    /**
     * 自动对焦失败后，再次对焦的延迟
     */
    public long getAutoFocusFailureDelay() {
        return mAutoFocusFailureDelay;
    }

    /**
     * 自动对焦失败后，再次对焦的延迟
     */
    public void setAutoFocusFailureDelay(long autoFocusFailureDelay) {
        mAutoFocusFailureDelay = autoFocusFailureDelay;
    }

}