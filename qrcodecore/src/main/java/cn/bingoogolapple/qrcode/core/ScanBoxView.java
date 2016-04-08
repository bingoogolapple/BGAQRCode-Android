package cn.bingoogolapple.qrcode.core;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.Rect;
import android.view.View;

public class ScanBoxView extends View {
    private static final int ANIMATION_DELAY = 1;
    private int SPEEN_DISTANCE = 3;

    private Rect mFramingRect;
    private int mScanLineTop;
    private Paint mPaint;
    private int mMaskColor;
    private int mCornerColor;
    private int mCornerLength;
    private int mCornerSize;
    private int mRectWidth;
    private int mTopOffset;
    private int mScanLineSize;
    private int mScanLineColor;

    public ScanBoxView(Context context) {
        super(context);
        mPaint = new Paint();
        mPaint.setAntiAlias(true);
        mPaint.setStyle(Paint.Style.FILL);
        mMaskColor = Color.parseColor("#33ffffff");
        mCornerColor = Color.WHITE;
        mCornerLength = DisplayUtils.dp2px(context, 20);
        mCornerSize = DisplayUtils.dp2px(context, 2);
        mScanLineSize = DisplayUtils.dp2px(context, 1);
        mScanLineColor = Color.WHITE;
        mTopOffset = DisplayUtils.dp2px(context, 80);
        mRectWidth = DisplayUtils.dp2px(context, 200);
    }

    public void setTopOffset(int topOffset) {
        mTopOffset = topOffset;
    }

    public void setMaskColor(int maskColor) {
        mMaskColor = maskColor;
    }

    public void setCornerColor(int cornerColor) {
        mCornerColor = cornerColor;
    }

    public void setCornerLength(int cornerLength) {
        mCornerLength = cornerLength;
    }

    public void setCornerSize(int cornerSize) {
        mCornerSize = cornerSize;
    }

    public void setScanLineSize(int scanLineSize) {
        mScanLineSize = scanLineSize;
    }

    public void setScanLineColor(int scanLineColor) {
        mScanLineColor = scanLineColor;
    }

    public void setRectWidth(int rectWidth) {
        mRectWidth = rectWidth;
    }

    public int getMaskColor() {
        return mMaskColor;
    }

    public int getCornerColor() {
        return mCornerColor;
    }

    public int getCornerLength() {
        return mCornerLength;
    }

    public int getCornerSize() {
        return mCornerSize;
    }

    public int getRectWidth() {
        return mRectWidth;
    }

    public int getTopOffset() {
        return mTopOffset;
    }

    public int getScanLineSize() {
        return mScanLineSize;
    }

    public int getScanLineColor() {
        return mScanLineColor;
    }

    @Override
    public void onDraw(Canvas canvas) {
        if (mFramingRect == null) {
            return;
        }

        // 获取屏幕的宽和高
        int width = canvas.getWidth();
        int height = canvas.getHeight();

        mPaint.setColor(mMaskColor);
        canvas.drawRect(0, 0, width, mFramingRect.top, mPaint);
        canvas.drawRect(0, mFramingRect.top, mFramingRect.left, mFramingRect.bottom + 1, mPaint);
        canvas.drawRect(mFramingRect.right + 1, mFramingRect.top, width, mFramingRect.bottom + 1, mPaint);
        canvas.drawRect(0, mFramingRect.bottom + 1, width, height, mPaint);


        mPaint.setColor(mCornerColor);
        canvas.drawRect(mFramingRect.left - mCornerSize + 2, mFramingRect.top - mCornerSize + 2, mFramingRect.left + mCornerLength - mCornerSize + 2, mFramingRect.top + 2, mPaint);
        canvas.drawRect(mFramingRect.left - mCornerSize + 2, mFramingRect.top - mCornerSize + 2, mFramingRect.left + 2, mFramingRect.top + mCornerLength - mCornerSize + 2, mPaint);
        canvas.drawRect(mFramingRect.right - mCornerLength + mCornerSize - 2, mFramingRect.top - mCornerSize + 2, mFramingRect.right + mCornerSize - 2, mFramingRect.top + 2, mPaint);
        canvas.drawRect(mFramingRect.right - 2, mFramingRect.top - mCornerSize + 2, mFramingRect.right + mCornerSize - 2, mFramingRect.top + mCornerLength - mCornerSize + 2, mPaint);

        canvas.drawRect(mFramingRect.left - mCornerSize + 2, mFramingRect.bottom - 2, mFramingRect.left + mCornerLength - mCornerSize + 2, mFramingRect.bottom + mCornerSize - 2, mPaint);
        canvas.drawRect(mFramingRect.left - mCornerSize + 2, mFramingRect.bottom - mCornerLength + mCornerSize - 2, mFramingRect.left + 2, mFramingRect.bottom + mCornerSize - 2, mPaint);
        canvas.drawRect(mFramingRect.right - mCornerLength + mCornerSize - 2, mFramingRect.bottom - 2, mFramingRect.right + mCornerSize - 2, mFramingRect.bottom + mCornerSize - 2, mPaint);
        canvas.drawRect(mFramingRect.right - 2, mFramingRect.bottom - mCornerLength + mCornerSize - 2, mFramingRect.right + mCornerSize - 2, mFramingRect.bottom + mCornerSize - 2, mPaint);


        mPaint.setColor(mScanLineColor);
        canvas.drawRect(mFramingRect.left, mScanLineTop, mFramingRect.right, mScanLineTop + mScanLineSize, mPaint);
        mScanLineTop += SPEEN_DISTANCE;
        if (mScanLineTop >= mFramingRect.bottom || mScanLineTop <= mFramingRect.top) {
            SPEEN_DISTANCE = -SPEEN_DISTANCE;
        }

        postInvalidateDelayed(ANIMATION_DELAY, mFramingRect.left, mFramingRect.top, mFramingRect.right, mFramingRect.bottom);
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
        Point screenResolution = DisplayUtils.getScreenResolution(getContext());
        int leftOffset = (screenResolution.x - mRectWidth) / 2;
        mFramingRect = new Rect(leftOffset, mTopOffset, leftOffset + mRectWidth, mTopOffset + mRectWidth);
        mScanLineTop = mTopOffset;
    }

}