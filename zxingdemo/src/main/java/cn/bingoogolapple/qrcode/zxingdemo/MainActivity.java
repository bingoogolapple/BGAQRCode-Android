package cn.bingoogolapple.qrcode.zxingdemo;

import android.os.Vibrator;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

import cn.bingoogolapple.qrcode.core.QRCodeView;
import cn.bingoogolapple.qrcode.zxing.ZXingView;


public class MainActivity extends ActionBarActivity  implements QRCodeView.ResultHandler {
    private ZXingView mZXingView;

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mZXingView = (ZXingView) findViewById(R.id.zxingview);
    }

    @Override
    protected void onStart() {
        super.onStart();
        mZXingView.setResultHandler(this);
        mZXingView.startCamera();
    }

    @Override
    protected void onStop() {
        mZXingView.stopCamera();
        super.onStop();
    }

    private void vibrate() {
        Vibrator vibrator = (Vibrator) getSystemService(VIBRATOR_SERVICE);
        vibrator.vibrate(200);
    }

    @Override
    public void handleResult(String result) {
        Log.i("bingo", "result:" + result);
        vibrate();
        mZXingView.startSpotDelay();
    }

    @Override
    public void handleCameraError() {
        Log.e("bingo", "打开相机出错");
    }

    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.start_spot:
                mZXingView.startSpotDelay();
                break;
            case R.id.stop_spot:
                mZXingView.stopSpot();
                break;
            case R.id.show_rect:
                mZXingView.showScanRect();
                break;
            case R.id.hidden_rect:
                mZXingView.hiddenScanRect();
                break;
        }
    }
}