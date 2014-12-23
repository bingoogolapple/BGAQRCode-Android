package cn.bingoogolapple.qrcode.zbar;

import net.sourceforge.zbar.Symbol;

import java.util.List;
import java.util.ArrayList;

public class BarcodeFormat {
    private int mId;
    private String mName;

    public static final BarcodeFormat NONE = new BarcodeFormat(Symbol.NONE, "NONE");
    public static final BarcodeFormat PARTIAL = new BarcodeFormat(Symbol.PARTIAL, "PARTIAL");
    public static final BarcodeFormat EAN8 = new BarcodeFormat(Symbol.EAN8, "EAN8");
    public static final BarcodeFormat UPCE = new BarcodeFormat(Symbol.UPCE, "UPCE");
    public static final BarcodeFormat ISBN10 = new BarcodeFormat(Symbol.ISBN10, "ISBN10");
    public static final BarcodeFormat UPCA = new BarcodeFormat(Symbol.UPCA, "UPCA");
    public static final BarcodeFormat EAN13 = new BarcodeFormat(Symbol.EAN13, "EAN13");
    public static final BarcodeFormat ISBN13 = new BarcodeFormat(Symbol.ISBN13, "ISBN13");
    public static final BarcodeFormat I25 = new BarcodeFormat(Symbol.I25, "I25");
    public static final BarcodeFormat DATABAR = new BarcodeFormat(Symbol.DATABAR, "DATABAR");
    public static final BarcodeFormat DATABAR_EXP = new BarcodeFormat(Symbol.DATABAR_EXP, "DATABAR_EXP");
    public static final BarcodeFormat CODABAR = new BarcodeFormat(Symbol.CODABAR, "CODABAR");
    public static final BarcodeFormat CODE39 = new BarcodeFormat(Symbol.CODE39, "CODE39");
    public static final BarcodeFormat PDF417 = new BarcodeFormat(Symbol.PDF417, "PDF417");
    public static final BarcodeFormat QRCODE = new BarcodeFormat(Symbol.QRCODE, "QRCODE");
    public static final BarcodeFormat CODE93 = new BarcodeFormat(Symbol.CODE93, "CODE93");
    public static final BarcodeFormat CODE128 = new BarcodeFormat(Symbol.CODE128, "CODE128");

    public static final List<BarcodeFormat> ALL_FORMATS = new ArrayList<BarcodeFormat>();

    static {
        ALL_FORMATS.add(BarcodeFormat.PARTIAL);
        ALL_FORMATS.add(BarcodeFormat.EAN8);
        ALL_FORMATS.add(BarcodeFormat.UPCE);
        ALL_FORMATS.add(BarcodeFormat.ISBN10);
        ALL_FORMATS.add(BarcodeFormat.UPCA);
        ALL_FORMATS.add(BarcodeFormat.EAN13);
        ALL_FORMATS.add(BarcodeFormat.ISBN13);
        ALL_FORMATS.add(BarcodeFormat.I25);
        ALL_FORMATS.add(BarcodeFormat.DATABAR);
        ALL_FORMATS.add(BarcodeFormat.DATABAR_EXP);
        ALL_FORMATS.add(BarcodeFormat.CODABAR);
        ALL_FORMATS.add(BarcodeFormat.CODE39);
        ALL_FORMATS.add(BarcodeFormat.PDF417);
        ALL_FORMATS.add(BarcodeFormat.QRCODE);
        ALL_FORMATS.add(BarcodeFormat.CODE93);
        ALL_FORMATS.add(BarcodeFormat.CODE128);
    }

    public BarcodeFormat(int id, String name) {
        mId = id;
        mName = name;
    }

    public int getId() {
        return mId;
    }

    public String getName() {
        return mName;
    }

    public static BarcodeFormat getFormatById(int id) {
        for(BarcodeFormat format : ALL_FORMATS) {
            if(format.getId() == id) {
                return format;
            }
        }
        return BarcodeFormat.NONE;
    }
}