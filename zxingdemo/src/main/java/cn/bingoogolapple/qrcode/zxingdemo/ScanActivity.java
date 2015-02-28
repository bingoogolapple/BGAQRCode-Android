package cn.bingoogolapple.qrcode.zxingdemo;

import android.os.Vibrator;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

import cn.bingoogolapple.qrcode.core.QRCodeView;
import cn.bingoogolapple.qrcode.zxing.ZXingView;

public class ScanActivity extends ActionBarActivity implements QRCodeView.ResultHandler {
    private ZXingView mZXingView;

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_scan);
        mZXingView = (ZXingView) findViewById(R.id.zxingview);
        mZXingView.setResultHandler(this);
    }

    @Override
    protected void onStart() {
        super.onStart();
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
        Toast.makeText(this,result,Toast.LENGTH_SHORT).show();
        vibrate();
        mZXingView.startSpot();
    }

    @Override
    public void handleCameraError() {
        Log.e("bingo", "打开相机出错");
    }

    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.start_spot:
                mZXingView.startSpot();
                break;
            case R.id.stop_spot:
                mZXingView.stopSpot();
                break;
            case R.id.start_spot_showrect:
                mZXingView.startSpotAndShowRect();
                break;
            case R.id.stop_spot_hiddenrect:
                mZXingView.stopSpotAndHiddenRect();
                break;
            case R.id.show_rect:
                mZXingView.showScanRect();
                break;
            case R.id.hidden_rect:
                mZXingView.hiddenScanRect();
                break;
            case R.id.start_preview:
                mZXingView.startCamera();
                break;
            case R.id.stop_preview:
                mZXingView.stopCamera();
                break;
        }
    }
}