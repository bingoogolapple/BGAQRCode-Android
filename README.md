:running:QRCode-Android:running:
============

根据公司项目需求，参考这个项目改的 [barcodescanner](https://github.com/dm77/barcodescanner)

主要功能：ZXing生成二维码、ZXing扫描二维码、ZBar扫描二维码(扫描中文会有乱码)、可控制闪光灯和定制各式各样的扫描框

### 效果图
![Image of IqeggQRCodeDemo](http://7xk9dj.com1.z0.glb.clouddn.com/qrcode/screenshots/IqeggQRCodeDemo.gif)
![Image of ZXingDemo](http://7xk9dj.com1.z0.glb.clouddn.com/qrcode/screenshots/ZXingDemo.gif)
![Image of ZBarDemo](http://7xk9dj.com1.z0.glb.clouddn.com/qrcode/screenshots/ZBarDemo.gif)

### Gradle依赖

[ ![Download](https://api.bintray.com/packages/bingoogolapple/maven/qrcodecore/images/download.svg) ](https://bintray.com/bingoogolapple/maven/qrcodecore/_latestVersion)

>ZXing

```groovy
dependencies {
    compile 'com.google.zxing:core:3.1.0'
    compile 'cn.bingoogolapple:qrcodecore:latestVersion@aar'
    compile 'cn.bingoogolapple:zxing:latestVersion@aar'
}
```
>ZBar

```groovy
dependencies {
    compile 'cn.bingoogolapple:qrcodecore:latestVersion@aar'
    compile 'cn.bingoogolapple:zbar:latestVersion@aar'
}
```

### 自定义属性说明

属性名 | 说明 | 默认值
:----------- | :----------- | :-----------
qrcv_topOffset         | 扫描框距离扫描视图顶部的距离        | 80dp
qrcv_cornerSize         | 扫描框边角线的宽度        | 2dp
qrcv_cornerLength         | 扫描框边角线的长度        | 20dp
qrcv_cornerColor         | 扫描框边角线的颜色        | @android:color/white
qrcv_rectWidth         | 扫描框的宽度        | 200dp
qrcv_maskColor         | 除去扫描框，其余部分阴影颜色        | #33ffffff
qrcv_scanLineSize         | 扫描线的宽度        | 1dp
qrcv_scanLineColor         | 扫描线的颜色        | @android:color/white

### 接口说明

>QRCodeView

```java
/**
 * 显示扫描框
 */
public void showScanRect()

/**
 * 隐藏扫描框
 */
public void hiddenScanRect()

/**
 * 打开摄像头开始预览，但是并未开始识别
 */
public void startCamera()

/**
 * 关闭摄像头预览，并且隐藏扫描框
 */
public void stopCamera()

/**
 * 延迟1.5秒后开始识别
 */
public void startSpot()

/**
 * 延迟delay毫秒后开始识别
 *
 * @param delay
 */
public void startSpotDelay(int delay)

/**
 * 停止识别
 */
public void stopSpot()

/**
 * 停止识别，并且隐藏扫描框
 */
public void stopSpotAndHiddenRect()

/**
 * 显示扫描框，并且延迟1.5秒后开始识别
 */
public void startSpotAndShowRect()

/**
 * 打开闪光灯
 */
public void openFlashlight()

/**
 * 关闭散光灯
 */
public void closeFlashlight()
```

>ResultHandler

```java
/**
 * 处理扫描结果
 *
 * @param result
 */
void handleResult(String result)

/**
 * 处理打开相机出错
 */
void handleCameraError()
```

### 详细用法请查看[ZBarDemo](https://github.com/bingoogolapple/QRCode-Android/tree/master/zbardemo):feet:

### 详细用法请查看[ZXingDemo](https://github.com/bingoogolapple/QRCode-Android/tree/master/zxingdemo):feet:

>关于我<br/>
>微博：<a href="http://weibo.com/bingoogol" target="_blank">bingoogolapple</a>&nbsp;&nbsp;&nbsp;&nbsp;主页：<a  href="http://www.bingoogolapple.cn" target="_blank">bingoogolapple.cn</a>&nbsp;&nbsp;&nbsp;&nbsp;邮箱：<a href="mailto:bingoogolapple@gmail.com" target="_blank">bingoogolapple@gmail.com</a>
