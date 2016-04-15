package cn.bingoogolapple.qrcode.zxing;

import android.graphics.Bitmap;
import android.graphics.Color;
import android.os.AsyncTask;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.EncodeHintType;
import com.google.zxing.MultiFormatWriter;
import com.google.zxing.common.BitMatrix;

import java.util.EnumMap;
import java.util.Map;

/**
 * 作者:王浩 邮件:bingoogolapple@gmail.com
 * 创建时间:16/4/8 下午11:22
 * 描述:创建二维码图片
 */
public class QRCodeEncoder {
    public static final Map<EncodeHintType, Object> HINTS = new EnumMap<EncodeHintType, Object>(EncodeHintType.class);

    static {
        HINTS.put(EncodeHintType.CHARACTER_SET, "utf-8");
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
     * @param size     图片宽高，单位为px
     * @param color    二维码图片的颜色
     * @param delegate 创建二维码图片的代理
     */
    public static void encodeQRCode(final String content, final int size, final int color, final Delegate delegate) {
        new AsyncTask<Void, Void, Bitmap>() {
            @Override
            protected Bitmap doInBackground(Void... params) {
                try {
                    BitMatrix matrix = new MultiFormatWriter().encode(content, BarcodeFormat.QR_CODE, size, size, HINTS);
                    int[] pixels = new int[size * size];
                    for (int y = 0; y < size; y++) {
                        for (int x = 0; x < size; x++) {
                            if (matrix.get(x, y)) {
                                pixels[y * size + x] = color;
                            }
                        }
                    }
                    Bitmap bitmap = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888);
                    bitmap.setPixels(pixels, 0, size, 0, 0, size, size);
                    return bitmap;
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