package cn.bingoogolapple.qrcode.zxingdemo;

/**
 * 作者:王浩
 * 创建时间:2018/6/19
 * 描述:
 */
public class RotateTest {
    public static void main(String[] args) {
        byte[] data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

        oneDimension2TwoDimension(data, 3, 5);
        oneDimension2TwoDimensionLeftRotate90(data, 5, 3);
        oneDimension2TwoDimensionRightRotate90(data, 5, 3);
        oneDimensionRightRotate90(data, 3, 5);
    }

    private static void oneDimension2TwoDimension(byte[] data, int height, int width) {
        int k = 0;
        byte[][] twoDegree = new byte[height][width];
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                twoDegree[i][j] = data[k];
                k++;
            }
        }
        displayTwoDimension(twoDegree, height, width);
        towDimension2OneDimension(twoDegree, height, width);
    }

    private static void towDimension2OneDimension(byte[][] twoDegree, int height, int width) {
        byte[] oneDimension = new byte[height * width];
        int k = 0;
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                oneDimension[k] = twoDegree[i][j];
                k++;
            }
        }

        displayOneDimension(oneDimension);
    }

    private static void oneDimension2TwoDimensionLeftRotate90(byte[] oneDimension, int height, int width) {
        int k = 0;
        byte[][] twoDimension = new byte[height][width];
        for (int i = 0; i < width; i++) {
            for (int j = height - 1; j >= 0; j--) {
                twoDimension[j][i] = oneDimension[k];
                k++;
            }
        }
        displayTwoDimension(twoDimension, height, width);
        towDimension2OneDimension(twoDimension, height, width);
    }

    private static void oneDimension2TwoDimensionRightRotate90(byte[] oneDimension, int height, int width) {
        int k = 0;
        byte[][] twoDimension = new byte[height][width];
        for (int i = width - 1; i >= 0; i--) {
            for (int j = 0; j < height; j++) {
                twoDimension[j][i] = oneDimension[k];
                k++;
            }
        }

        displayTwoDimension(twoDimension, height, width);
        towDimension2OneDimension(twoDimension, height, width);
    }

    private static void oneDimensionRightRotate90(byte[] oneDimension, int height, int width) {
        byte[] result = new byte[oneDimension.length];
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                result[x * height + height - y - 1] = oneDimension[x + y * width];
            }
        }
        displayOneDimension(result);
        oneDimension2TwoDimension(result, width, height);
    }

    private static void displayTwoDimension(byte[][] twoDimension, int height, int width) {
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                System.out.print(twoDimension[i][j] + " ");
            }
            System.out.println();
        }
    }

    private static void displayOneDimension(byte[] oneDimension) {
        for (int i = 0; i < oneDimension.length; i++) {
            System.out.print(oneDimension[i] + " ");
        }
        System.out.println();
        System.out.println();
    }
}
