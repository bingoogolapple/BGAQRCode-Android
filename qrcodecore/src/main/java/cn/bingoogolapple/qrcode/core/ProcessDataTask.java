package cn.bingoogolapple.qrcode.core;

import android.graphics.Bitmap;
import android.hardware.Camera;
import android.os.AsyncTask;

public class ProcessDataTask extends AsyncTask<Void, Void, String> {
    private Camera mCamera;
    private byte[] mData;
    private Delegate mDelegate;
    private boolean mIsPortrait;
    private String mPicturePath;
    private Bitmap mBitmap;

    ProcessDataTask(Camera camera, byte[] data, Delegate delegate, boolean isPortrait) {
        mCamera = camera;
        mData = data;
        mDelegate = delegate;
        mIsPortrait = isPortrait;
    }

    ProcessDataTask(String picturePath, Delegate delegate) {
        mPicturePath = picturePath;
        mDelegate = delegate;
    }

    ProcessDataTask(Bitmap bitmap, Delegate delegate) {
        mBitmap = bitmap;
        mDelegate = delegate;
    }

    public ProcessDataTask perform() {
        executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
        return this;
    }

    void cancelTask() {
        if (getStatus() != Status.FINISHED) {
            cancel(true);
        }
    }

    @Override
    protected void onCancelled() {
        super.onCancelled();
        mDelegate = null;
        mBitmap = null;
        mData = null;
    }

    private String processData() {
        if (mData == null) {
            return null;
        }

        int width = 0;
        int height = 0;
        byte[] data = mData;
        try {
            Camera.Parameters parameters = mCamera.getParameters();
            Camera.Size size = parameters.getPreviewSize();
            width = size.width;
            height = size.height;

            if (mIsPortrait) {
                data = new byte[mData.length];
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        data[x * height + height - y - 1] = mData[x + y * width];
                    }
                }
                int tmp = width;
                width = height;
                height = tmp;
            }

            if (mDelegate == null) {
                return null;
            }
            return mDelegate.processData(data, width, height, false);
        } catch (Exception e1) {
            e1.printStackTrace();
            try {
                if (mDelegate != null && width != 0 && height != 0) {
                    BGAQRCodeUtil.d("识别失败重试");
                    return mDelegate.processData(data, width, height, true);
                } else {
                    return null;
                }
            } catch (Exception e2) {
                e2.printStackTrace();
                return null;
            }
        }
    }

    @Override
    protected String doInBackground(Void... params) {
        if (mDelegate == null) {
            return null;
        }
        if (mPicturePath != null) {
            return mDelegate.processBitmapData(BGAQRCodeUtil.getDecodeAbleBitmap(mPicturePath));
        } else if (mBitmap != null) {
            String result = mDelegate.processBitmapData(mBitmap);
            mBitmap = null;
            return result;
        } else {
            return processData();
        }
    }

    public interface Delegate {
        String processData(byte[] data, int width, int height, boolean isRetry);

        String processBitmapData(Bitmap bitmap);
    }
}
