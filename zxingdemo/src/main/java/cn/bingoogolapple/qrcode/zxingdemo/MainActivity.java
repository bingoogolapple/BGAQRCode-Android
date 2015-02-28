package cn.bingoogolapple.qrcode.zxingdemo;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.ActionBarActivity;
import android.view.View;

public class MainActivity extends ActionBarActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.scan_qrcode:
                startActivity(new Intent(this,ScanActivity.class));
                break;
            case R.id.generate_qrcode:
                startActivity(new Intent(this,Generatectivity.class));
                break;
        }
    }
}
