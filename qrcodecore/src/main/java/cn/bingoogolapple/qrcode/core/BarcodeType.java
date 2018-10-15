package cn.bingoogolapple.qrcode.core;

/**
 * 作者:王浩
 * 创建时间:2018/6/20
 * 描述:识别的格式
 */
public enum BarcodeType {
    /**
     * 所有格式
     */
    ALL,
    /**
     * 所有一维条码格式
     */
    ONE_DIMENSION,
    /**
     * 所有二维条码格式
     */
    TWO_DIMENSION,
    /**
     * 仅 QR_CODE
     */
    ONLY_QR_CODE,
    /**
     * 仅 CODE_128
     */
    ONLY_CODE_128,
    /**
     * 仅 EAN_13
     */
    ONLY_EAN_13,
    /**
     * 高频率格式，包括 QR_CODE、ISBN13、UPC_A、EAN_13、CODE_128
     */
    HIGH_FREQUENCY,
    /**
     * 自定义格式
     */
    CUSTOM
}
