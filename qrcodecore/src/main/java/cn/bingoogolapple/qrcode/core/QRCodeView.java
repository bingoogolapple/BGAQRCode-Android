package cn.bingoogolapple.qrcode.core;

import android.content.Context;
import android.content.res.TypedArray;
import android.hardware.Camera;
import android.os.Handler;
import android.util.AttributeSet;
import android.view.View;
import android.widget.FrameLayout;

public abstract class QRCodeView extends FrameLayout implements Camera.PreviewCallback {
    protected Camera mCamera;
    protected CameraPreview mPreview;
    protected ScanBoxView mScanBoxView;
    protected Delegate mDelegate;
    protected Handler mHandler;

    public QRCodeView(Context context, AttributeSet attributeSet) {
        this(context, attributeSet, 0);
    }

    public QRCodeView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        mHandler = new Handler();
        initView(context, attrs);
    }

    public void setResultHandler(Delegate delegate) {
        mDelegate = delegate;
    }

    private void initView(Context context, AttributeSet attrs) {
        mPreview = new CameraPreview(getContext());
        mScanBoxView = new ScanBoxView(getContext());

        initAttrs(context, attrs);

        addView(mPreview);
        addView(mScanBoxView);
    }

    private void initAttrs(Context context, AttributeSet attrs) {
        TypedArray typedArray = context.obtainStyledAttributes(attrs, R.styleable.QRCodeView);
        final int count = typedArray.getIndexCount();
        for (int i = 0; i < count; i++) {
            initAttr(typedArray.getIndex(i), typedArray);
        }
        typedArray.recycle();
    }

    private void initAttr(int attr, TypedArray typedArray) {
        if (attr == R.styleable.QRCodeView_qrcv_topOffset) {
            mScanBoxView.setTopOffset(typedArray.getDimensionPixelSize(attr, mScanBoxView.getTopOffset()));
        } else if (attr == R.styleable.QRCodeView_qrcv_cornerSize) {
            mScanBoxView.setCornerSize(typedArray.getDimensionPixelSize(attr, mScanBoxView.getCornerSize()));
        } else if (attr == R.styleable.QRCodeView_qrcv_cornerLength) {
            mScanBoxView.setCornerLength(typedArray.getDimensionPixelSize(attr, mScanBoxView.getCornerLength()));
        } else if (attr == R.styleable.QRCodeView_qrcv_scanLineSize) {
            mScanBoxView.setScanLineSize(typedArray.getDimensionPixelSize(attr, mScanBoxView.getScanLineSize()));
        } else if (attr == R.styleable.QRCodeView_qrcv_rectWidth) {
            mScanBoxView.setRectWidth(typedArray.getDimensionPixelSize(attr, mScanBoxView.getRectWidth()));
        } else if (attr == R.styleable.QRCodeView_qrcv_maskColor) {
            mScanBoxView.setMaskColor(typedArray.getColor(attr, mScanBoxView.getMaskColor()));
        } else if (attr == R.styleable.QRCodeView_qrcv_cornerColor) {
            mScanBoxView.setCornerColor(typedArray.getColor(attr, mScanBoxView.getCornerColor()));
        } else if (attr == R.styleable.QRCodeView_qrcv_scanLineColor) {
            mScanBoxView.setScanLineColor(typedArray.getColor(attr, mScanBoxView.getScanLineColor()));
        } else if (attr == R.styleable.QRCodeView_qrcv_scanLineMargin) {
            mScanBoxView.setScanLineMargin(typedArray.getDimensionPixelSize(attr, mScanBoxView.getScanLineMargin()));
        } else if (attr == R.styleable.QRCodeView_qrcv_scanLineDrawable) {
            mScanBoxView.setScanLineDrawable(typedArray.getDrawable(attr));
        } else if (attr == R.styleable.QRCodeView_qrcv_borderSize) {
            mScanBoxView.setBorderSize(typedArray.getDimensionPixelSize(attr, mScanBoxView.getBorderSize()));
        } else if (attr == R.styleable.QRCodeView_qrcv_borderColor) {
            mScanBoxView.setBorderColor(typedArray.getColor(attr, mScanBoxView.getBorderColor()));
        }
    }

    /**
     * 显示扫描框
     */
    public void showScanRect() {
        if (mScanBoxView != null) {
            mScanBoxView.setVisibility(View.VISIBLE);
        }
    }

    /**
     * 隐藏扫描框
     */
    public void hiddenScanRect() {
        if (mScanBoxView != null) {
            mScanBoxView.setVisibility(View.GONE);
        }
    }

    /**
     * 打开摄像头开始预览，但是并未开始识别
     */
    public void startCamera() {
        if (mCamera != null) {
            return;
        }

        try {
            mCamera = Camera.open();
        } catch (Exception e) {
            if (mDelegate != null) {
                mDelegate.onScanQRCodeOpenCameraError();
            }
        }
        if (mCamera != null) {
            mPreview.setCamera(mCamera);
            mPreview.initCameraPreview();
        }
    }

    /**
     * 关闭摄像头预览，并且隐藏扫描框
     */
    public void stopCamera() {
        stopSpotAndHiddenRect();
        if (mCamera != null) {
            mPreview.stopCameraPreview();
            mPreview.setCamera(null);
            mCamera.release();
            mCamera = null;
        }
    }

    /**
     * 延迟1.5秒后开始识别
     */
    public void startSpot() {
        startSpotDelay(1500);
    }

    /**
     * 延迟delay毫秒后开始识别
     *
     * @param delay
     */
    public void startSpotDelay(int delay) {
        startCamera();
        // 开始前先移除之前的任务
        mHandler.removeCallbacks(mOneShotPreviewCallbackTask);
        mHandler.postDelayed(mOneShotPreviewCallbackTask, delay);
    }

    /**
     * 停止识别
     */
    public void stopSpot() {
        if (mCamera != null) {
            mCamera.setOneShotPreviewCallback(null);
        }
        if (mHandler != null) {
            mHandler.removeCallbacks(mOneShotPreviewCallbackTask);
        }
    }

    /**
     * 停止识别，并且隐藏扫描框
     */
    public void stopSpotAndHiddenRect() {
        stopSpot();
        hiddenScanRect();
    }

    /**
     * 显示扫描框，并且延迟1.5秒后开始识别
     */
    public void startSpotAndShowRect() {
        startSpot();
        showScanRect();
    }

    /**
     * 打开闪光灯
     */
    public void openFlashlight() {
        mPreview.openFlashlight();
    }

    /**
     * 关闭散光灯
     */
    public void closeFlashlight() {
        mPreview.closeFlashlight();
    }

    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
        Camera.Parameters parameters = camera.getParameters();
        Camera.Size size = parameters.getPreviewSize();
        int width = size.width;
        int height = size.height;

        byte[] rotatedData = new byte[data.length];
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                rotatedData[x * height + height - y - 1] = data[x + y * width];
            }
        }
        int tmp = width;
        width = height;
        height = tmp;
        data = rotatedData;

        handleData(data, width, height, camera);
    }

    protected abstract void handleData(byte[] data, int width, int height, Camera camera);

    private Runnable mOneShotPreviewCallbackTask = new Runnable() {
        @Override
        public void run() {
            if (mCamera != null) {
                mCamera.setOneShotPreviewCallback(QRCodeView.this);
            }
        }
    };

    public interface Delegate {
        /**
         * 处理扫描结果
         *
         * @param result
         */
        void onScanQRCodeSuccess(String result);

        /**
         * 处理打开相机出错
         */
        void onScanQRCodeOpenCameraError();
    }
}