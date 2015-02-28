package cn.bingoogolapple.qrcode.zxing;

import android.graphics.Bitmap;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.MultiFormatWriter;
import com.google.zxing.WriterException;
import com.google.zxing.common.BitMatrix;

public class EncodingHandler {
    private static final int BLACK = 0xff000000;

    public static Bitmap createQRCode(String content, int size) throws WriterException {
        return EncodingHandler.createQRCode(content,size,BLACK);
    }

    public static Bitmap createQRCode(String content, int size, int color) throws WriterException {
        BitMatrix matrix = new MultiFormatWriter().encode(content, BarcodeFormat.QR_CODE, size, size);
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
    }
}