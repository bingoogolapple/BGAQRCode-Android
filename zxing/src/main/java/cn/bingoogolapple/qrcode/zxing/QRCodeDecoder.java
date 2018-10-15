package cn.bingoogolapple.qrcode.zxing;

import android.graphics.Bitmap;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.BinaryBitmap;
import com.google.zxing.DecodeHintType;
import com.google.zxing.MultiFormatReader;
import com.google.zxing.RGBLuminanceSource;
import com.google.zxing.Result;
import com.google.zxing.common.GlobalHistogramBinarizer;
import com.google.zxing.common.HybridBinarizer;

import java.util.ArrayList;
import java.util.Collections;
import java.util.EnumMap;
import java.util.List;
import java.util.Map;

import cn.bingoogolapple.qrcode.core.BGAQRCodeUtil;

/**
 * 作者:王浩 邮件:bingoogolapple@gmail.com
 * 创建时间:16/4/8 下午11:22
 * 描述:解析二维码图片。一维条码、二维码各种类型简介 https://blog.csdn.net/xdg_blog/article/details/52932707
 */
public class QRCodeDecoder {
    static final Map<DecodeHintType, Object> ALL_HINT_MAP = new EnumMap<>(DecodeHintType.class);

    static {
        List<BarcodeFormat> allFormatList = new ArrayList<>();
        allFormatList.add(BarcodeFormat.AZTEC);
        allFormatList.add(BarcodeFormat.CODABAR);
        allFormatList.add(BarcodeFormat.CODE_39);
        allFormatList.add(BarcodeFormat.CODE_93);
        allFormatList.add(BarcodeFormat.CODE_128);
        allFormatList.add(BarcodeFormat.DATA_MATRIX);
        allFormatList.add(BarcodeFormat.EAN_8);
        allFormatList.add(BarcodeFormat.EAN_13);
        allFormatList.add(BarcodeFormat.ITF);
        allFormatList.add(BarcodeFormat.MAXICODE);
        allFormatList.add(BarcodeFormat.PDF_417);
        allFormatList.add(BarcodeFormat.QR_CODE);
        allFormatList.add(BarcodeFormat.RSS_14);
        allFormatList.add(BarcodeFormat.RSS_EXPANDED);
        allFormatList.add(BarcodeFormat.UPC_A);
        allFormatList.add(BarcodeFormat.UPC_E);
        allFormatList.add(BarcodeFormat.UPC_EAN_EXTENSION);

        // 可能的编码格式
        ALL_HINT_MAP.put(DecodeHintType.POSSIBLE_FORMATS, allFormatList);
        // 花更多的时间用于寻找图上的编码，优化准确性，但不优化速度
        ALL_HINT_MAP.put(DecodeHintType.TRY_HARDER, Boolean.TRUE);
        // 复杂模式，开启 PURE_BARCODE 模式（带图片 LOGO 的解码方案）
//        ALL_HINT_MAP.put(DecodeHintType.PURE_BARCODE, Boolean.TRUE);
        // 编码字符集
        ALL_HINT_MAP.put(DecodeHintType.CHARACTER_SET, "utf-8");
    }

    static final Map<DecodeHintType, Object> ONE_DIMENSION_HINT_MAP = new EnumMap<>(DecodeHintType.class);

    static {
        List<BarcodeFormat> oneDimenFormatList = new ArrayList<>();
        oneDimenFormatList.add(BarcodeFormat.CODABAR);
        oneDimenFormatList.add(BarcodeFormat.CODE_39);
        oneDimenFormatList.add(BarcodeFormat.CODE_93);
        oneDimenFormatList.add(BarcodeFormat.CODE_128);
        oneDimenFormatList.add(BarcodeFormat.EAN_8);
        oneDimenFormatList.add(BarcodeFormat.EAN_13);
        oneDimenFormatList.add(BarcodeFormat.ITF);
        oneDimenFormatList.add(BarcodeFormat.PDF_417);
        oneDimenFormatList.add(BarcodeFormat.RSS_14);
        oneDimenFormatList.add(BarcodeFormat.RSS_EXPANDED);
        oneDimenFormatList.add(BarcodeFormat.UPC_A);
        oneDimenFormatList.add(BarcodeFormat.UPC_E);
        oneDimenFormatList.add(BarcodeFormat.UPC_EAN_EXTENSION);

        ONE_DIMENSION_HINT_MAP.put(DecodeHintType.POSSIBLE_FORMATS, oneDimenFormatList);
        ONE_DIMENSION_HINT_MAP.put(DecodeHintType.TRY_HARDER, Boolean.TRUE);
        ONE_DIMENSION_HINT_MAP.put(DecodeHintType.CHARACTER_SET, "utf-8");
    }

    static final Map<DecodeHintType, Object> TWO_DIMENSION_HINT_MAP = new EnumMap<>(DecodeHintType.class);

