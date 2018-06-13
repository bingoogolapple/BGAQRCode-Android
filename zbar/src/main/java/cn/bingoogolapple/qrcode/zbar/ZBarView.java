package cn.bingoogolapple.qrcode.zbar;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Rect;
import android.text.TextUtils;
import android.util.AttributeSet;

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
        String result;
        Image barcode = new Image(width, height, "Y800");

        Rect rect = mScanBoxView.getScanBoxAreaRect(height);
        if (rect != null && !isRetry && rect.left + rect.width() <= width && rect.top + rect.height() <= height) {
            barcode.setCrop(rect.left, rect.top, rect.width(), rect.height());
        }

        barcode.setData(data);
        result = processData(barcode);

        return result;
    }

    private String processData(Image barcode) {
        String result = null;
        if (mScanner.scanImage(barcode) != 0) {
            SymbolSet symbolSet = mScanner.getResults();
            for (Symbol symbol : symbolSet) {
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

    @Override
    public String processBitmapData(Bitmap bitmap) {
        try {
            int picw = bitmap.getWidth();
            int pich = bitmap.getHeight();
            Image barcode = new Image(picw, pich, "RGB4");
            int[] pix = new int[picw * pich];
            bitmap.getPixels(pix, 0, picw, 0, 0, picw, pich);
            barcode.setData(pix);
            return processData(barcode.convert("Y800"));
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }
}