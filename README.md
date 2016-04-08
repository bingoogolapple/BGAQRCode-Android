:running:QRCode-Android:running:
============

[![License](https://img.shields.io/badge/license-Apache%202-green.svg)](https://www.apache.org/licenses/LICENSE-2.0)
[![Maven Central](https://maven-badges.herokuapp.com/maven-central/cn.bingoogolapple/qrcodecore/badge.svg)](https://maven-badges.herokuapp.com/maven-central/cn.bingoogolapple/qrcodecore)

根据公司项目需求，参考这个项目改的 [barcodescanner](https://github.com/dm77/barcodescanner)

主要功能：ZXing生成二维码、ZXing扫描二维码、ZBar扫描二维码(扫描中文会有乱码)、可控制闪光灯和定制各式各样的扫描框

| [点击下载ZXingDemo.apk](http://fir.im/ZXingDemo)或扫描下面的二维码安装 | [点击下载ZXingDemo apk](http://fir.im/ZXingDemo)或扫描下面的二维码安装 |
| ------------ | ------------ |
| ![ZXingDemo apk文件二维码](http://7xk9dj.com1.z0.glb.clouddn.com/qrcode/zxingdemoapk.png) | ![ZBarDemo apk文件二维码](http://7xk9dj.com1.z0.glb.clouddn.com/qrcode/zbardemoapk.png) |

### 效果图
![Image of ZXingDemo](http://7xk9dj.com1.z0.glb.clouddn.com/qrcode/screenshots/zxing102.gif)
![Image of ZBarDemo](http://7xk9dj.com1.z0.glb.clouddn.com/qrcode/screenshots/zbar102.gif)
![Image of IqeggQRCodeDemo](http://7xk9dj.com1.z0.glb.clouddn.com/qrcode/screenshots/IqeggQRCodeDemo.gif)

### Gradle依赖
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
### Layout
>ZXing

```xml
<cn.bingoogolapple.qrcode.zxing.ZXingView
    android:id="@+id/zxingview"
    style="@style/MatchMatch"
    app:qrcv_cornerColor="@android:color/white"
    app:qrcv_cornerLength="20dp"
    app:qrcv_cornerSize="2dp"
    app:qrcv_maskColor="#33ffffff"
    app:qrcv_rectWidth="200dp"
    app:qrcv_scanLineColor="@android:color/white"
    app:qrcv_scanLineSize="1dp"
    app:qrcv_topOffset="80dp" />
```
>ZBar

```xml
<cn.bingoogolapple.qrcode.zbar.ZBarView
    android:id="@+id/zbarview"
    style="@style/MatchMatch"
    app:qrcv_cornerColor="@color/colorPrimaryDark"
    app:qrcv_cornerLength="25dp"
    app:qrcv_cornerSize="3dp"
    app:qrcv_maskColor="#66ffffff"
    app:qrcv_rectWidth="220dp"
    app:qrcv_scanLineColor="@color/colorPrimary"
    app:qrcv_scanLineSize="2dp"
    app:qrcv_topOffset="90dp" />
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

### 关于我

| 新浪微博 | 个人主页 | 邮箱 | BGA系列开源库QQ群 |
| ------------ | ------------- | ------------ | ------------ |
| <a href="http://weibo.com/bingoogol" target="_blank">bingoogolapple</a> | <a  href="http://www.bingoogolapple.cn" target="_blank">bingoogolapple.cn</a>  | <a href="mailto:bingoogolapple@gmail.com" target="_blank">bingoogolapple@gmail.com</a> | ![BGA_CODE_CLUB](http://7xk9dj.com1.z0.glb.clouddn.com/BGA_CODE_CLUB.png?imageView2/2/w/200) |

## License

    Copyright 2015 bingoogolapple

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