    static {
        List<BarcodeFormat> twoDimenFormatList = new ArrayList<>();
        twoDimenFormatList.add(BarcodeFormat.AZTEC);
        twoDimenFormatList.add(BarcodeFormat.DATA_MATRIX);
        twoDimenFormatList.add(BarcodeFormat.MAXICODE);
        twoDimenFormatList.add(BarcodeFormat.QR_CODE);

        TWO_DIMENSION_HINT_MAP.put(DecodeHintType.POSSIBLE_FORMATS, twoDimenFormatList);
        TWO_DIMENSION_HINT_MAP.put(DecodeHintType.TRY_HARDER, Boolean.TRUE);
        TWO_DIMENSION_HINT_MAP.put(DecodeHintType.CHARACTER_SET, "utf-8");
    }

    static final Map<DecodeHintType, Object> QR_CODE_HINT_MAP = new EnumMap<>(DecodeHintType.class);

    static {
        QR_CODE_HINT_MAP.put(DecodeHintType.POSSIBLE_FORMATS, Collections.singletonList(BarcodeFormat.QR_CODE));
        QR_CODE_HINT_MAP.put(DecodeHintType.TRY_HARDER, Boolean.TRUE);
        QR_CODE_HINT_MAP.put(DecodeHintType.CHARACTER_SET, "utf-8");
    }

    static final Map<DecodeHintType, Object> CODE_128_HINT_MAP = new EnumMap<>(DecodeHintType.class);

    static {
        CODE_128_HINT_MAP.put(DecodeHintType.POSSIBLE_FORMATS, Collections.singletonList(BarcodeFormat.CODE_128));
        CODE_128_HINT_MAP.put(DecodeHintType.TRY_HARDER, Boolean.TRUE);
        CODE_128_HINT_MAP.put(DecodeHintType.CHARACTER_SET, "utf-8");
    }

    static final Map<DecodeHintType, Object> EAN_13_HINT_MAP = new EnumMap<>(DecodeHintType.class);

    static {
        EAN_13_HINT_MAP.put(DecodeHintType.POSSIBLE_FORMATS, Collections.singletonList(BarcodeFormat.EAN_13));
        EAN_13_HINT_MAP.put(DecodeHintType.TRY_HARDER, Boolean.TRUE);
        EAN_13_HINT_MAP.put(DecodeHintType.CHARACTER_SET, "utf-8");
    }

    static final Map<DecodeHintType, Object> HIGH_FREQUENCY_HINT_MAP = new EnumMap<>(DecodeHintType.class);

    static {
        List<BarcodeFormat> highFrequencyFormatList = new ArrayList<>();
        highFrequencyFormatList.add(BarcodeFormat.QR_CODE);
        highFrequencyFormatList.add(BarcodeFormat.UPC_A);
        highFrequencyFormatList.add(BarcodeFormat.EAN_13);
        highFrequencyFormatList.add(BarcodeFormat.CODE_128);

        HIGH_FREQUENCY_HINT_MAP.put(DecodeHintType.POSSIBLE_FORMATS, highFrequencyFormatList);
        HIGH_FREQUENCY_HINT_MAP.put(DecodeHintType.TRY_HARDER, Boolean.TRUE);
        HIGH_FREQUENCY_HINT_MAP.put(DecodeHintType.CHARACTER_SET, "utf-8");
    }

    private QRCodeDecoder() {
    }

    /**
     * 同步解析本地图片二维码。该方法是耗时操作，请在子线程中调用。
     *
     * @param picturePath 要解析的二维码图片本地路径
     * @return 返回二维码图片里的内容 或 null
     */
    public static String syncDecodeQRCode(String picturePath) {
        return syncDecodeQRCode(BGAQRCodeUtil.getDecodeAbleBitmap(picturePath));
    }

    /**
     * 同步解析bitmap二维码。该方法是耗时操作，请在子线程中调用。
     *
     * @param bitmap 要解析的二维码图片
     * @return 返回二维码图片里的内容 或 null
     */
    public static String syncDecodeQRCode(Bitmap bitmap) {
        Result result;
        RGBLuminanceSource source = null;
        try {
            int width = bitmap.getWidth();
            int height = bitmap.getHeight();
            int[] pixels = new int[width * height];
            bitmap.getPixels(pixels, 0, width, 0, 0, width, height);
            source = new RGBLuminanceSource(width, height, pixels);
            result = new MultiFormatReader().decode(new BinaryBitmap(new HybridBinarizer(source)), ALL_HINT_MAP);
            return result.getText();
        } catch (Exception e) {
            e.printStackTrace();
            if (source != null) {
                try {
                    result = new MultiFormatReader().decode(new BinaryBitmap(new GlobalHistogramBinarizer(source)), ALL_HINT_MAP);
                    return result.getText();
                } catch (Throwable e2) {
                    e2.printStackTrace();
                }
            }
            return null;
        }
    }
}