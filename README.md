:running:BGAQRCode-Android:running: [ ![Download](https://api.bintray.com/packages/bingoogolapple/maven/bga-qrcode-core/images/download.svg) ](https://bintray.com/bingoogolapple/maven/bga-qrcode-core/_latestVersion)
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

- [x] 可定制各式各样的扫描框
- [x] 可定制全屏扫描或只识别扫描框区域内码
- [x] 可定制要识别的码的格式（详细用法查看 TestScanActivity 中的 onClick 方法）
- [x] 可以控制闪光灯，方便夜间使用
- [x] 可以设置用前置摄像头扫描
- [x] 可以二指缩放预览
- [x] 可以监听环境亮度，提示用户打开、关闭闪光灯
- [x] 识别到比较小的码时自动放大
***
- [x] ZXing 生成可自定义颜色、带 logo 的二维码
- [x] ZXing 生成一维码
- [x] ZXing 扫描条码、二维码
- [x] ZXing 识别图库中的条码、二维码图片
***
- [x] ZBar 扫描条码、二维码「已解决中文乱码问题」
- [x] ZBar 识别图库中的条码、二维码图片

## 常见问题
#### 1.部分手机无法扫描出结果，扫描预览界面二维码被压缩

使用的时候将 Toolbar 或者其他 View 盖在 ZBarView 或者 ZXingView 的上面，让 ZBarView 或者 ZXingView 填充屏幕宽高。[ZXing 布局文件参考](https://github.com/bingoogolapple/BGAQRCode-Android/blob/master/zxingdemo/src/main/res/layout/activity_test_scan.xml) [ZBar 布局文件参考](https://github.com/bingoogolapple/BGAQRCode-Android/blob/master/zbardemo/src/main/res/layout/activity_test_scan.xml)

#### 2.出现黑屏

在自己项目里集成时记得在 onDestroy 方法中调用 mQRCodeView.onDestroy()，在 onStop 方法中调用 mQRCodeView.stopCamera()，否则会出现黑屏。如果没执行前面提到的这两个方法出现黑屏的话，那你就只能加上前面提到的两个方法后，重启手机后重新运行了

#### 3.Gradle 依赖时提示找不到cn.bingoogolapple:bga-libraryname:「latestVersion」

[ ![Download](https://api.bintray.com/packages/bingoogolapple/maven/bga-qrcode-core/images/download.svg) ](https://bintray.com/bingoogolapple/maven/bga-qrcode-core/_latestVersion)「latestVersion」指的是左边这个 Download 徽章后面的「数字」，请自行替换。***请不要再来问我「latestVersion」是什么了***

## 效果图与示例 apk

![zbar109](https://cloud.githubusercontent.com/assets/8949716/17475203/5d788730-5d8c-11e6-836a-61e885e05453.gif)
![zxingbarcode109](https://cloud.githubusercontent.com/assets/8949716/17475222/76339bd4-5d8c-11e6-934f-96db6917f69b.gif)
![zxingdecode109](https://cloud.githubusercontent.com/assets/8949716/17475235/8c03b2be-5d8c-11e6-931d-a50942a8ab75.gif)
![zxingqrcode109](https://cloud.githubusercontent.com/assets/8949716/17475249/a551cc06-5d8c-11e6-85dc-4e2e07051cae.gif)
![iqegg](https://cloud.githubusercontent.com/assets/8949716/17475267/bd9c0a60-5d8c-11e6-8487-c732306debe2.gif)

| [点击下载 ZXingDemo.apk](http://fir.im/ZXingDemo)或扫描下面的二维码安装 | [点击下载 ZBarDemo.apk](http://fir.im/ZBarDemo)或扫描下面的二维码安装 |
| :------------: | :------------: |
| ![ZXingDemo apk文件二维码](http://bgashare.bingoogolapple.cn/qrcode/zxingdemoapk.png) | ![ZBarDemo apk文件二维码](http://bgashare.bingoogolapple.cn/qrcode/zbardemoapk.png) |

## Gradle 依赖
[ ![Download](https://api.bintray.com/packages/bingoogolapple/maven/bga-qrcode-core/images/download.svg) ](https://bintray.com/bingoogolapple/maven/bga-qrcode-core/_latestVersion)「latestVersion」指的是左边这个 Download 徽章后面的「数字」，请自行替换。***请不要再来问我「latestVersion」是什么了***

>ZXing

```groovy
dependencies {
    implementation 'cn.bingoogolapple:bga-qrcode-zxing:latestVersion'
}
```
>ZBar

```groovy
dependencies {
    implementation 'cn.bingoogolapple:bga-qrcode-zbar:latestVersion'
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
qrcv_cornerDisplayType         | 扫描框边角线显示位置(相对于边框)，默认值为中间        | center
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
qrcv_isCenterVertical（已废弃，如果要垂直居中用 qrcv_verticalBias="0.5"来代替）         | 扫描框是否垂直居中，该属性为true时会忽略 qrcv_topOffset 属性        | false
qrcv_verticalBias         | 扫描框中心点在屏幕垂直方向的比例，当设置此值时，会忽略 qrcv_topOffset 属性        | -1
qrcv_toolbarHeight         | Toolbar 的高度，通过该属性来修正由 Toolbar 导致扫描框在垂直方向上的偏差        | 0dp
qrcv_isBarcode         | 扫描框的样式是否为扫条形码样式        | false
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
qrcv_isOnlyDecodeScanBoxArea         | 是否只识别扫描框中的码        | false
qrcv_isShowLocationPoint         | 是否显示定位点        | false
qrcv_isAutoZoom         | 码太小时是否自动缩放        | false

## 接口说明

>QRCodeView

```java

/**
 * ZBarView 设置识别的格式。详细用法请看 zbardemo 的 TestScanActivity 中的 onClick 方法
 *
 * @param barcodeType 识别的格式
 * @param formatList  barcodeType 为 BarcdeType.CUSTOM 时，必须指定该值
 */
public void setType(BarcodeType barcodeType, List<BarcodeFormat> formatList)

/**
 * ZXingView 设置识别的格式。详细用法请看 zxingdemo TestScanActivity 中的 onClick 方法
 *
 * @param barcodeType 识别的格式
 * @param hintMap     barcodeType 为 BarcodeType.CUSTOM 时，必须指定该值
 */
public void setType(BarcodeType barcodeType, Map<DecodeHintType, Object> hintMap)

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
 * 打开后置摄像头开始预览，但是并未开始识别
 */
public void startCamera()

/**
 * 打开指定摄像头开始预览，但是并未开始识别
 *
 * @param cameraFacing  Camera.CameraInfo.CAMERA_FACING_BACK or Camera.CameraInfo.CAMERA_FACING_FRONT
 */
public void startCamera(int cameraFacing)

/**
 * 关闭摄像头预览，并且隐藏扫描框
 */
public void stopCamera()

/**
 * 开始识别
 */
public void startSpot()

/**
 * 停止识别
 */
public void stopSpot()

/**
 * 停止识别，并且隐藏扫描框
 */
public void stopSpotAndHiddenRect()

/**
 * 显示扫描框，并开始识别
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

/**
 * 解析本地图片二维码。返回二维码图片里的内容 或 null
 *
 * @param picturePath 要解析的二维码图片本地路径
 */
public void decodeQRCode(String picturePath)

/**
 * 解析 Bitmap 二维码。返回二维码图片里的内容 或 null
 *
 * @param bitmap 要解析的二维码图片
 */
public void decodeQRCode(Bitmap bitmap)
```

>QRCodeView.Delegate   扫描二维码的代理

```java
/**
 * 处理扫描结果
 *
 * @param result 摄像头扫码时只要回调了该方法 result 就一定有值，不会为 null。解析本地图片或 Bitmap 时 result 可能为 null
 */
void onScanQRCodeSuccess(String result)

/**
 * 摄像头环境亮度发生变化
 *
 * @param isDark 是否变暗
 */
void onCameraAmbientBrightnessChanged(boolean isDark);

/**
 * 处理打开相机出错
 */
void onScanQRCodeOpenCameraError()
```

>QRCodeEncoder  创建二维码图片。几个重载方法都是耗时操作，请在子线程中调用。

```java
/**
 * 同步创建黑色前景色、白色背景色的二维码图片。该方法是耗时操作，请在子线程中调用。
 *
 * @param content 要生成的二维码图片内容
 * @param size    图片宽高，单位为px
 */
public static Bitmap syncEncodeQRCode(String content, int size)

/**
 * 同步创建指定前景色、白色背景色的二维码图片。该方法是耗时操作，请在子线程中调用。
 *
 * @param content         要生成的二维码图片内容
 * @param size            图片宽高，单位为px
 * @param foregroundColor 二维码图片的前景色
 */
public static Bitmap syncEncodeQRCode(String content, int size, int foregroundColor)

/**
 * 同步创建指定前景色、白色背景色、带logo的二维码图片。该方法是耗时操作，请在子线程中调用。
 *
 * @param content         要生成的二维码图片内容
 * @param size            图片宽高，单位为px
 * @param foregroundColor 二维码图片的前景色
 * @param logo            二维码图片的logo
 */
public static Bitmap syncEncodeQRCode(String content, int size, int foregroundColor, Bitmap logo)

/**
 * 同步创建指定前景色、指定背景色、带logo的二维码图片。该方法是耗时操作，请在子线程中调用。
 *
 * @param content         要生成的二维码图片内容
 * @param size            图片宽高，单位为px
 * @param foregroundColor 二维码图片的前景色
 * @param backgroundColor 二维码图片的背景色
 * @param logo            二维码图片的logo
 */
public static Bitmap syncEncodeQRCode(String content, int size, int foregroundColor, int backgroundColor, Bitmap logo)

/**
 * 同步创建条形码图片
 *
 * @param content  要生成条形码包含的内容
 * @param width    条形码的宽度，单位px
 * @param height   条形码的高度，单位px
 * @param textSize 字体大小，单位px，如果等于0则不在底部绘制文字
 */
public static Bitmap syncEncodeBarcode(String content, int width, int height, int textSize)
```

#### 详细用法请查看[ZBarDemo](https://github.com/bingoogolapple/BGAQRCode-Android/tree/master/zbardemo):feet:

#### 详细用法请查看[ZXingDemo](https://github.com/bingoogolapple/BGAQRCode-Android/tree/master/zxingdemo):feet:

## 关于我

| 个人主页 | 邮箱 | BGA系列开源库QQ群
| ------------- | ------------ | ------------ |
| <a  href="http://www.bingoogolapple.cn" target="_blank">bingoogolapple.cn</a>  | <a href="mailto:bingoogolapple@gmail.com" target="_blank">bingoogolapple@gmail.com</a> | ![BGA_CODE_CLUB](http://bgashare.bingoogolapple.cn/BGA_CODE_CLUB.png?imageView2/2/w/200) |

## 打赏支持

如果您觉得 BGA 系列开源库帮你节省了大量的开发时间，请扫描下方的二维码随意打赏，要是能打赏个 10.24 :monkey_face:就太:thumbsup:了。您的支持将鼓励我继续创作:octocat:

如果您目前正打算购买通往墙外的梯子，可以使用我的邀请码「YFQ9Q3B」购买 [Lantern](https://github.com/getlantern/forum)，双方都赠送三个月的专业版使用时间:beers:

<p align="center">
  <img src="http://bgashare.bingoogolapple.cn/bga_pay.png?imageView2/2/w/450" width="450">
</p>

## License

    Copyright (C) 2012 The Android Open Source Project
    Copyright 2014 bingoogolapple

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
