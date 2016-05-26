package cn.bingoogolapple.qrcode.core;

import android.hardware.Camera;
import android.os.AsyncTask;

public class ProcessDataTask extends AsyncTask<Void,Void,String> {
    private Camera mCamera;
    private byte[] mData;
    private Delegate mDelegate;

    public ProcessDataTask(Camera camera, byte[] data, Delegate delegate) {
        mCamera = camera;
        mData = data;
        mDelegate = delegate;
    }

    @Override
    protected String doInBackground(Void... params) {
        Camera.Parameters parameters = mCamera.getParameters();
        Camera.Size size = parameters.getPreviewSize();
        int width = size.width;
        int height = size.height;

        byte[] rotatedData = new byte[mData.length];
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                rotatedData[x * height + height - y - 1] = mData[x + y * width];
            }
        }
        int tmp = width;
        width = height;
        height = tmp;

        return mDelegate.processData(rotatedData, width, height);
    }

    public interface Delegate {
        String processData(byte[] data, int width, int height);
    }
}
