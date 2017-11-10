package cn.bingoogolapple.qrcode.core;

import android.content.Context;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.graphics.Point;
import android.hardware.Camera;
import android.os.Handler;
import android.util.Log;
import android.view.Display;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;

import java.util.List;


public class CameraPreview extends SurfaceView implements SurfaceHolder.Callback {
    private static final String TAG = CameraPreview.class.getSimpleName();
    private Camera mCamera;
    private boolean mPreviewing = true;
    private boolean mSurfaceCreated = false;
    private CameraConfigurationManager mCameraConfigurationManager;
    private Camera.PreviewCallback mPreviewCallback;
    private boolean mAutoFocus = true;
    private Handler mAutoFocusHandler;
    private boolean mShouldScaleToFill = true;
    private float mAspectTolerance = 0.1f;
    private int mCameraId = -1;

    //private int actionbarHeight;  //这里请注意，如果自定义了actionbar，在计算预览高度的地方要加上
                                    //自定义actionbar的高度，如果没有自定义actionbar,不需要加

    public CameraPreview(Context context) {
        super(context);
        mAutoFocusHandler = new Handler();
        getHolder().addCallback(this);
        getHolder().setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
    }

    public void setCamera(Camera camera, Camera.PreviewCallback previewCallback, int cameraId) {
        mCamera = camera;
        mCameraId = cameraId;
        mPreviewCallback = previewCallback;
        if (mCamera != null) {
            mCameraConfigurationManager = new CameraConfigurationManager(getContext());
            //mCameraConfigurationManager.initFromCameraParameters(mCamera);

            /*if (mPreviewing) {
                requestLayout();
            } else {
                showCameraPreview();
            }*/
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

    public void showCameraPreview() {
        if (mCamera != null) {
            try {
                getHolder().addCallback(this);
                mPreviewing = true;
                setupCameraParameters();
                mCamera.setPreviewDisplay(getHolder());
                mCamera.setDisplayOrientation(getDisplayOrientation());
                mCamera.setOneShotPreviewCallback(mPreviewCallback);
                //mCameraConfigurationManager.setDesiredCameraParameters(mCamera);
                mCamera.startPreview();
                if(mAutoFocus) {
                    if (mSurfaceCreated) { // check if surface created before using autofocus
                        safeAutoFocus();
                    } else {
                        scheduleAutoFocus(); // wait 1 sec and then do check again
                    }
                }
            } catch (Exception e) {
                Log.e(TAG, e.toString(), e);
            }
        }
    }

    public void setupCameraParameters() {
        Camera.Size optimalSize = getOptimalPreviewSize();
        Camera.Parameters parameters = mCamera.getParameters();
        parameters.setPreviewSize(optimalSize.width, optimalSize.height);
        mCamera.setParameters(parameters);
        adjustViewSize(optimalSize);
    }

    private void adjustViewSize(Camera.Size cameraSize) {
        Point previewSize = convertSizeToLandscapeOrientation(new Point(getWidth(), getHeight()));
        float cameraRatio = ((float) cameraSize.width) / cameraSize.height;
        float screenRatio = ((float) previewSize.x) / previewSize.y;

        if (screenRatio > cameraRatio) {
            setViewSize((int) (previewSize.y * cameraRatio), previewSize.y);
        } else {
            setViewSize(previewSize.x, (int) (previewSize.x / cameraRatio));
        }
    }

    private void setViewSize(int width, int height) {
        ViewGroup.LayoutParams layoutParams = getLayoutParams();
        int tmpWidth;
        int tmpHeight;
        if (getDisplayOrientation() % 180 == 0) {
            tmpWidth = width;
            tmpHeight = height;
        } else {
            tmpWidth = height;
            tmpHeight = width;
        }

        if (mShouldScaleToFill) {
            int parentWidth = ((View) getParent()).getWidth();
            int parentHeight = ((View) getParent()).getHeight();
            float ratioWidth = (float) parentWidth / (float) tmpWidth;
            float ratioHeight = (float) parentHeight / (float) tmpHeight;

            float compensation;

            if (ratioWidth > ratioHeight) {
                compensation = ratioWidth;
            } else {
                compensation = ratioHeight;
            }

            tmpWidth = Math.round(tmpWidth * compensation);
            tmpHeight = Math.round(tmpHeight * compensation);
        }

        layoutParams.width = tmpWidth;
        layoutParams.height = tmpHeight;
        setLayoutParams(layoutParams);
    }

    private Point convertSizeToLandscapeOrientation(Point size) {
        if (getDisplayOrientation() % 180 == 0) {
            return size;
        } else {
            return new Point(size.y, size.x);
        }
    }

    public int getDisplayOrientation() {
        if (mCamera == null) {
            //If we don't have a camera set there is no orientation so return dummy value
            return 0;
        }

        Camera.CameraInfo info = new Camera.CameraInfo();
        if(mCameraId == -1) {
            Camera.getCameraInfo(Camera.CameraInfo.CAMERA_FACING_BACK, info);
        } else {
            Camera.getCameraInfo(mCameraId, info);
        }

        WindowManager wm = (WindowManager) getContext().getSystemService(Context.WINDOW_SERVICE);
        Display display = wm.getDefaultDisplay();

        int rotation = display.getRotation();
        int degrees = 0;
        switch (rotation) {
            case Surface.ROTATION_0: degrees = 0; break;
            case Surface.ROTATION_90: degrees = 90; break;
            case Surface.ROTATION_180: degrees = 180; break;
            case Surface.ROTATION_270: degrees = 270; break;
        }

        int result;
        if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
            result = (info.orientation + degrees) % 360;
            result = (360 - result) % 360;  // compensate the mirror
        } else {  // back-facing
            result = (info.orientation - degrees + 360) % 360;
        }
        return result;
    }

    private Camera.Size getOptimalPreviewSize() {
        if(mCamera == null) {
            return null;
        }

        List<Camera.Size> sizes = mCamera.getParameters().getSupportedPreviewSizes();
        int w = getWidth();
        int h = getHeight();

        //记得若自定义了actionBar此处高度要加上自定义actionBar的高度，我的页面自定义actionbar高度为40dp，
        //先dp转xp，再加到高度上
        //actionbarHeight = BGAQRCodeUtil.dp2px(getContext(), 40);
        //int h = getHeight() + actionbarHeight;

        Log.i("CameraPreview", "supportWidth=" + w + ", supportHeight=" + h);
        if (BGAQRCodeUtil.getScreenOrientation(getContext()) == Configuration.ORIENTATION_PORTRAIT) {
            int portraitWidth = h;
            h = w;
            w = portraitWidth;
            Log.i("CameraPreview2", "supportWidth=" + w + ", supportHeight=" + h);
        }

        double targetRatio = (double) w / h;
        Log.i("CameraPreview2-1", "targetRadio=" + targetRatio);
        if (sizes == null) return null;

        Camera.Size optimalSize = null;
        double minDiff = Double.MAX_VALUE;

        int targetHeight = h;

        // Try to find an size match aspect ratio and size
        for (Camera.Size size : sizes) {
            Log.i("CameraPreview##", "sizes,width="+ size.width + ", height=" + size.height);
            double ratio = (double) size.width / size.height;
            Log.i("CameraPreview##1", "ratio=" + ratio);
            if (Math.abs(ratio - targetRatio) > mAspectTolerance) continue;
            if (Math.abs(size.height - targetHeight) < minDiff) {
                optimalSize = size;
                minDiff = Math.abs(size.height - targetHeight);
            }
        }
        if (optimalSize != null)
        Log.i("CameraPreview3", "supportWidth=" + optimalSize.width + ", supportHeight=" + optimalSize.height);

        // Cannot find the one match the aspect ratio, ignore the requirement
        if (optimalSize == null) {
            minDiff = Double.MAX_VALUE;
            for (Camera.Size size : sizes) {
                if (Math.abs(size.height - targetHeight) < minDiff) {
                    optimalSize = size;
                    minDiff = Math.abs(size.height - targetHeight);
                }
            }
            Log.i("CameraPreview4", "supportWidth=" + optimalSize.width + ", supportHeight=" + optimalSize.height);
        }
        return optimalSize;
    }

    public void safeAutoFocus() {
        try {
           mCamera.autoFocus(autoFocusCB);
        } catch (RuntimeException re) {
            // Horrible hack to deal with autofocus errors on Sony devices
            // See https://github.com/dm77/barcodescanner/issues/7 for example
            scheduleAutoFocus(); // wait 1 sec and then do check again
        }
    }
    private void scheduleAutoFocus() {
        mAutoFocusHandler.postDelayed(doAutoFocus, 1000);
    }

    public void stopCameraPreview() {
        if (mCamera != null) {
            try {
                mPreviewing = false;
                getHolder().removeCallback(this);
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

    /*@Override
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
    }*/


    private boolean flashLightAvailable() {
        return mCamera != null && mPreviewing && mSurfaceCreated && getContext().getPackageManager().hasSystemFeature(PackageManager.FEATURE_CAMERA_FLASH);
    }

    private Runnable doAutoFocus = new Runnable() {
        public void run() {
            if (mCamera != null && mPreviewing && mAutoFocus && mSurfaceCreated) {
                safeAutoFocus();
            }
        }
    };

    // Mimic continuous auto-focusing
    Camera.AutoFocusCallback autoFocusCB = new Camera.AutoFocusCallback() {
        public void onAutoFocus(boolean success, Camera camera) {
            scheduleAutoFocus();
        }
    };

}