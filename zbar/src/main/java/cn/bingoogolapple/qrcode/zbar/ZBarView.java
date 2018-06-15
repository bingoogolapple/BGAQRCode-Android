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
import cn.bingoogolapple.qrcode.core.ScanResult;

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

    private void setupScanner() {
        mScanner = new ImageScanner();
        mScanner.setConfig(0, Config.X_DENSITY, 3);
        mScanner.setConfig(0, Config.Y_DENSITY, 3);

        mScanner.setConfig(Symbol.NONE, Config.ENABLE, 0);
        for (BarcodeFormat format : BarcodeFormat.ALL_FORMATS) {
            mScanner.setConfig(format.getId(), Config.ENABLE, 1);
        }
    }

    @Override
    protected ScanResult processData(byte[] data, int width, int height, boolean isRetry) {
        Image barcode = new Image(width, height, "Y800");

        Rect rect = mScanBoxView.getScanBoxAreaRect(height);
        if (rect != null && !isRetry && rect.left + rect.width() <= width && rect.top + rect.height() <= height) {
            barcode.setCrop(rect.left, rect.top, rect.width(), rect.height());
        }

        barcode.setData(data);
        String result = processData(barcode);
        return new ScanResult(result);
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
    protected ScanResult processBitmapData(Bitmap bitmap) {
        try {
            int picWidth = bitmap.getWidth();
            int picHeight = bitmap.getHeight();
            Image barcode = new Image(picWidth, picHeight, "RGB4");
            int[] pix = new int[picWidth * picHeight];
            bitmap.getPixels(pix, 0, picWidth, 0, 0, picWidth, picHeight);
            barcode.setData(pix);
            String result = processData(barcode.convert("Y800"));
            return new ScanResult(result);
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }
}