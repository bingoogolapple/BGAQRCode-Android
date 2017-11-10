package cn.bingoogolapple.qrcode.core;

import android.content.Context;
import android.content.res.Configuration;
import android.graphics.Point;
import android.hardware.Camera;

import java.util.Collection;
import java.util.List;
import java.util.regex.Pattern;

final class CameraConfigurationManager {
    private static final int TEN_DESIRED_ZOOM = 27;
    private static final Pattern COMMA_PATTERN = Pattern.compile(",");
    private final Context mContext;
    private Point mScreenResolution;
    private Point mCameraResolution;
    private Point mPreviewResolution;

    public CameraConfigurationManager(Context context) {
        mContext = context;
    }

    public void initFromCameraParameters(Camera camera) {
        Camera.Parameters parameters = camera.getParameters();

        if (CameraConfigurationManager.autoFocusAble(camera)) {
            parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_AUTO);
        }

        mScreenResolution = BGAQRCodeUtil.getScreenResolution(mContext);
        Point screenResolutionForCamera = new Point();
        screenResolutionForCamera.x = mScreenResolution.x;
        screenResolutionForCamera.y = mScreenResolution.y;

        // preview size is always something like 480*320, other 320*480
        int orientation = BGAQRCodeUtil.getScreenOrientation(mContext);

        if (orientation == Configuration.ORIENTATION_PORTRAIT) {
            screenResolutionForCamera.x = mScreenResolution.y;
            screenResolutionForCamera.y = mScreenResolution.x;
        }

        mPreviewResolution = getPreviewResolution(parameters, screenResolutionForCamera);

        if (orientation == Configuration.ORIENTATION_PORTRAIT) {
            mCameraResolution = new Point(mPreviewResolution.y, mPreviewResolution.x);
        } else {
            mCameraResolution = mPreviewResolution;
        }
    }

    public static boolean autoFocusAble(Camera camera) {
        List<String> supportedFocusModes = camera.getParameters().getSupportedFocusModes();
        String focusMode = findSettableValue(supportedFocusModes, Camera.Parameters.FOCUS_MODE_AUTO);
        return focusMode != null;
    }

    public Point getCameraResolution() {
        return mCameraResolution;
    }


    public void openFlashlight(Camera camera) {
        doSetTorch(camera, true);
    }

    public void closeFlashlight(Camera camera) {
        doSetTorch(camera, false);
    }

    private void doSetTorch(Camera camera, boolean newSetting) {
        Camera.Parameters parameters = camera.getParameters();
        String flashMode;
        /** 是否支持闪光灯 */
        if (newSetting) {
            flashMode = findSettableValue(parameters.getSupportedFlashModes(), Camera.Parameters.FLASH_MODE_TORCH, Camera.Parameters.FLASH_MODE_ON);
        } else {
            flashMode = findSettableValue(parameters.getSupportedFlashModes(), Camera.Parameters.FLASH_MODE_OFF);
        }
        if (flashMode != null) {
            parameters.setFlashMode(flashMode);
        }
        camera.setParameters(parameters);
    }

    private static String findSettableValue(Collection<String> supportedValues, String... desiredValues) {
        String result = null;
        if (supportedValues != null) {
            for (String desiredValue : desiredValues) {
                if (supportedValues.contains(desiredValue)) {
                    result = desiredValue;
                    break;
                }
            }
        }
        return result;
    }

    private static Point getPreviewResolution(Camera.Parameters parameters, Point screenResolution) {
        Point previewResolution =
                findBestPreviewSizeValue(parameters.getSupportedPreviewSizes(), screenResolution);
        if (previewResolution == null) {
            previewResolution = new Point((screenResolution.x >> 3) << 3, (screenResolution.y >> 3) << 3);
        }
        return previewResolution;
    }

    private static Point findBestPreviewSizeValue(List<Camera.Size> supportSizeList, Point screenResolution) {
        int bestX = 0;
        int bestY = 0;
        int diff = Integer.MAX_VALUE;
        for (Camera.Size previewSize : supportSizeList) {

            int newX = previewSize.width;
            int newY = previewSize.height;

            int newDiff = Math.abs(newX - screenResolution.x) + Math.abs(newY - screenResolution.y);
            if (newDiff == 0) {
                bestX = newX;
                bestY = newY;
                break;
            } else if (newDiff < diff) {
                bestX = newX;
                bestY = newY;
                diff = newDiff;
            }

        }

        if (bestX > 0 && bestY > 0) {
            return new Point(bestX, bestY);
        }
        return null;
    }

}