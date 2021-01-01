package com.unnamed.satconv;

import java.awt.Graphics;
import java.awt.Transparency;
import java.awt.image.BufferedImage;
import java.awt.image.DataBuffer;
import java.awt.image.IndexColorModel;
import java.awt.image.Raster;
import java.awt.image.WritableRaster;
import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;
import java.util.Set;

import org.mapeditor.core.Tile;
import org.mapeditor.core.TileLayer;

public class ImageUtils {
    public static BufferedImage getTileSetImage(ArrayList<Tile> tiles) {
        int imageWidth = 0;
        int imageHeight = 0;

        for (Tile tile : tiles) {
            BufferedImage image = tile.getImage();

            imageWidth = Math.max(imageWidth, image.getWidth());
            imageHeight += image.getHeight();
        }

        if ((imageWidth == 0) || (imageHeight == 0)) {
            return null;
        }

        BufferedImage tileSetImage = new BufferedImage(imageWidth,
                                                       imageHeight,
                                                       BufferedImage.TYPE_INT_ARGB);

        Graphics g = tileSetImage.getGraphics();
        int imageOffset = 0;

        for (Tile tile : tiles) {
            BufferedImage image = tile.getImage();

            g.drawImage(image, 0, imageOffset, null);

            imageOffset += image.getHeight();
        }
        g.dispose();

        return tileSetImage;
    }

    public static BufferedImage getTileSetImage(TileLayer tileLayer) {
        ArrayList<Tile> tiles = new ArrayList<>();
        Set<Integer> tileHashSet = new HashSet<>();

        int imageWidth = 0;
        int imageHeight = 0;

        for (int y = 0; y < tileLayer.getHeight(); y++) {
            for (int x = 0; x < tileLayer.getWidth(); x++) {
                Tile tile = tileLayer.getTileAt(x, y);

                if ((tile != null) && !tileHashSet.contains(tile.getId())) {
                    tileHashSet.add(tile.getId());

                    BufferedImage image = tile.getImage();

                    imageWidth = Math.max(imageWidth, image.getWidth());
                    imageHeight += image.getHeight();

                    tiles.add(tile);
                }
            }
        }

        Collections.sort(tiles, tileComparator);

        BufferedImage tileSetImage = new BufferedImage(imageWidth,
                                                       imageHeight,
                                                       BufferedImage.TYPE_INT_ARGB);

        Graphics g = tileSetImage.getGraphics();
        int imageOffset = 0;
        for (Tile tile : tiles) {
            BufferedImage image = tile.getImage();

            g.drawImage(image, 0, imageOffset, null);

            imageOffset += image.getHeight();
        }
        g.dispose();

        return tileSetImage;
    }

    public static int[] getCMAP(BufferedImage image) {
        return getCMAP(image, -1);
    }

    public static int[] getCMAP(BufferedImage image, int bpp) {
        Set<Integer> uniqueColors = new HashSet<>();

        for (int y = 0; y < image.getHeight(); y++) {
            for (int x = 0; x < image.getWidth(); x++) {
                // System.out.println(String.format("COLOR: 0x%08X", image.getRGB(x, y)));

                int color = image.getRGB(x, y) | 0xFF000000;

                uniqueColors.add(color);
            }
        }

        int colorCount = uniqueColors.size();
        int roundedColorCount = colorCount;

        switch (bpp) {
        case 4:
            roundedColorCount = 16;
            break;
        case 8:
            roundedColorCount = 256;
        default:
            if (roundedColorCount < 16) {
                roundedColorCount = 16;
            } else if (roundedColorCount > 16) {
                roundedColorCount = 256;
            }
        }

        int[] cmap = new int[roundedColorCount];
        Integer[] values = uniqueColors.toArray(new Integer[uniqueColors.size()]);

        for (int i = 0; i < roundedColorCount; i++) {
            if (i < values.length) {
                cmap[i] = values[i];
            } else {
                cmap[i] = 0xFFFFFFFF;
            }

            // System.out.println(String.format("#%08X", colors[i]));
        }

        return cmap;
    }

    public static BufferedImage convertToIndexed(BufferedImage srcImage) {
        int[] cmap = getCMAP(srcImage, -1);

        return convertToIndexed(srcImage, cmap, -1);
    }

    public static BufferedImage convertToIndexed(BufferedImage srcImage, int[] cmap) {
        return convertToIndexed(srcImage, cmap, -1);
    }

