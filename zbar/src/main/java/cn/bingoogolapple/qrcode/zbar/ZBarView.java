package cn.bingoogolapple.qrcode.zbar;

import android.content.Context;
import android.graphics.Rect;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.util.Log;

import net.sourceforge.zbar.Config;
import net.sourceforge.zbar.Image;
import net.sourceforge.zbar.ImageScanner;
import net.sourceforge.zbar.Symbol;
import net.sourceforge.zbar.SymbolSet;

import java.nio.charset.StandardCharsets;

import cn.bingoogolapple.qrcode.core.QRCodeView;

public class ZBarView extends QRCodeView {

    static {
        System.loadLibrary("iconv");
    }

    private ImageScanner mScanner;

    public ZBarView(Context context, AttributeSet attributeSet) {
        this(context, attributeSet, 0);
    }

    public ZBarView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        setupScanner();
    }

    public void setupScanner() {
        mScanner = new ImageScanner();
        mScanner.setConfig(0, Config.X_DENSITY, 3);
        mScanner.setConfig(0, Config.Y_DENSITY, 3);

        mScanner.setConfig(Symbol.NONE, Config.ENABLE, 0);
        for (BarcodeFormat format : BarcodeFormat.ALL_FORMATS) {
            mScanner.setConfig(format.getId(), Config.ENABLE, 1);
        }
    }

    @Override
    public String processData(byte[] data, int width, int height, boolean isRetry) {
        String result = null;
        Image barcode = new Image(width, height, "Y800");

        Rect rect = mScanBoxView.getScanBoxAreaRect(height);
        /*if (rect != null && !isRetry && rect.left + rect.width() <= width && rect.top + rect.height() <= height) {
            barcode.setCrop(rect.left, rect.top, rect.width(), rect.height());

        }*/
        Log.i("ZBarView", "analysis data ready,width=" + width + ",height=" + height);
        Log.i("ZBarView", "analysis data ready2,rect.left=" + rect.left + ", rect.top=" + rect.top + ", rect.width()=" + rect.width() + ", rect.height=" + rect.height());
        barcode.setData(data);

        barcode.setCrop(rect.left, rect.top, rect.width(), rect.height());
        Log.i("ZBarView", "analysis data start");
        result = processData(barcode);
        Log.i("ZBarView", "analysis data result=" + result);
        return result;
    }

    private String processData(Image barcode) {
        String result = "";
        Log.i("ZBarView", "into analysis");
        int value = mScanner.scanImage(barcode);
        Log.i("ZBarView", "analysis data scanImage value=" + value);
        if (value != 0) {
            SymbolSet symbols = mScanner.getResults();
            for (Symbol symbol : symbols) {
                String symData;
                if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.KITKAT) {
                    symData = new String(symbol.getDataBytes(), StandardCharsets.UTF_8);
                } else {
                    symData = symbol.getData();
                }
                if (!TextUtils.isEmpty(symData)) {
                    result = symData;
                    break;
                }
            }
        }
        return result;
    }
}