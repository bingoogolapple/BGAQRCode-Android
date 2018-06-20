package cn.bingoogolapple.qrcode.zxingdemo;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Vibrator;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.View;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.DecodeHintType;

import java.util.ArrayList;
import java.util.EnumMap;
import java.util.List;
import java.util.Map;

import cn.bingoogolapple.photopicker.activity.BGAPhotoPickerActivity;
import cn.bingoogolapple.qrcode.core.BarcodeType;
import cn.bingoogolapple.qrcode.core.QRCodeView;
import cn.bingoogolapple.qrcode.zxing.ZXingView;

public class TestScanActivity extends AppCompatActivity implements QRCodeView.Delegate {
    private static final String TAG = TestScanActivity.class.getSimpleName();
    private static final int REQUEST_CODE_CHOOSE_QRCODE_FROM_GALLERY = 666;

    private ZXingView mZXingView;

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_test_scan);
        setSupportActionBar((Toolbar) findViewById(R.id.toolbar));

        mZXingView = findViewById(R.id.zxingview);
        mZXingView.setDelegate(this);
    }

    @Override
    protected void onStart() {
        super.onStart();

        mZXingView.startCamera(); // 打开后置摄像头开始预览，但是并未开始识别
//        mZXingView.startCamera(Camera.CameraInfo.CAMERA_FACING_FRONT); // 打开前置摄像头开始预览，但是并未开始识别

        mZXingView.startSpotAndShowRect(); // 显示扫描框，并且延迟0.5秒后开始识别
    }

    @Override
    protected void onStop() {
        mZXingView.stopCamera(); // 关闭摄像头预览，并且隐藏扫描框
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        mZXingView.onDestroy(); // 销毁二维码扫描控件
        super.onDestroy();
    }

    private void vibrate() {
        Vibrator vibrator = (Vibrator) getSystemService(VIBRATOR_SERVICE);
        vibrator.vibrate(200);
    }

    @Override
    public void onScanQRCodeSuccess(String result) {
        Log.i(TAG, "result:" + result);
        setTitle("扫描结果为：" + result);
        vibrate();

        mZXingView.startSpot(); // 延迟0.5秒后开始识别
    }

    @Override
    public void onScanQRCodeOpenCameraError() {
        Log.e(TAG, "打开相机出错");
    }

    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.start_preview:
                mZXingView.startCamera(); // 打开后置摄像头开始预览，但是并未开始识别
                break;
            case R.id.stop_preview:
                mZXingView.stopCamera(); // 关闭摄像头预览，并且隐藏扫描框
                break;
            case R.id.start_spot:
                mZXingView.startSpot(); // 延迟0.5秒后开始识别
                break;
            case R.id.stop_spot:
                mZXingView.stopSpot(); // 停止识别
                break;
            case R.id.start_spot_showrect:
                mZXingView.startSpotAndShowRect(); // 显示扫描框，并且延迟0.5秒后开始识别
                break;
            case R.id.stop_spot_hiddenrect:
                mZXingView.stopSpotAndHiddenRect(); // 停止识别，并且隐藏扫描框
                break;
            case R.id.show_scan_rect:
                mZXingView.showScanRect(); // 显示扫描框
                break;
            case R.id.hidden_scan_rect:
                mZXingView.hiddenScanRect(); // 隐藏扫描框
                break;
            case R.id.decode_scan_box_area:
                mZXingView.getScanBoxView().setOnlyDecodeScanBoxArea(true); // 仅识别扫描框中的码
                break;
            case R.id.decode_full_screen_area:
                mZXingView.getScanBoxView().setOnlyDecodeScanBoxArea(false); // 识别整个屏幕中的码
                break;
            case R.id.open_flashlight:
                mZXingView.openFlashlight(); // 打开闪光灯
                break;
            case R.id.close_flashlight:
                mZXingView.closeFlashlight(); // 关闭闪光灯
                break;
            case R.id.scan_one_dimension:
                mZXingView.changeToScanBarcodeStyle(); // 切换成扫描条码样式
                mZXingView.setType(BarcodeType.ONE_DIMENSION, null); // 只识别一维条码
                mZXingView.startSpotAndShowRect(); // 显示扫描框，并且延迟0.5秒后开始识别
                break;
            case R.id.scan_two_dimension:
                mZXingView.changeToScanQRCodeStyle(); // 切换成扫描二维码样式
                mZXingView.setType(BarcodeType.TWO_DIMENSION, null); // 只识别二维条码
                mZXingView.startSpotAndShowRect(); // 显示扫描框，并且延迟0.5秒后开始识别
                break;
            case R.id.scan_qr_code:
                mZXingView.changeToScanQRCodeStyle(); // 切换成扫描二维码样式
                mZXingView.setType(BarcodeType.ONLY_QR_CODE, null); // 只识别 QR_CODE
                mZXingView.startSpotAndShowRect(); // 显示扫描框，并且延迟0.5秒后开始识别
                break;
            case R.id.scan_code128:
                mZXingView.changeToScanBarcodeStyle(); // 切换成扫描条码样式
                mZXingView.setType(BarcodeType.ONLY_CODE_128, null); // 只识别 CODE_128
                mZXingView.startSpotAndShowRect(); // 显示扫描框，并且延迟0.5秒后开始识别
                break;
            case R.id.scan_ean13:
                mZXingView.changeToScanBarcodeStyle(); // 切换成扫描条码样式
                mZXingView.setType(BarcodeType.ONLY_EAN_13, null); // 只识别 EAN_13
                mZXingView.startSpotAndShowRect(); // 显示扫描框，并且延迟0.5秒后开始识别
                break;
            case R.id.scan_high_frequency:
                mZXingView.changeToScanQRCodeStyle(); // 切换成扫描二维码样式
                mZXingView.setType(BarcodeType.HIGH_FREQUENCY, null); // 只识别高频率格式，包括 QR_CODE、EAN_13、CODE_128
                mZXingView.startSpotAndShowRect(); // 显示扫描框，并且延迟0.5秒后开始识别
                break;
            case R.id.scan_all:
                mZXingView.changeToScanQRCodeStyle(); // 切换成扫描二维码样式
                mZXingView.setType(BarcodeType.ALL, null); // 识别所有类型的码
                mZXingView.startSpotAndShowRect(); // 显示扫描框，并且延迟0.5秒后开始识别
                break;
            case R.id.scan_custom:
                mZXingView.changeToScanQRCodeStyle(); // 切换成扫描二维码样式

                Map<DecodeHintType, Object> hintMap = new EnumMap<>(DecodeHintType.class);
                List<BarcodeFormat> formatList = new ArrayList<>();
                formatList.add(BarcodeFormat.QR_CODE);
                formatList.add(BarcodeFormat.EAN_13);
                formatList.add(BarcodeFormat.CODE_128);
                hintMap.put(DecodeHintType.POSSIBLE_FORMATS, formatList); // 可能的编码格式
                hintMap.put(DecodeHintType.TRY_HARDER, Boolean.TRUE); // 花更多的时间用于寻找图上的编码，优化准确性，但不优化速度
                hintMap.put(DecodeHintType.CHARACTER_SET, "utf-8"); // 编码字符集
                mZXingView.setType(BarcodeType.CUSTOM, hintMap); // 自定义识别的类型

                mZXingView.startSpotAndShowRect(); // 显示扫描框，并且延迟0.5秒后开始识别
                break;
            case R.id.choose_qrcde_from_gallery:
                /*
                从相册选取二维码图片，这里为了方便演示，使用的是
                https://github.com/bingoogolapple/BGAPhotoPicker-Android
                这个库来从图库中选择二维码图片，这个库不是必须的，你也可以通过自己的方式从图库中选择图片
                 */
                Intent photoPickerIntent = new BGAPhotoPickerActivity.IntentBuilder(this)
                        .cameraFileDir(null)
                        .maxChooseCount(1)
                        .selectedPhotos(null)
                        .pauseOnScroll(false)
                        .build();
                startActivityForResult(photoPickerIntent, REQUEST_CODE_CHOOSE_QRCODE_FROM_GALLERY);
                break;
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        mZXingView.startSpotAndShowRect(); // 显示扫描框，并且延迟0.5秒后开始识别

        if (resultCode == Activity.RESULT_OK && requestCode == REQUEST_CODE_CHOOSE_QRCODE_FROM_GALLERY) {
            final String picturePath = BGAPhotoPickerActivity.getSelectedPhotos(data).get(0);
            // 本来就用到 QRCodeView 时可直接调 QRCodeView 的方法，走通用的回调
            mZXingView.decodeQRCode(picturePath);

            /*
            没有用到 QRCodeView 时可以调用 QRCodeDecoder 的 syncDecodeQRCode 方法

            这里为了偷懒，就没有处理匿名 AsyncTask 内部类导致 Activity 泄漏的问题
            请开发在使用时自行处理匿名内部类导致Activity内存泄漏的问题，处理方式可参考 https://github
            .com/GeniusVJR/LearningNotes/blob/master/Part1/Android/Android%E5%86%85%E5%AD%98%E6%B3%84%E6%BC%8F%E6%80%BB%E7%BB%93.md
             */
//            new AsyncTask<Void, Void, String>() {
//                @Override
//                protected String doInBackground(Void... params) {
//                    return QRCodeDecoder.syncDecodeQRCode(picturePath);
//                }
//
//                @Override
//                protected void onPostExecute(String result) {
//                    if (TextUtils.isEmpty(result)) {
//                        Toast.makeText(TestScanActivity.this, "未发现二维码", Toast.LENGTH_SHORT).show();
//                    } else {
//                        Toast.makeText(TestScanActivity.this, result, Toast.LENGTH_SHORT).show();
//                    }
//                }
//            }.execute();
        }
    }


}