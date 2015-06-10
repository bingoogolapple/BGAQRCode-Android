:running:QRCode-Android v1.0.0:running:
============
根据公司项目需要，参考这个项目改的 [barcodescanner](https://github.com/dm77/barcodescanner)

主要功能：ZXing生成二维码、ZXing扫描二维码、ZBar扫描二维码

#### 效果图
![Image of IqeggQRCodeDemo](https://raw.githubusercontent.com/bingoogolapple/QRCode-Android/master/screenshots/IqeggQRCodeDemo.gif)
![Image of ZXingDemo](https://raw.githubusercontent.com/bingoogolapple/QRCode-Android/master/screenshots/ZXingDemo.gif)
![Image of ZBarDemo](https://raw.githubusercontent.com/bingoogolapple/QRCode-Android/master/screenshots/ZBarDemo.gif)

>ZXing

```groovy
dependencies {
    compile 'com.google.zxing:core:3.1.0'
    compile 'cn.bingoogolapple:qrcodecore:1.0.0@aar'
    compile 'cn.bingoogolapple:zxing:1.0.0@aar'
}
```
>ZBar

```groovy
dependencies {
    compile 'cn.bingoogolapple:qrcodecore:1.0.0@aar'
    compile 'cn.bingoogolapple:zbar:1.0.0@aar'
}
```

##### 详细用法请查看[ZBarDemo](https://github.com/bingoogolapple/QRCode-Android/tree/master/zbardemo):feet:

##### 详细用法请查看[ZXingDemo](https://github.com/bingoogolapple/QRCode-Android/tree/master/zxingdemo):feet:

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
