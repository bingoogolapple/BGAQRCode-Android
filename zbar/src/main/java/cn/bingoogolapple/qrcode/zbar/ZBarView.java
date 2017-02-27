package cn.bingoogolapple.qrcode.zbar;

import android.content.Context;
import android.util.AttributeSet;

import com.dtr.zbar.build.ZBarDecoder;
import cn.bingoogolapple.qrcode.core.QRCodeView;

public class ZBarView extends QRCodeView {




    public ZBarView(Context context, AttributeSet attributeSet) {
        this(context, attributeSet, 0);
    }

    public ZBarView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }


    @Override
    public String processData(byte[] data, int width, int height, boolean isRetry) {
        ZBarDecoder zBarDecoder = new ZBarDecoder();
        String result = zBarDecoder.decodeRaw(data, width, height);
        return result;
    }


}