package cn.bingoogolapple.qrcode.zxingdemo;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.text.TextUtils;
import android.view.View;
import android.widget.ImageView;
import android.widget.Toast;

import cn.bingoogolapple.qrcode.core.BGAQRCodeUtil;
import cn.bingoogolapple.qrcode.zxing.QRCodeDecoder;
import cn.bingoogolapple.qrcode.zxing.QRCodeEncoder;

public class TestGeneratectivity extends AppCompatActivity {
    private ImageView mChineseIv;
    private ImageView mEnglishIv;
    private ImageView mChineseLogoIv;
    private ImageView mEnglishLogoIv;
    private ImageView mBarcodeWithContentIv;
    private ImageView mBarcodeWithoutContentIv;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_test_generate);
        setSupportActionBar((Toolbar) findViewById(R.id.toolbar));

        initView();
        createQRCode();
    }

    private void initView() {
        mChineseIv = findViewById(R.id.iv_chinese);
        mChineseLogoIv = findViewById(R.id.iv_chinese_logo);
        mEnglishIv = findViewById(R.id.iv_english);
        mEnglishLogoIv = findViewById(R.id.iv_english_logo);
        mBarcodeWithContentIv = findViewById(R.id.iv_barcode_with_content);
        mBarcodeWithoutContentIv = findViewById(R.id.iv_barcode_without_content);
    }

    private void createQRCode() {
        createChineseQRCode();
        createEnglishQRCode();
        createChineseQRCodeWithLogo();
        createEnglishQRCodeWithLogo();

        createBarcodeWidthContent();
        createBarcodeWithoutContent();
    }

    private void createChineseQRCode() {
        /*
        这里为了偷懒，就没有处理匿名 AsyncTask 内部类导致 Activity 泄漏的问题
        请开发在使用时自行处理匿名内部类导致Activity内存泄漏的问题，处理方式可参考 https://github
        .com/GeniusVJR/LearningNotes/blob/master/Part1/Android/Android%E5%86%85%E5%AD%98%E6%B3%84%E6%BC%8F%E6%80%BB%E7%BB%93.md
         */
        new AsyncTask<Void, Void, Bitmap>() {
            @Override
            protected Bitmap doInBackground(Void... params) {
                return QRCodeEncoder.syncEncodeQRCode("王浩", BGAQRCodeUtil.dp2px(TestGeneratectivity.this, 150));
            }

            @Override
            protected void onPostExecute(Bitmap bitmap) {
                if (bitmap != null) {
                    mChineseIv.setImageBitmap(bitmap);
                } else {
                    Toast.makeText(TestGeneratectivity.this, "生成中文二维码失败", Toast.LENGTH_SHORT).show();
                }
            }
        }.execute();
    }

    private void createEnglishQRCode() {
        new AsyncTask<Void, Void, Bitmap>() {
            @Override
            protected Bitmap doInBackground(Void... params) {
                return QRCodeEncoder.syncEncodeQRCode("bingoogolapple", BGAQRCodeUtil.dp2px(TestGeneratectivity.this, 150), Color.parseColor("#ff0000"));
            }

            @Override
            protected void onPostExecute(Bitmap bitmap) {
                if (bitmap != null) {
                    mEnglishIv.setImageBitmap(bitmap);
                } else {
                    Toast.makeText(TestGeneratectivity.this, "生成英文二维码失败", Toast.LENGTH_SHORT).show();
                }
            }
        }.execute();
    }

    private void createChineseQRCodeWithLogo() {
        new AsyncTask<Void, Void, Bitmap>() {
            @Override
            protected Bitmap doInBackground(Void... params) {
                Bitmap logoBitmap = BitmapFactory.decodeResource(TestGeneratectivity.this.getResources(), R.mipmap.logo);
                return QRCodeEncoder.syncEncodeQRCode("王浩", BGAQRCodeUtil.dp2px(TestGeneratectivity.this, 150), Color.parseColor("#ff0000"), logoBitmap);
            }

            @Override
            protected void onPostExecute(Bitmap bitmap) {
                if (bitmap != null) {
                    mChineseLogoIv.setImageBitmap(bitmap);
                } else {
                    Toast.makeText(TestGeneratectivity.this, "生成带logo的中文二维码失败", Toast.LENGTH_SHORT).show();
                }
            }
        }.execute();
    }

    private void createEnglishQRCodeWithLogo() {
        new AsyncTask<Void, Void, Bitmap>() {
            @Override
            protected Bitmap doInBackground(Void... params) {
                Bitmap logoBitmap = BitmapFactory.decodeResource(TestGeneratectivity.this.getResources(), R.mipmap.logo);
                return QRCodeEncoder.syncEncodeQRCode("bingoogolapple", BGAQRCodeUtil.dp2px(TestGeneratectivity.this, 150), Color.BLACK, Color.WHITE,
                        logoBitmap);
            }

            @Override
            protected void onPostExecute(Bitmap bitmap) {
                if (bitmap != null) {
                    mEnglishLogoIv.setImageBitmap(bitmap);
                } else {
                    Toast.makeText(TestGeneratectivity.this, "生成带logo的英文二维码失败", Toast.LENGTH_SHORT).show();
                }
            }
        }.execute();
    }


    private void createBarcodeWidthContent() {
        new AsyncTask<Void, Void, Bitmap>() {
            @Override
            protected Bitmap doInBackground(Void... params) {
                int width = BGAQRCodeUtil.dp2px(TestGeneratectivity.this, 150);
                int height = BGAQRCodeUtil.dp2px(TestGeneratectivity.this, 70);
                int textSize = BGAQRCodeUtil.sp2px(TestGeneratectivity.this, 14);
                return QRCodeEncoder.syncEncodeBarcode("bga123", width, height, textSize);
            }

            @Override
            protected void onPostExecute(Bitmap bitmap) {
                if (bitmap != null) {
                    mBarcodeWithContentIv.setImageBitmap(bitmap);
                } else {
                    Toast.makeText(TestGeneratectivity.this, "生成条底部带文字形码失败", Toast.LENGTH_SHORT).show();
                }
            }
        }.execute();
    }

    private void createBarcodeWithoutContent() {
        new AsyncTask<Void, Void, Bitmap>() {
            @Override
            protected Bitmap doInBackground(Void... params) {
                int width = BGAQRCodeUtil.dp2px(TestGeneratectivity.this, 150);
                int height = BGAQRCodeUtil.dp2px(TestGeneratectivity.this, 70);
                return QRCodeEncoder.syncEncodeBarcode("bingoogolapple123", width, height, 0);
            }

            @Override
            protected void onPostExecute(Bitmap bitmap) {
                if (bitmap != null) {
                    mBarcodeWithoutContentIv.setImageBitmap(bitmap);
                } else {
                    Toast.makeText(TestGeneratectivity.this, "生成条底部不带文字形码失败", Toast.LENGTH_SHORT).show();
                }
            }
        }.execute();
    }

    public void decodeChinese(View v) {
        mChineseIv.setDrawingCacheEnabled(true);
        Bitmap bitmap = mChineseIv.getDrawingCache();
        decode(bitmap, "解析中文二维码失败");
    }

    public void decodeEnglish(View v) {
        mEnglishIv.setDrawingCacheEnabled(true);
        Bitmap bitmap = mEnglishIv.getDrawingCache();
        decode(bitmap, "解析英文二维码失败");
    }

    public void decodeChineseWithLogo(View v) {
        mChineseLogoIv.setDrawingCacheEnabled(true);
        Bitmap bitmap = mChineseLogoIv.getDrawingCache();
        decode(bitmap, "解析带logo的中文二维码失败");
    }

    public void decodeEnglishWithLogo(View v) {
        mEnglishLogoIv.setDrawingCacheEnabled(true);
        Bitmap bitmap = mEnglishLogoIv.getDrawingCache();
        decode(bitmap, "解析带logo的英文二维码失败");
    }

    public void decodeBarcodeWithContent(View v) {
        mBarcodeWithContentIv.setDrawingCacheEnabled(true);
        Bitmap bitmap = mBarcodeWithContentIv.getDrawingCache();
        decode(bitmap, "识别底部带文字的条形码失败");
    }

    public void decodeBarcodeWithoutContent(View v) {
        mBarcodeWithoutContentIv.setDrawingCacheEnabled(true);
        Bitmap bitmap = mBarcodeWithoutContentIv.getDrawingCache();
        decode(bitmap, "识别底部没带文字的条形码失败");
    }

    private void decode(final Bitmap bitmap, final String errorTip) {
        /*
        这里为了偷懒，就没有处理匿名 AsyncTask 内部类导致 Activity 泄漏的问题
        请开发在使用时自行处理匿名内部类导致Activity内存泄漏的问题，处理方式可参考 https://github
        .com/GeniusVJR/LearningNotes/blob/master/Part1/Android/Android%E5%86%85%E5%AD%98%E6%B3%84%E6%BC%8F%E6%80%BB%E7%BB%93.md
         */
        new AsyncTask<Void, Void, String>() {
            @Override
            protected String doInBackground(Void... params) {
                return QRCodeDecoder.syncDecodeQRCode(bitmap);
            }

            @Override
            protected void onPostExecute(String result) {
                if (TextUtils.isEmpty(result)) {
                    Toast.makeText(TestGeneratectivity.this, errorTip, Toast.LENGTH_SHORT).show();
                } else {
                    Toast.makeText(TestGeneratectivity.this, result, Toast.LENGTH_SHORT).show();
                }
            }
        }.execute();
    }
}