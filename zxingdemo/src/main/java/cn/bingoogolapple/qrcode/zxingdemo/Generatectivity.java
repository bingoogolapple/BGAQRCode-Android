package cn.bingoogolapple.qrcode.zxingdemo;

import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.ImageView;
import android.widget.Toast;

import cn.bingoogolapple.qrcode.core.DisplayUtils;
import cn.bingoogolapple.qrcode.zxing.QRCodeDecoder;
import cn.bingoogolapple.qrcode.zxing.QRCodeEncoder;

public class Generatectivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_generate);
        setSupportActionBar((Toolbar) findViewById(R.id.toolbar));

        createChineseQRCode();
        createEnglishQRCode();
    }

    private void createChineseQRCode() {
        QRCodeEncoder.encodeQRCode("王浩", DisplayUtils.dp2px(Generatectivity.this, 150), new QRCodeEncoder.Delegate() {
            @Override
            public void onEncodeQRCodeSuccess(Bitmap bitmap) {
                ((ImageView) findViewById(R.id.iv_chinese_qrcode)).setImageBitmap(bitmap);
            }

            @Override
            public void onEncodeQRCodeFailure() {
                Toast.makeText(Generatectivity.this, "生成中文二维码失败", Toast.LENGTH_SHORT).show();
            }
        });
    }

    private void createEnglishQRCode() {
        QRCodeEncoder.encodeQRCode("bingoogolapple", DisplayUtils.dp2px(Generatectivity.this, 150), Color.parseColor("#B2DB4D"), new QRCodeEncoder.Delegate() {
            @Override
            public void onEncodeQRCodeSuccess(Bitmap bitmap) {
                ((ImageView) findViewById(R.id.iv_english_qrcode)).setImageBitmap(bitmap);
            }

            @Override
            public void onEncodeQRCodeFailure() {
                Toast.makeText(Generatectivity.this, "生成英文二维码失败", Toast.LENGTH_SHORT).show();
            }
        });
    }

    public void onClick(View v) {
        Drawable drawable = null;
        if (v.getId() == R.id.decode_chinese_qrcode) {
            drawable = getResources().getDrawable(R.mipmap.test_chinese);
        } else if (v.getId() == R.id.decode_english_qrcode) {
            drawable = getResources().getDrawable(R.mipmap.test_english);
        } else if (v.getId() == R.id.decode_isbn) {
            drawable = getResources().getDrawable(R.mipmap.test_isbn);
        }

        if (drawable != null) {
            Bitmap bitmap = ((BitmapDrawable) drawable).getBitmap();

            QRCodeDecoder.decodeQRCode(bitmap, new QRCodeDecoder.Delegate() {
                @Override
                public void onDecodeQRCodeSuccess(String result) {
                    Toast.makeText(Generatectivity.this, result, Toast.LENGTH_SHORT).show();
                }

                @Override
                public void onDecodeQRCodeFailure() {
                    Toast.makeText(Generatectivity.this, "解码二维码失败", Toast.LENGTH_SHORT).show();
                }
            });
        }
    }
}