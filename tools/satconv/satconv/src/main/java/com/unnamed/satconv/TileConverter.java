package com.unnamed.satconv;

import java.awt.image.BufferedImage;
import java.awt.image.IndexColorModel;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;

import javax.imageio.ImageIO;

public class TileConverter {
    private byte[] tileBytes;
    private byte[] paletteBytes;

    private int width;
    private int height;
    private int bpp;

    public TileConverter(BufferedImage bufferedImage) throws IOException {
        ArrayList<Byte> tileByteList = new ArrayList<>();

            IndexColorModel indexColorModel;

            if (bufferedImage.getColorModel() instanceof IndexColorModel) {
                indexColorModel = (IndexColorModel)bufferedImage.getColorModel();
            } else {
                System.out.println("ERROR: Image not indexed color");
                return;
            }

            width = bufferedImage.getWidth();
            height = bufferedImage.getHeight();
            bpp = indexColorModel.getPixelSize();

            // Split image up into 16x16 tiles
            int[] imageData = new int[8 * 8];

            for (int i = 0; i < height; i += 16) {
                for (int j = 0; j < width; j += 16) {
                    // Top left
                    imageData = bufferedImage.getData().getPixels(j, i, 8, 8, imageData);
                    if (bpp == 8) {
                        for (int k = 0; k < imageData.length; k++) {
                            tileByteList.add((byte) (imageData[k] & 0xFF));
                        }
                    } else if (bpp == 4) {
                        for (int k = 0; k < imageData.length; k += 2) {
                            byte pixels = (byte) (((imageData[k] & 0xF) << 4) | (imageData[k+1] & 0xF));
                            tileByteList.add(pixels);
                        }
                    }
                    // Top right
                    imageData = bufferedImage.getData().getPixels(j + 8, i, 8, 8, imageData);
                    if (bpp == 8) {
                        for (int k = 0; k < imageData.length; k++) {
                            tileByteList.add((byte) (imageData[k] & 0xFF));
                        }
                    } else if (bpp == 4) {
                        for (int k = 0; k < imageData.length; k += 2) {
                            byte pixels = (byte) (((imageData[k] & 0xF) << 4) | (imageData[k+1] & 0xF));
                            tileByteList.add(pixels);
                        }
                    }
                    // Bottom left
                    imageData = bufferedImage.getData().getPixels(j, i + 8, 8, 8, imageData);
                    if (bpp == 8) {
                        for (int k = 0; k < imageData.length; k++) {
                            tileByteList.add((byte) (imageData[k] & 0xFF));
                        }
                    } else if (bpp == 4) {
                        for (int k = 0; k < imageData.length; k += 2) {
                            byte pixels = (byte) (((imageData[k] & 0xF) << 4) | (imageData[k+1] & 0xF));
                            tileByteList.add(pixels);
                        }
                    }
                    // Bottom right
                    imageData = bufferedImage.getData().getPixels(j + 8, i + 8, 8, 8, imageData);
                    if (bpp == 8) {
                        for (int k = 0; k < imageData.length; k++) {
                            tileByteList.add((byte) (imageData[k] & 0xFF));
                        }
                    } else if (bpp == 4) {
                        for (int k = 0; k < imageData.length; k += 2) {
                            byte pixels = (byte) (((imageData[k] & 0xF) << 4) | (imageData[k+1] & 0xF));
                            tileByteList.add(pixels);
                        }
                    }
                }
            }

            tileBytes = ByteUtils.toPrimitiveBytes(tileByteList);
            paletteBytes = ImageUtils.getRGB1555Bytes(indexColorModel);
    }

    public TileConverter(File bmpFile) throws IOException {
        this(ImageIO.read(bmpFile));
    }

    public int getWidth() {
        return width;
    }

    public int getHeight() {
        return height;
    }

    public byte[] toTilesByteArray() {
        return tileBytes;
    }

    public byte[] toPaletteByteArray() {
        return paletteBytes;
    }
}
