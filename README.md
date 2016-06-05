:running:BGAQRCode-Android:running:
============

## 目录
* [功能介绍](#功能介绍)
* [常见问题](#常见问题)
* [效果图与示例 apk](#效果图与示例-apk)
* [Gradle 依赖](#gradle-依赖)
* [布局文件](#布局文件)
* [自定义属性说明](#自定义属性说明)
* [接口说明](#接口说明)
* [关于我](#关于我)

## 功能介绍
根据[之前公司](http://www.iqegg.com)的产品需求，参考 [barcodescanner](https://github.com/dm77/barcodescanner) 改的，希望能帮助到有生成二维码、扫描二维码、识别图片二维码等需求的猿友。修改幅度较大，也就没准备针对 [barcodescanner](https://github.com/dm77/barcodescanner) 库提交PR。

* ZXing 生成可自定义颜色、带 logo 的二维码
* ZXing 扫描二维码
* ZXing 识别图库中的二维码图片
* 可以控制闪光灯，方便夜间使用
* 可以定制各式各样的扫描框
* ZBar 扫描二维码「扫描中文会有乱码，如果对中文有要求，请使用 ZXing」

## 常见问题
#### 1.宽高一定要填充除了状态栏以外的其余部分

```xml
android:layout_width="match_parent"
android:layout_height="match_parent"
```
#### 2.Gradle 依赖时提示找不到cn.bingoogolapple:bga-libraryname:「latestVersion」@aar

[![Maven Central](https://maven-badges.herokuapp.com/maven-central/cn.bingoogolapple/bga-qrcodecore/badge.svg)](https://maven-badges.herokuapp.com/maven-central/cn.bingoogolapple/bga-qrcodecore) 「latestVersion」指的是左边这个 maven-central 徽章后面的「数字」，请自行替换。***请不要再来问我「latestVersion」是什么了***

#### 3.ZBar 混淆规则

```java
-keep class net.sourceforge.zbar.** { *; }
-keep interface net.sourceforge.zbar.** { *; }
-dontwarn net.sourceforge.zbar.**
```

## 效果图与示例 apk

![Image of zbar](http://7xk9dj.com1.z0.glb.clouddn.com/qrcode/screenshots/zbar109.gif?imageView2/2/w/250)
![Image of zxingbarcode](http://7xk9dj.com1.z0.glb.clouddn.com/qrcode/screenshots/zxingbarcode109.gif?imageView2/2/w/250)
![Image of zxingdecode](http://7xk9dj.com1.z0.glb.clouddn.com/qrcode/screenshots/zxingdecode109.gif?imageView2/2/w/250)
![Image of zxingqrcode](http://7xk9dj.com1.z0.glb.clouddn.com/qrcode/screenshots/zxingqrcode109.gif?imageView2/2/w/250)
![Image of 小蛋空气净化器](http://7xk9dj.com1.z0.glb.clouddn.com/qrcode/screenshots/iqegg.gif?imageView2/2/w/250)

| [点击下载 ZXingDemo.apk](http://fir.im/ZXingDemo)或扫描下面的二维码安装 | [点击下载 ZBarDemo.apk](http://fir.im/ZBarDemo)或扫描下面的二维码安装 |
| :------------: | :------------: |
| ![ZXingDemo apk文件二维码](http://7xk9dj.com1.z0.glb.clouddn.com/qrcode/zxingdemoapk.png) | ![ZBarDemo apk文件二维码](http://7xk9dj.com1.z0.glb.clouddn.com/qrcode/zbardemoapk.png) |

## Gradle 依赖
[![Maven Central](https://maven-badges.herokuapp.com/maven-central/cn.bingoogolapple/bga-qrcodecore/badge.svg)](https://maven-badges.herokuapp.com/maven-central/cn.bingoogolapple/bga-qrcodecore) 「latestVersion」指的是左边这个 maven-central 徽章后面的「数字」，请自行替换。

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
## 布局文件
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

## 自定义属性说明

属性名 | 说明 | 默认值
:----------- | :----------- | :-----------
qrcv_topOffset         | 扫描框距离 toolbar 底部的距离        | 90dp
qrcv_cornerSize         | 扫描框边角线的宽度        | 3dp
qrcv_cornerLength         | 扫描框边角线的长度        | 20dp
qrcv_cornerColor         | 扫描框边角线的颜色        | @android:color/white
qrcv_rectWidth         | 扫描框的宽度        | 200dp
qrcv_barcodeRectHeight         | 条码扫样式描框的高度        | 140dp
qrcv_maskColor         | 除去扫描框，其余部分阴影颜色        | #33FFFFFF
qrcv_scanLineSize         | 扫描线的宽度        | 1dp
qrcv_scanLineColor         | 扫描线的颜色「扫描线和默认的扫描线图片的颜色」        | @android:color/white
qrcv_scanLineMargin         | 扫描线距离上下或者左右边框的间距        | 0dp
qrcv_isShowDefaultScanLineDrawable         | 是否显示默认的图片扫描线「设置该属性后 qrcv_scanLineSize 将失效，可以通过 qrcv_scanLineColor 设置扫描线的颜色，避免让你公司的UI单独给你出特定颜色的扫描线图片」        | false
qrcv_customScanLineDrawable         | 扫描线的图片资源「默认的扫描线图片样式不能满足你的需求时使用，设置该属性后 qrcv_isShowDefaultScanLineDrawable、qrcv_scanLineSize、qrcv_scanLineColor 将失效」        | null
qrcv_borderSize         | 扫描边框的宽度        | 1dp
qrcv_borderColor         | 扫描边框的颜色        | @android:color/white
qrcv_animTime         | 扫描线从顶部移动到底部的动画时间「单位为毫秒」        | 1000
qrcv_isCenterVertical         | 扫描框是否垂直居中，该属性为true时会忽略 qrcv_topOffset 属性        | false
qrcv_toolbarHeight         | Toolbar 的高度，通过该属性来修正由 Toolbar 导致扫描框在垂直方向上的偏差        | 0dp
qrcv_isBarcode         | 是否是扫条形码        | false
qrcv_tipText         | 提示文案        | null
qrcv_tipTextSize         | 提示文案字体大小        | 14sp
qrcv_tipTextColor         | 提示文案颜色        | @android:color/white
qrcv_isTipTextBelowRect         | 提示文案是否在扫描框的底部        | false
qrcv_tipTextMargin         | 提示文案与扫描框之间的间距        | 20dp
qrcv_isShowTipTextAsSingleLine         | 是否把提示文案作为单行显示        | false
qrcv_isShowTipBackground         | 是否显示提示文案的背景        | false
qrcv_tipBackgroundColor         | 提示文案的背景色        | #22000000
qrcv_isScanLineReverse         | 扫描线是否来回移动        | true
qrcv_isShowDefaultGridScanLineDrawable         | 是否显示默认的网格图片扫描线        | false
qrcv_customGridScanLineDrawable         | 扫描线的网格图片资源        | nulll

## 接口说明

>QRCodeView

```java
/**
 * 设置扫描二维码的代理
 *
 * @param delegate 扫描二维码的代理
 */
public void setDelegate(Delegate delegate)

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

#### 详细用法请查看[ZBarDemo](https://github.com/bingoogolapple/BGAQRCode-Android/tree/master/zbardemo):feet:

#### 详细用法请查看[ZXingDemo](https://github.com/bingoogolapple/BGAQRCode-Android/tree/master/zxingdemo):feet:

## 关于我

| 新浪微博 | 个人主页 | 邮箱 | BGA系列开源库QQ群 | 如果你觉得这个库确实对你有帮助，可以考虑赞助我一块钱买机械键盘来撸代码 |
| ------------ | ------------- | ------------ | ------------ | ------------ |
| <a href="http://weibo.com/bingoogol" target="_blank">bingoogolapple</a> | <a  href="http://www.bingoogolapple.cn" target="_blank">bingoogolapple.cn</a>  | <a href="mailto:bingoogolapple@gmail.com" target="_blank">bingoogolapple@gmail.com</a> | ![BGA_CODE_CLUB](http://7xk9dj.com1.z0.glb.clouddn.com/BGA_CODE_CLUB.png?imageView2/2/w/200) | ![BGA_AliPay](http://7xk9dj.com1.z0.glb.clouddn.com/BGAAliPay.JPG?imageView2/2/w/300) |
