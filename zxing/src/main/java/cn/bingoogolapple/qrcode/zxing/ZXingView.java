package cn.bingoogolapple.qrcode.zxing;

import android.content.Context;
import android.hardware.Camera;
import android.os.AsyncTask;
import android.text.TextUtils;
import android.util.AttributeSet;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.BinaryBitmap;
import com.google.zxing.DecodeHintType;
import com.google.zxing.MultiFormatReader;
import com.google.zxing.PlanarYUVLuminanceSource;
import com.google.zxing.Result;
import com.google.zxing.common.HybridBinarizer;

import java.util.ArrayList;
import java.util.EnumMap;
import java.util.List;
import java.util.Map;

import cn.bingoogolapple.qrcode.core.QRCodeView;

public class ZXingView extends QRCodeView {
    private MultiFormatReader mMultiFormatReader;
    public static final List<BarcodeFormat> ALL_FORMATS = new ArrayList<BarcodeFormat>();

    static {
        ALL_FORMATS.add(BarcodeFormat.UPC_A);
        ALL_FORMATS.add(BarcodeFormat.UPC_E);
        ALL_FORMATS.add(BarcodeFormat.EAN_13);
        ALL_FORMATS.add(BarcodeFormat.EAN_8);
        ALL_FORMATS.add(BarcodeFormat.RSS_14);
        ALL_FORMATS.add(BarcodeFormat.CODE_39);
        ALL_FORMATS.add(BarcodeFormat.CODE_93);
        ALL_FORMATS.add(BarcodeFormat.CODE_128);
        ALL_FORMATS.add(BarcodeFormat.ITF);
        ALL_FORMATS.add(BarcodeFormat.CODABAR);
        ALL_FORMATS.add(BarcodeFormat.QR_CODE);
        ALL_FORMATS.add(BarcodeFormat.DATA_MATRIX);
        ALL_FORMATS.add(BarcodeFormat.PDF_417);
    }

    public ZXingView(Context context, AttributeSet attributeSet) {
        this(context, attributeSet, 0);
    }

    public ZXingView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initMultiFormatReader();
    }

    private void initMultiFormatReader() {
        Map<DecodeHintType, Object> hints = new EnumMap<DecodeHintType, Object>(DecodeHintType.class);
        hints.put(DecodeHintType.POSSIBLE_FORMATS, ALL_FORMATS);
        mMultiFormatReader = new MultiFormatReader();
        mMultiFormatReader.setHints(hints);
    }

    @Override
    protected void handleData(final byte[] data, final int width, final int height, final Camera camera) {
        new AsyncTask<Void,Void,String>() {

            @Override
            protected String doInBackground(Void... params) {
                String result = null;
                Result rawResult = null;
                PlanarYUVLuminanceSource source = buildLuminanceSource(data, width, height);
                if (source != null) {
                    BinaryBitmap bitmap = new BinaryBitmap(new HybridBinarizer(source));
                    try {
                        rawResult = mMultiFormatReader.decodeWithState(bitmap);
                    } catch (Exception e) {
                    } finally {
                        mMultiFormatReader.reset();
                    }
                }

                if (rawResult != null) {
                    result = rawResult.getText();
                }
                return result;
            }

            @Override
            protected void onPostExecute(String result) {
                if (mResultHandler != null && !TextUtils.isEmpty(result)) {
                    mResultHandler.handleResult(result);
                } else {
                    try {
                        camera.setOneShotPreviewCallback(ZXingView.this);
                    } catch (RuntimeException e) {
                    }

                }
            }
        }.execute();
    }

    public PlanarYUVLuminanceSource buildLuminanceSource(byte[] data, int width, int height) {
        PlanarYUVLuminanceSource source = null;
        try {
            source = new PlanarYUVLuminanceSource(data, width, height, 0, 0, width, height, false);
        } catch (Exception e) {
        }
        return source;
    }
}