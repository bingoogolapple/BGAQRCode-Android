package cn.bingoogolapple.qrcode.zxing;

import android.content.Context;
import android.hardware.Camera;
import android.os.AsyncTask;
import android.text.TextUtils;
import android.util.AttributeSet;

import com.google.zxing.BinaryBitmap;
import com.google.zxing.MultiFormatReader;
import com.google.zxing.PlanarYUVLuminanceSource;
import com.google.zxing.Result;
import com.google.zxing.common.HybridBinarizer;

import cn.bingoogolapple.qrcode.core.QRCodeView;

public class ZXingView extends QRCodeView {
    private MultiFormatReader mMultiFormatReader;


    public ZXingView(Context context, AttributeSet attributeSet) {
        this(context, attributeSet, 0);
    }

    public ZXingView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initMultiFormatReader();
    }

    private void initMultiFormatReader() {
        mMultiFormatReader = new MultiFormatReader();
        mMultiFormatReader.setHints(QRCodeDecoder.HINTS);
    }

    @Override
    protected void handleData(final byte[] data, final int width, final int height, final Camera camera) {
        new AsyncTask<Void,Void,String>() {

            @Override
            protected String doInBackground(Void... params) {
                String result = null;
                Result rawResult = null;

                try {
                    PlanarYUVLuminanceSource source = new PlanarYUVLuminanceSource(data, width, height, 0, 0, width, height, false);
                    rawResult = mMultiFormatReader.decodeWithState(new BinaryBitmap(new HybridBinarizer(source)));
                } catch (Exception e) {
                } finally {
                    mMultiFormatReader.reset();
                }

                if (rawResult != null) {
                    result = rawResult.getText();
                }
                return result;
            }

            @Override
            protected void onPostExecute(String result) {
                if (mDelegate != null && !TextUtils.isEmpty(result)) {
                    mDelegate.onScanQRCodeSuccess(result);
                } else {
                    try {
                        camera.setOneShotPreviewCallback(ZXingView.this);
                    } catch (RuntimeException e) {
                    }

                }
            }
        }.execute();
    }
}