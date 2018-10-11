#!/bin/bash +x

./gradlew :qrcodecore:clean :qrcodecore:build :qrcodecore:bintrayUpload -PpublishAar
./gradlew :zxing:clean :zxing:build :zxing:bintrayUpload -PpublishAar
./gradlew :zbar:clean :zbar:build :zbar:bintrayUpload -PpublishAar