    public static BufferedImage convertToIndexed(BufferedImage srcImage, int bpp) {
        int[] cmap = getCMAP(srcImage, bpp);

        return convertToIndexed(srcImage, cmap, bpp);
    }

    public static BufferedImage convertToIndexed(BufferedImage srcImage, int cmap[], int bpp) {
        int transparentIndex = 0;

        for (int i = 0; i < cmap.length; i++) {
            int alpha = cmap[i] >> 24;

            // System.out.println(String.format("#%08X -> alpha: #%02X, index: %d", cmap[i], alpha, i));

            if (alpha < 0xFF) {
                transparentIndex = i;
            }
        }

        int bitDepth = (cmap.length == 16) ? 4 : 8;

        if (bpp >= 0) {
            bitDepth = bpp;
        }

        IndexColorModel indexColorModel = new IndexColorModel(bitDepth,
                                                              cmap.length,
                                                              cmap,
                                                              0,
                                                              false,
                                                              transparentIndex,
                                                              DataBuffer.TYPE_BYTE);

        BufferedImage dstImage = new BufferedImage(srcImage.getWidth(),
                                                   srcImage.getHeight(),
                                                   BufferedImage.TYPE_BYTE_INDEXED,
                                                   indexColorModel);

        Raster srcRaster = srcImage.getRaster();
        WritableRaster writableRaster =
            indexColorModel.createCompatibleWritableRaster(srcImage.getWidth(),
                                                           srcImage.getHeight());

        int[] inRGB = new int[4];
        Object pixel = null;

        for (int y = 0; y < srcImage.getHeight(); y++) {
            for (int x = 0; x < srcImage.getWidth(); x++) {
                srcRaster.getPixel(x, y, inRGB);
                pixel = indexColorModel.getDataElements(toIntARGB(inRGB), pixel);

                // System.out.println(String.format("-> (%d,%d) #0x%08X", x, y, toIntARGB(inRGB)));

                writableRaster.setDataElements(x, y, pixel);
            }
        }

        dstImage.setData(writableRaster);

        return dstImage;
    }

    public static byte[] getRGB1555Bytes(int[] cmap) throws IOException {
        ByteArrayOutputStream outputStream = new ByteArrayOutputStream();

        try (DataOutputStream stream = new DataOutputStream(outputStream)) {
            for (int i = 0; i < cmap.length; i++) {
                int r = (cmap[i] >> 16) & 0xFF;
                int g = (cmap[i] >> 8) & 0xFF;
                int b = cmap[i] & 0xFF;

                int r555 = (r >> 3) & 0x1F;
                int g555 = (g >> 3) & 0x1F;
                int b555 = (b >> 3) & 0x1F;

                int value = 0x8000 | (b555 << 10) | (g555 << 5) | r555;

                stream.writeShort(value);
            }
        }

        return outputStream.toByteArray();
    }

    public static byte[] getRGB1555Bytes(IndexColorModel indexColorModel) {
        int colorCount = indexColorModel.getMapSize();

        byte[] reds = new byte[colorCount];
        byte[] greens = new byte[colorCount];
        byte[] blues = new byte[colorCount];

        indexColorModel.getReds(reds);
        indexColorModel.getGreens(greens);
        indexColorModel.getBlues(blues);

        byte[] bytes = new byte[colorCount * 2];

        for (int i = 0, color = 0; i < colorCount; i++, color += 2) {
            // Saturn palette format:
            // 0123456789ABCDEF0123456789ABCDEF
            // RRRRRRRRGGGGGGGGBBBBBBBB00000000

            int r555 = ((reds[i] >> 3) & 0x1F);
            int g555 = ((greens[i] >> 3) & 0x1F);
            int b555 = ((blues[i] >> 3) & 0x1F);

            int value = 0x8000 | (b555 << 10) | (g555 << 5) | r555;

            bytes[color    ] = (byte)((value >> 8) & 0xFF);
            bytes[color + 1] = (byte)(value & 0xFF);
        }

        return bytes;
    }

    private static int toIntARGB(int[] rgb) {
        return (0xFF000000 |
                ((rgb[0] & 0xFF) << 16) |
                ((rgb[1] & 0xFF) << 8) |
                (rgb[2] & 0xFF));
    }

    private static Comparator<Tile> tileComparator = new Comparator<Tile>() {
        public int compare(Tile tile1, Tile tile2) {
            return (tile1.getId() - tile2.getId());
        }
    };
}
