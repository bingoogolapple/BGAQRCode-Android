#!/bin/bash +x

sed -i -e "s/\/\/ classpath 'com.novoda:bintray-release:0.8.1'/classpath 'com.novoda:bintray-release:0.8.1'/" build.gradle

./gradlew :qrcodecore:clean :qrcodecore:build :qrcodecore:bintrayUpload -PpublishAar
./gradlew :zxing:clean :zxing:build :zxing:bintrayUpload -PpublishAar
./gradlew :zbar:clean :zbar:build :zbar:bintrayUpload -PpublishAar

sed -i -e "s/classpath 'com.novoda:bintray-release:0.8.1'/\/\/ classpath 'com.novoda:bintray-release:0.8.1'/" build.gradle