package cn.bingoogolapple.qrcode.core;

import android.hardware.Camera;
import android.os.AsyncTask;
import android.os.Build;

public class ProcessDataTask extends AsyncTask<Void, Void, String> {
    private Camera mCamera;
    private byte[] mData;
    private Delegate mDelegate;
    private int orientation;

    public ProcessDataTask(Camera camera, byte[] data, Delegate delegate, int orientation) {
        mCamera = camera;
        mData = data;
        mDelegate = delegate;
        this.orientation = orientation;
    }

    public ProcessDataTask perform() {
        if (Build.VERSION.SDK_INT >= 11) {
            executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
        } else {
            execute();
        }
        return this;
    }

    public void cancelTask() {
        if (getStatus() != Status.FINISHED) {
            cancel(true);
        }
    }

    @Override
    protected void onCancelled() {
        super.onCancelled();
        mDelegate = null;
    }

    @Override
    protected String doInBackground(Void... params) {
        int width = 0;
        int height = 0;
        byte[] data = mData;
        try {
            Camera.Parameters parameters = mCamera.getParameters();
            Camera.Size size = parameters.getPreviewSize();
            width = size.width;
            height = size.height;

            if (orientation == BGAQRCodeUtil.ORIENTATION_PORTRAIT) {
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
                    BGAQRCodeUtil.d("失败失败重试");
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

    public interface Delegate {
        String processData(byte[] data, int width, int height, boolean isRetry);
    }
}
