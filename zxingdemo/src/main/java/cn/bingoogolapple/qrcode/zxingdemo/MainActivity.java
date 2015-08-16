package cn.bingoogolapple.qrcode.zxingdemo;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.View;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        setSupportActionBar((Toolbar) findViewById(R.id.toolbar));
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
