package cn.bingoogolapple.qrcode.core;

import android.content.res.Configuration;
import android.hardware.Camera;
import android.os.AsyncTask;
import android.os.Build;

public class ProcessDataTask extends AsyncTask<Void, Void, String> {
    private Camera mCamera;
    private byte[] mData;
    private Delegate mDelegate;
    private int rotationCount;
    private int orientation;

    public ProcessDataTask(int rotationCount, Camera camera, byte[] data, Delegate delegate, int orientation) {
        this.mCamera = camera;
        this.mData = data;
        this.mDelegate = delegate;
        this.rotationCount = rotationCount;
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
        Camera.Parameters parameters = mCamera.getParameters();
        Camera.Size size = parameters.getPreviewSize();
        int width = size.width;
        int height = size.height;

        if (orientation == Configuration.ORIENTATION_PORTRAIT) {
            if (rotationCount == 1 || rotationCount == 3) {
                int tmp = width;
                width = height;
                height = tmp;
            }

        }

        try {
            if (mDelegate == null) {
                return null;
            }
            return mDelegate.processData(mData, width, height, false);
        } catch (Exception e1) {
            try {
                return mDelegate.processData(mData, width, height, true);
            } catch (Exception e2) {
                return null;
            }
        }
    }

    public interface Delegate {
        String processData(byte[] data, int width, int height, boolean isRetry);
    }
}
