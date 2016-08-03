package cn.bingoogolapple.qrcode.zxing;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.os.AsyncTask;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.EncodeHintType;
import com.google.zxing.MultiFormatWriter;
import com.google.zxing.common.BitMatrix;
import com.google.zxing.qrcode.decoder.ErrorCorrectionLevel;

import java.util.EnumMap;
import java.util.Map;

/**
 * 作者:王浩 邮件:bingoogolapple@gmail.com
 * 创建时间:16/4/8 下午11:22
 * 描述:创建二维码图片
 */
public class QRCodeEncoder {
    public static final Map<EncodeHintType, Object> HINTS = new EnumMap<>(EncodeHintType.class);

    static {
        HINTS.put(EncodeHintType.CHARACTER_SET, "utf-8");
        HINTS.put(EncodeHintType.ERROR_CORRECTION, ErrorCorrectionLevel.H);
        HINTS.put(EncodeHintType.MARGIN, 0);
    }

    private QRCodeEncoder() {
    }

    /**
     * 创建黑色的二维码图片
     *
     * @param content
     * @param size     图片宽高，单位为px
     * @param delegate 创建二维码图片的代理
     */
    public static void encodeQRCode(String content, int size, Delegate delegate) {
        QRCodeEncoder.encodeQRCode(content, size, Color.BLACK, delegate);
    }

    /**
     * 创建指定颜色的二维码图片
     *
     * @param content
     * @param size            图片宽高，单位为px
     * @param foregroundColor 二维码图片的前景色
     * @param delegate        创建二维码图片的代理
     */
    public static void encodeQRCode(final String content, final int size, final int foregroundColor, final Delegate delegate) {
        encodeQRCode(content, size, foregroundColor, Color.WHITE, null, delegate);
    }

    /**
     * 创建指定颜色的、带logo的二维码图片
     *
     * @param content
     * @param size            图片宽高，单位为px
     * @param foregroundColor 二维码图片的前景色
     * @param logo            二维码图片的logo
     * @param delegate        创建二维码图片的代理
     */
    public static void encodeQRCode(final String content, final int size, final int foregroundColor, final Bitmap logo, final Delegate delegate) {
        encodeQRCode(content, size, foregroundColor, Color.WHITE, logo, delegate);
    }

    /**
     * 创建指定颜色的、带logo的二维码图片
     *
     * @param content
     * @param size            图片宽高，单位为px
     * @param foregroundColor 二维码图片的前景色
     * @param backgroundColor 二维码图片的背景色
     * @param logo            二维码图片的logo
     * @param delegate        创建二维码图片的代理
     */
    public static void encodeQRCode(final String content, final int size, final int foregroundColor, final int backgroundColor, final Bitmap logo, final Delegate delegate) {
        new AsyncTask<Void, Void, Bitmap>() {
            @Override
            protected Bitmap doInBackground(Void... params) {
                try {
                    BitMatrix matrix = new MultiFormatWriter().encode(content, BarcodeFormat.QR_CODE, size, size, HINTS);
                    int[] pixels = new int[size * size];
                    for (int y = 0; y < size; y++) {
                        for (int x = 0; x < size; x++) {
                            if (matrix.get(x, y)) {
                                pixels[y * size + x] = foregroundColor;
                            } else {
                                pixels[y * size + x] = backgroundColor;
                            }
                        }
                    }
                    Bitmap bitmap = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888);
                    bitmap.setPixels(pixels, 0, size, 0, 0, size, size);
                    return addLogoToQRCode(bitmap, logo);
                } catch (Exception e) {
                    return null;
                }
            }

            @Override
            protected void onPostExecute(Bitmap bitmap) {
                if (delegate != null) {
                    if (bitmap != null) {
                        delegate.onEncodeQRCodeSuccess(bitmap);
                    } else {
                        delegate.onEncodeQRCodeFailure();
                    }
                }
            }
        }.execute();
    }

    /**
     * 添加logo到二维码图片上
     *
     * @param src
     * @param logo
     * @return
     */
    private static Bitmap addLogoToQRCode(Bitmap src, Bitmap logo) {
        if (src == null || logo == null) {
            return src;
        }

        int srcWidth = src.getWidth();
        int srcHeight = src.getHeight();
        int logoWidth = logo.getWidth();
        int logoHeight = logo.getHeight();

        float scaleFactor = srcWidth * 1.0f / 5 / logoWidth;
        Bitmap bitmap = Bitmap.createBitmap(srcWidth, srcHeight, Bitmap.Config.ARGB_8888);
        try {
            Canvas canvas = new Canvas(bitmap);
            canvas.drawBitmap(src, 0, 0, null);
            canvas.scale(scaleFactor, scaleFactor, srcWidth / 2, srcHeight / 2);
            canvas.drawBitmap(logo, (srcWidth - logoWidth) / 2, (srcHeight - logoHeight) / 2, null);
            canvas.save(Canvas.ALL_SAVE_FLAG);
            canvas.restore();
        } catch (Exception e) {
            bitmap = null;
        }
        return bitmap;
    }

    public interface Delegate {
        /**
         * 创建二维码图片成功
         *
         * @param bitmap
         */
        void onEncodeQRCodeSuccess(Bitmap bitmap);

        /**
         * 创建二维码图片失败
         */
        void onEncodeQRCodeFailure();
    }
}