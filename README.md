:running:BGAQRCode-Android:running:
============

根据[之前公司](http://www.iqegg.com)的产品需求，参考这个项目改的 [barcodescanner](https://github.com/dm77/barcodescanner)，希望能帮助到有生成二维码、扫描二维码、识别图片二维码等需求的猿友。修改幅度较大，也就没准备针对[barcodescanner](https://github.com/dm77/barcodescanner)库提交PR。

## 常见问题
> 1.ZBar混淆规则

```java
-keep class net.sourceforge.zbar.** { *; }
-keep interface net.sourceforge.zbar.** { *; }
-dontwarn net.sourceforge.zbar.**
```

主要功能：
* ZXing生成可自定义颜色、带logo的二维码
* ZXing扫描二维码
* ZXing识别图库中的二维码图片
* 可以控制闪光灯，方便夜间使用
* 可以定制各式各样的扫描框
* ZBar扫描二维码「扫描中文会有乱码，如果对中文有要求，请使用ZXing」

### 效果图与示例apk

| ZXingDemo | ZBarDemo | [之前公司的Android客户端扫描二维码添加设备](http://www.iqegg.com) |
| :------------: | :------------: | :------------: |
| ![Image of ZXingDemo](http://7xk9dj.com1.z0.glb.clouddn.com/qrcode/screenshots/zxing106.gif) | ![Image of ZBarDemo](http://7xk9dj.com1.z0.glb.clouddn.com/qrcode/screenshots/zbar106.gif) | ![Image of 小蛋空气净化器](http://7xk9dj.com1.z0.glb.clouddn.com/qrcode/screenshots/iqegg.gif) |

| [点击下载ZXingDemo.apk](http://fir.im/ZXingDemo)或扫描下面的二维码安装 | [点击下载ZBarDemo apk](http://fir.im/ZBarDemo)或扫描下面的二维码安装 |
| :------------: | :------------: |
| ![ZXingDemo apk文件二维码](http://7xk9dj.com1.z0.glb.clouddn.com/qrcode/zxingdemoapk.png) | ![ZBarDemo apk文件二维码](http://7xk9dj.com1.z0.glb.clouddn.com/qrcode/zbardemoapk.png) |

### Gradle依赖 [![Maven Central](https://maven-badges.herokuapp.com/maven-central/cn.bingoogolapple/bga-qrcodecore/badge.svg)](https://maven-badges.herokuapp.com/maven-central/cn.bingoogolapple/bga-qrcodecore) ***「latestVersion」指的是左边这个 maven-central 徽章后面的「数字」，请自行替换。不要再来问我「latestVersion」是什么了:angry:不过你问了我还是回回答你的:smile:***

>ZXing

```groovy
dependencies {
    compile 'com.google.zxing:core:3.2.1'
    compile 'cn.bingoogolapple:bga-qrcodecore:latestVersion@aar'
    compile 'cn.bingoogolapple:bga-zxing:latestVersion@aar'
}
```
>ZBar

```groovy
dependencies {
    compile 'cn.bingoogolapple:bga-qrcodecore:latestVersion@aar'
    compile 'cn.bingoogolapple:bga-zbar:latestVersion@aar'
}
```
### Layout
>ZXing

```xml
<cn.bingoogolapple.qrcode.zxing.ZXingView
    android:id="@+id/zxingview"
    style="@style/MatchMatch"
    app:qrcv_animTime="1000"
    app:qrcv_borderColor="@android:color/white"
    app:qrcv_borderSize="1dp"
    app:qrcv_cornerColor="@color/colorPrimaryDark"
    app:qrcv_cornerLength="20dp"
    app:qrcv_cornerSize="3dp"
    app:qrcv_maskColor="#33FFFFFF"
    app:qrcv_rectWidth="200dp"
    app:qrcv_scanLineColor="@color/colorPrimaryDark"
    app:qrcv_scanLineSize="1dp"
    app:qrcv_topOffset="90dp" />
```
>ZBar

```xml
<cn.bingoogolapple.qrcode.zbar.ZBarView
    android:id="@+id/zbarview"
    style="@style/MatchMatch"
    app:qrcv_animTime="1000"
    app:qrcv_borderColor="@android:color/white"
    app:qrcv_borderSize="1dp"
    app:qrcv_cornerColor="@color/colorPrimaryDark"
    app:qrcv_cornerLength="20dp"
    app:qrcv_cornerSize="3dp"
    app:qrcv_isShowDefaultScanLineDrawable="true"
    app:qrcv_maskColor="#33FFFFFF"
    app:qrcv_rectWidth="200dp"
    app:qrcv_scanLineColor="@color/colorPrimaryDark"
    app:qrcv_topOffset="90dp" />
```

### 自定义属性说明

属性名 | 说明 | 默认值
:----------- | :----------- | :-----------
qrcv_topOffset         | 扫描框距离扫描视图顶部的距离        | 90dp
qrcv_cornerSize         | 扫描框边角线的宽度        | 3dp
qrcv_cornerLength         | 扫描框边角线的长度        | 20dp
qrcv_cornerColor         | 扫描框边角线的颜色        | @android:color/white
qrcv_rectWidth         | 扫描框的宽度        | 200dp
qrcv_maskColor         | 除去扫描框，其余部分阴影颜色        | #33FFFFFF
qrcv_scanLineSize         | 扫描线的宽度        | 1dp
qrcv_scanLineColor         | 扫描线的颜色「扫描线和默认的扫描线图片的颜色」        | @android:color/white
qrcv_scanLineHorizontalMargin         | 扫描线距离左右边框的间距        | 0dp
qrcv_isShowDefaultScanLineDrawable         | 是否显示默认的图片扫描线「设置该属性后 qrcv_scanLineSize 将失效，可以通过 qrcv_scanLineColor 设置扫描线的颜色，避免让你公司的UI单独给你出特定颜色的扫描线图片」        | false
qrcv_customScanLineDrawable         | 扫描线的图片资源「默认的扫描线图片样式不能满足你的需求时使用，设置该属性后 qrcv_isShowDefaultScanLineDrawable、qrcv_scanLineSize、qrcv_scanLineColor 将失效」        | null
qrcv_borderSize         | 扫描边框的宽度        | 1dp
qrcv_borderColor         | 扫描边框的颜色        | @android:color/white
qrcv_animTime         | 扫描线从顶部移动到底部的动画时间「单位为毫秒」        | 1000

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

>QRCodeView.Delegate   扫描二维码的代理

```java
/**
 * 处理扫描结果
 *
 * @param result
 */
void onScanQRCodeSuccess(String result)

/**
 * 处理打开相机出错
 */
void onScanQRCodeOpenCameraError()
```

>QRCodeDecoder  解析二维码图片

```java
/**
 * 解析二维码图片
 *
 * @param bitmap   要解析的二维码图片
 * @param delegate 解析二位码图片的代理
 */
public static void decodeQRCode(Bitmap bitmap, Delegate delegate)
```

>QRCodeDecoder.Delegate  解析二位码图片的代理

```java
/**
 * 解析二维码成功
 *
 * @param result 从二维码中解析的文本，如果该方法有被调用，result不会为空
 */
void onDecodeQRCodeSuccess(String result)

/**
 * 解析二维码失败
 */
void onDecodeQRCodeFailure()
```

>QRCodeEncoder  创建二维码图片

```java
/**
 * 创建黑色的二维码图片
 *
 * @param content
 * @param size     图片宽高，单位为px
 * @param delegate 创建二维码图片的代理
 */
public static void encodeQRCode(String content, int size, Delegate delegate)

/**
 * 创建指定颜色的二维码图片
 *
 * @param content
 * @param size     图片宽高，单位为px
 * @param color    二维码图片的颜色
 * @param delegate 创建二维码图片的代理
 */
public static void encodeQRCode(String content, int size, int color, Delegate delegate)

/**
 * 创建指定颜色的、带logo的二维码图片
 *
 * @param content
 * @param size     图片宽高，单位为px
 * @param color    二维码图片的颜色
 * @param logo     二维码图片的logo
 * @param delegate 创建二维码图片的代理
 */
public static void encodeQRCode(final String content, final int size, final int color, final Bitmap logo, final Delegate delegate)
```

>QRCodeEncoder.Delegate   创建二维码图片的代理

```java
/**
 * 创建二维码图片成功
 *
 * @param bitmap
 */
void onEncodeQRCodeSuccess(Bitmap bitmap)

/**
 * 创建二维码图片失败
 */
void onEncodeQRCodeFailure()
```

### 详细用法请查看[ZBarDemo](https://github.com/bingoogolapple/BGAQRCode-Android/tree/master/zbardemo):feet:

### 详细用法请查看[ZXingDemo](https://github.com/bingoogolapple/BGAQRCode-Android/tree/master/zxingdemo):feet:

### 关于我

| 新浪微博 | 个人主页 | 邮箱 | BGA系列开源库QQ群 | 如果你觉得这个库确实对你有帮助，可以考虑赞助我一块钱买机械键盘来撸代码 |
| ------------ | ------------- | ------------ | ------------ | ------------ |
| <a href="http://weibo.com/bingoogol" target="_blank">bingoogolapple</a> | <a  href="http://www.bingoogolapple.cn" target="_blank">bingoogolapple.cn</a>  | <a href="mailto:bingoogolapple@gmail.com" target="_blank">bingoogolapple@gmail.com</a> | ![BGA_CODE_CLUB](http://7xk9dj.com1.z0.glb.clouddn.com/BGA_CODE_CLUB.png?imageView2/2/w/200) | ![BGA_AliPay](http://7xk9dj.com1.z0.glb.clouddn.com/BGAAliPay.JPG?imageView2/2/w/300) |
