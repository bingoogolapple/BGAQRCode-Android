package cn.bingoogolapple.qrcode.core;

import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.Point;
import android.graphics.Rect;
import android.hardware.Camera;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.util.Collections;

public class CameraPreview extends SurfaceView implements SurfaceHolder.Callback {
    private Camera mCamera;
    private boolean mPreviewing = true;
    private boolean mSurfaceCreated = false;
    private boolean mIsTouchFocusing = false;
    private float mOldDist = 1f;
    private CameraConfigurationManager mCameraConfigurationManager;
    private Delegate mDelegate;

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

    void setDelegate(Delegate delegate) {
        mDelegate = delegate;
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
        if (mCamera != null) {
            try {
                mPreviewing = true;
                SurfaceHolder surfaceHolder = getHolder();
                surfaceHolder.setKeepScreenOn(true);
                mCamera.setPreviewDisplay(surfaceHolder);

                mCameraConfigurationManager.setDesiredCameraParameters(mCamera);
                mCamera.startPreview();
                if (mDelegate != null) {
                    mDelegate.onStartPreview();
                }
                startContinuousAutoFocus();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    void stopCameraPreview() {
        if (mCamera != null) {
            try {
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

    private boolean flashLightAvailable() {
        return isPreviewing() && getContext().getPackageManager().hasSystemFeature(PackageManager.FEATURE_CAMERA_FLASH);
    }

    void onScanBoxRectChanged(Rect scanRect) {
        if (mCamera == null || scanRect == null || scanRect.left <= 0 || scanRect.top <= 0) {
            return;
        }
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

        BGAQRCodeUtil.d("扫码框发生变化触发对焦测光");
        handleFocusMetering(scanRect.centerX(), scanRect.centerY(), scanRect.width(), scanRect.height());
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (!isPreviewing()) {
            return super.onTouchEvent(event);
        }

        if (event.getPointerCount() == 1 && (event.getAction() & MotionEvent.ACTION_MASK) == MotionEvent.ACTION_UP) {
            if (mIsTouchFocusing) {
                return true;
            }
            mIsTouchFocusing = true;
            BGAQRCodeUtil.d("手指触摸触发对焦测光");
            float centerX = event.getX();
            float centerY = event.getY();
            if (BGAQRCodeUtil.isPortrait(getContext())) {
                float temp = centerX;
                centerX = centerY;
                centerY = temp;
            }
            int focusSize = BGAQRCodeUtil.dp2px(getContext(), 120);
            handleFocusMetering(centerX, centerY, focusSize, focusSize);
        }

        if (event.getPointerCount() == 2) {
            switch (event.getAction() & MotionEvent.ACTION_MASK) {
                case MotionEvent.ACTION_POINTER_DOWN:
                    mOldDist = BGAQRCodeUtil.calculateFingerSpacing(event);
                    break;
                case MotionEvent.ACTION_MOVE:
                    float newDist = BGAQRCodeUtil.calculateFingerSpacing(event);
                    if (newDist > mOldDist) {
                        handleZoom(true, mCamera);
                    } else if (newDist < mOldDist) {
                        handleZoom(false, mCamera);
                    }
                    break;
            }
        }
        return true;
    }

    private static void handleZoom(boolean isZoomIn, Camera camera) {
        Camera.Parameters params = camera.getParameters();
        if (params.isZoomSupported()) {
            int zoom = params.getZoom();
            if (isZoomIn && zoom < params.getMaxZoom()) {
                BGAQRCodeUtil.d("放大");
                zoom++;
            } else if (!isZoomIn && zoom > 0) {
                BGAQRCodeUtil.d("缩小");
                zoom--;
            } else {
                BGAQRCodeUtil.d("既不放大也不缩小");
            }
            params.setZoom(zoom);
            camera.setParameters(params);
        } else {
            BGAQRCodeUtil.d("不支持缩放");
        }
    }

    private void handleFocusMetering(float originFocusCenterX, float originFocusCenterY,
            int originFocusWidth, int originFocusHeight) {
        try {
            boolean isNeedUpdate = false;
            Camera.Parameters focusMeteringParameters = mCamera.getParameters();
            Camera.Size size = focusMeteringParameters.getPreviewSize();
            if (focusMeteringParameters.getMaxNumFocusAreas() > 0) {
                BGAQRCodeUtil.d("支持设置对焦区域");
                isNeedUpdate = true;
                Rect focusRect = BGAQRCodeUtil.calculateFocusMeteringArea(1f,
                        originFocusCenterX, originFocusCenterY,
                        originFocusWidth, originFocusHeight,
                        size.width, size.height);
                BGAQRCodeUtil.printRect("对焦区域", focusRect);
                focusMeteringParameters.setFocusAreas(Collections.singletonList(new Camera.Area(focusRect, 1000)));
                focusMeteringParameters.setFocusMode(Camera.Parameters.FOCUS_MODE_MACRO);
            } else {
                BGAQRCodeUtil.d("不支持设置对焦区域");
            }

            if (focusMeteringParameters.getMaxNumMeteringAreas() > 0) {
                BGAQRCodeUtil.d("支持设置测光区域");
                isNeedUpdate = true;
                Rect meteringRect = BGAQRCodeUtil.calculateFocusMeteringArea(1.5f,
                        originFocusCenterX, originFocusCenterY,
                        originFocusWidth, originFocusHeight,
                        size.width, size.height);
                BGAQRCodeUtil.printRect("测光区域", meteringRect);
                focusMeteringParameters.setMeteringAreas(Collections.singletonList(new Camera.Area(meteringRect, 1000)));
            } else {
                BGAQRCodeUtil.d("不支持设置测光区域");
            }

            if (isNeedUpdate) {
                mCamera.cancelAutoFocus();
                mCamera.setParameters(focusMeteringParameters);
                mCamera.autoFocus(new Camera.AutoFocusCallback() {
                    public void onAutoFocus(boolean success, Camera camera) {
                        if (success) {
                            BGAQRCodeUtil.d("对焦测光成功");
                        } else {
                            BGAQRCodeUtil.e("对焦测光失败");
                        }
                        startContinuousAutoFocus();
                    }
                });
            } else {
                mIsTouchFocusing = false;
            }
        } catch (Exception e) {
            e.printStackTrace();
            BGAQRCodeUtil.e("对焦测光失败：" + e.getMessage());
            startContinuousAutoFocus();
        }
    }

    /**
     * 连续对焦
     */
    private void startContinuousAutoFocus() {
        mIsTouchFocusing = false;
        if (mCamera == null) {
            return;
        }
        try {
            Camera.Parameters parameters = mCamera.getParameters();
            // 连续对焦
            parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE);
            mCamera.setParameters(parameters);
            // 要实现连续的自动对焦，这一句必须加上
            mCamera.cancelAutoFocus();
        } catch (Exception e) {
            BGAQRCodeUtil.e("连续对焦失败");
        }
    }

    boolean isPreviewing() {
        return mCamera != null && mPreviewing && mSurfaceCreated;
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

    interface Delegate {
        void onStartPreview();
    }
}