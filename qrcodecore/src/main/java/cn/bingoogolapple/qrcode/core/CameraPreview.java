package cn.bingoogolapple.qrcode.core;

import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.RectF;
import android.hardware.Camera;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.util.Collections;

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
                        SurfaceHolder surfaceHolder = getHolder();
                        surfaceHolder.setKeepScreenOn(true);
                        mCamera.setPreviewDisplay(surfaceHolder);

                        mCameraConfigurationManager.setDesiredCameraParameters(mCamera);
                        mCamera.startPreview();

                        mCamera.autoFocus(mAutoFocusCallback);
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

    void onScanBoxRectChanged(Rect scanRect) {
        if (mCamera == null || scanRect == null || scanRect.left <= 0 || scanRect.top <= 0) {
            return;
        }
        try {
            Camera.Parameters parameters = mCamera.getParameters(); // 先获取当前相机的参数配置对象
            parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_AUTO); // 设置聚焦模式
            Camera.Size size = parameters.getPreviewSize();

            int centerX = scanRect.centerX();
            int centerY = scanRect.centerY();
            int rectHalfWidth = scanRect.width() / 2;
            int rectHalfHeight = scanRect.height() / 2;

            BGAQRCodeUtil.printRect("转换前", scanRect);

            if (BGAQRCodeUtil.isPortrait(getContext())) {
                int temp = centerX;
                centerX = centerY;
                centerY = temp;

                temp = rectHalfWidth;
                rectHalfWidth = rectHalfHeight;
                rectHalfHeight = temp;
            }
            scanRect = new Rect(centerX - rectHalfWidth, centerY - rectHalfHeight, centerX + rectHalfWidth, centerY + rectHalfHeight);

            BGAQRCodeUtil.printRect("转换后", scanRect);

            if (parameters.getMaxNumFocusAreas() > 0) {
                Rect focusRect = calculateFocusArea(scanRect, 1f, size);
                BGAQRCodeUtil.printRect("聚焦区域", focusRect);
                parameters.setFocusAreas(Collections.singletonList(new Camera.Area(focusRect, 1000)));
            }

            if (parameters.getMaxNumMeteringAreas() > 0) {
                Rect meteringRect = calculateFocusArea(scanRect, 1.5f, size);
                BGAQRCodeUtil.printRect("测光区域", meteringRect);
                parameters.setMeteringAreas(Collections.singletonList(new Camera.Area(meteringRect, 1000)));
            }

            removeCallbacks(doAutoFocus);
            mCamera.cancelAutoFocus(); // 先要取消掉进程中所有的聚焦功能
            mCamera.setParameters(parameters); // 一定要记得把相应参数设置给相机
            mCamera.autoFocus(mAutoFocusCallback);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * 计算扫码区域
     *
     * @param scanBox     扫码框
     * @param coefficient 比率
     */
    private Rect calculateFocusArea(Rect scanBox, float coefficient, Camera.Size previewSize) {
        int width = (int) (scanBox.width() * coefficient);
        int height = (int) (scanBox.height() * coefficient);
        float scanCenterX = scanBox.centerY();
        float scanCenterY = scanBox.centerX();

        int centerX = (int) (scanCenterX / previewSize.width * 2000 - 1000);
        int centerY = (int) (scanCenterY / previewSize.height * 2000 - 1000);

        int left = clamp(centerX - (width / 2), -1000, 1000);
        int top = clamp(centerY - (height / 2), -1000, 1000);

        RectF rectF = new RectF(left, top, left + width, top + height);
        return new Rect(Math.round(rectF.left), Math.round(rectF.top),
                Math.round(rectF.right), Math.round(rectF.bottom));
    }

    private int clamp(int x, int min, int max) {
        return Math.min(Math.max(x, min), max);
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
                    mCamera.autoFocus(mAutoFocusCallback);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    };

    Camera.AutoFocusCallback mAutoFocusCallback = new Camera.AutoFocusCallback() {
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