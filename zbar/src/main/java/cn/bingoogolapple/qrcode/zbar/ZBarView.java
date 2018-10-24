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

import java.nio.charset.StandardCharsets;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import cn.bingoogolapple.qrcode.core.BarcodeType;
import cn.bingoogolapple.qrcode.core.QRCodeView;
import cn.bingoogolapple.qrcode.core.ScanResult;

public class ZBarView extends QRCodeView {

    static {
        System.loadLibrary("iconv");
    }

    private ImageScanner mScanner;
    private List<BarcodeFormat> mFormatList;

    public ZBarView(Context context, AttributeSet attributeSet) {
        this(context, attributeSet, 0);
    }

    public ZBarView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        setupReader();
    }

    @Override
    protected void setupReader() {
        mScanner = new ImageScanner();
        mScanner.setConfig(0, Config.X_DENSITY, 3);
        mScanner.setConfig(0, Config.Y_DENSITY, 3);

        mScanner.setConfig(Symbol.NONE, Config.ENABLE, 0);

        for (BarcodeFormat format : getFormats()) {
            mScanner.setConfig(format.getId(), Config.ENABLE, 1);
        }
    }

    public Collection<BarcodeFormat> getFormats() {
        if (mBarcodeType == BarcodeType.ONE_DIMENSION) {
            return BarcodeFormat.ONE_DIMENSION_FORMAT_LIST;
        } else if (mBarcodeType == BarcodeType.TWO_DIMENSION) {
            return BarcodeFormat.TWO_DIMENSION_FORMAT_LIST;
        } else if (mBarcodeType == BarcodeType.ONLY_QR_CODE) {
            return Collections.singletonList(BarcodeFormat.QRCODE);
        } else if (mBarcodeType == BarcodeType.ONLY_CODE_128) {
            return Collections.singletonList(BarcodeFormat.CODE128);
        } else if (mBarcodeType == BarcodeType.ONLY_EAN_13) {
            return Collections.singletonList(BarcodeFormat.EAN13);
        } else if (mBarcodeType == BarcodeType.HIGH_FREQUENCY) {
            return BarcodeFormat.HIGH_FREQUENCY_FORMAT_LIST;
        } else if (mBarcodeType == BarcodeType.CUSTOM) {
            return mFormatList;
        } else {
            return BarcodeFormat.ALL_FORMAT_LIST;
        }
    }

    /**
     * 设置识别的格式
     *
     * @param barcodeType 识别的格式
     * @param formatList  barcodeType 为 BarcodeType.CUSTOM 时，必须指定该值
     */
    public void setType(BarcodeType barcodeType, List<BarcodeFormat> formatList) {
        mBarcodeType = barcodeType;
        mFormatList = formatList;

        if (mBarcodeType == BarcodeType.CUSTOM && (mFormatList == null || mFormatList.isEmpty())) {
            throw new RuntimeException("barcodeType 为 BarcodeType.CUSTOM 时 formatList 不能为空");
        }
        setupReader();
    }

    @Override
    protected ScanResult processData(byte[] data, int width, int height, boolean isRetry) {
        Image barcode = new Image(width, height, "Y800");

        Rect scanBoxAreaRect = mScanBoxView.getScanBoxAreaRect(height);
        if (scanBoxAreaRect != null && !isRetry && scanBoxAreaRect.left + scanBoxAreaRect.width() <= width
                && scanBoxAreaRect.top + scanBoxAreaRect.height() <= height) {
            barcode.setCrop(scanBoxAreaRect.left, scanBoxAreaRect.top, scanBoxAreaRect.width(), scanBoxAreaRect.height());
        }

        barcode.setData(data);
        String result = processData(barcode);
        return new ScanResult(result);
    }

    private String processData(Image barcode) {
        if (mScanner.scanImage(barcode) == 0) {
            return null;
        }

        for (Symbol symbol : mScanner.getResults()) {
            // 未能识别的格式继续遍历
            if (symbol.getType() == Symbol.NONE) {
                continue;
            }

            String symData;
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.KITKAT) {
                symData = new String(symbol.getDataBytes(), StandardCharsets.UTF_8);
            } else {
                symData = symbol.getData();
            }
            // 空数据继续遍历
            if (TextUtils.isEmpty(symData)) {
                continue;
            }

            // 处理自动缩放和定位点
            boolean isNeedAutoZoom = isNeedAutoZoom(symbol);
            if (isShowLocationPoint() || isNeedAutoZoom) {
                if (transformToViewCoordinates(symbol.getLocationPoints(), null, isNeedAutoZoom, symData)) {
                    return null;
                } else {
                    return symData;
                }
            } else {
                return symData;
            }
        }
        return null;
    }

    private boolean isNeedAutoZoom(Symbol symbol) {
        return isAutoZoom() && symbol.getType() == Symbol.QRCODE;
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