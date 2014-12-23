package cn.bingoogolapple.qrcode.core;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Color;
import android.graphics.Point;
import android.hardware.Camera;
import android.os.Handler;
import android.util.AttributeSet;
import android.view.View;
import android.widget.FrameLayout;

public abstract class QRCodeView extends FrameLayout implements Camera.PreviewCallback {
    private Camera mCamera;
    private CameraPreview mPreview;
    private ScanBoxView mScanBoxView;
    protected ResultHandler mResultHandler;
    protected Handler mHandler;

    public QRCodeView(Context context, AttributeSet attributeSet) {
        this(context, attributeSet, 0);
    }

    protected QRCodeView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        mHandler = new Handler();
        initView(context, attrs);
    }

    public void setResultHandler(ResultHandler resultHandler) {
        mResultHandler = resultHandler;
    }

    public void initView(Context context, AttributeSet attrs) {
        mPreview = new CameraPreview(getContext());
        mScanBoxView = new ScanBoxView(getContext());

        TypedArray typedArray = context.obtainStyledAttributes(attrs, R.styleable.QRCodeView);
        final int count = typedArray.getIndexCount();
        for (int i = 0; i < count; i++) {
            int attr = typedArray.getIndex(i);
            if (attr == R.styleable.QRCodeView_qrcv_topOffset) {
                int topOffset = (int) DisplayUtils.dp2px(context, 50);
                topOffset = typedArray.getDimensionPixelSize(attr, topOffset);
                mScanBoxView.setTopOffset(topOffset);
            } else if (attr == R.styleable.QRCodeView_qrcv_cornerSize) {
                int cornerSize = (int) DisplayUtils.dp2px(context, 2);
                cornerSize = typedArray.getDimensionPixelSize(attr, cornerSize);
                mScanBoxView.setCornerSize(cornerSize);
            } else if (attr == R.styleable.QRCodeView_qrcv_cornerLength) {
                int cornerLength = (int) DisplayUtils.dp2px(context, 20);
                cornerLength = typedArray.getDimensionPixelSize(attr, cornerLength);
                mScanBoxView.setCornerLength(cornerLength);
            } else if (attr == R.styleable.QRCodeView_qrcv_scanLineSize) {
                int scanLineSize = (int) DisplayUtils.dp2px(context, 1);
                scanLineSize = typedArray.getDimensionPixelSize(attr, scanLineSize);
                mScanBoxView.setScanLineSize(scanLineSize);
            } else if (attr == R.styleable.QRCodeView_qrcv_rectWidth) {
                Point screenResolution = DisplayUtils.getScreenResolution(getContext());
                int rectWidth = Math.min(screenResolution.x, screenResolution.y) * 3 / 5;
                rectWidth = typedArray.getDimensionPixelSize(attr, rectWidth);
                mScanBoxView.setRectWidth(rectWidth);
            } else if (attr == R.styleable.QRCodeView_qrcv_maskColor) {
                int maskColor = 0x60000000;
                maskColor = typedArray.getColor(attr, maskColor);
                mScanBoxView.setMaskColor(maskColor);
            } else if (attr == R.styleable.QRCodeView_qrcv_cornerColor) {
                int cornerColor = Color.WHITE;
                cornerColor = typedArray.getColor(attr, cornerColor);
                mScanBoxView.setCornerColor(cornerColor);
            } else if (attr == R.styleable.QRCodeView_qrcv_scanLineColor) {
                int scanLineColor = Color.WHITE;
                scanLineColor = typedArray.getColor(attr, scanLineColor);
                mScanBoxView.setScanLineColor(scanLineColor);
            }
        }
        typedArray.recycle();

        addView(mPreview);
        addView(mScanBoxView);
    }

    public void showScanRect() {
        if (mScanBoxView != null) {
            mScanBoxView.setVisibility(View.VISIBLE);
        }
    }

    public void hiddenScanRect() {
        if (mScanBoxView != null) {
            mScanBoxView.setVisibility(View.GONE);
        }
    }

    public void startCamera() {
        try {
            mCamera = Camera.open();
        } catch (Exception e) {
            if (mResultHandler != null) {
                mResultHandler.handleCameraError();
            }
        }
        if (mCamera != null) {
            mPreview.setCamera(mCamera, this);
            mPreview.initCameraPreview();
        }
    }

    public void stopCamera() {
        if (mCamera != null) {
            mPreview.stopCameraPreview();
            mPreview.setCamera(null, null);
            mCamera.release();
            mCamera = null;
        }
    }

    public void startSpotDelay() {
        mHandler.postDelayed(mOneShotPreviewCallbackTask, 1500);
    }

    public void startSpot() {
        mHandler.postDelayed(mOneShotPreviewCallbackTask, 300);
    }

    public void stopSpot() {
        mCamera.setOneShotPreviewCallback(null);
        mHandler.removeCallbacks(mOneShotPreviewCallbackTask);
    }

    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
        Camera.Parameters parameters = camera.getParameters();
        Camera.Size size = parameters.getPreviewSize();
        int width = size.width;
        int height = size.height;

        byte[] rotatedData = new byte[data.length];
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++)
                rotatedData[x * height + height - y - 1] = data[x + y * width];
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

    public interface ResultHandler {
        public void handleResult(String result);

        public void handleCameraError();
    }
}