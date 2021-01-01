package com.unnamed.satconv;

import java.awt.image.BufferedImage;
import java.awt.image.IndexColorModel;
import java.awt.image.Raster;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

import javax.imageio.ImageIO;

public class TileSetConverter implements Iterable<TileSetImage> {
    private final class Image {
        public BufferedImage bufferedImage;
        public int[] cmap;
    }

    private ArrayList<Image> images;
    private Set<Integer> cmapHashSet;

    public TileSetConverter() {
        images = new ArrayList<>();
        cmapHashSet = new HashSet<>();
    }

    public void addImage(File imageFile) throws IOException {
        Image image = new Image();

        image.bufferedImage = ImageIO.read(imageFile);
        image.cmap = ImageUtils.getCMAP(image.bufferedImage);

        for (int color : image.cmap) {
            cmapHashSet.add(color);
        }

        images.add(image);

        System.out.println(String.format("Adding sprite image \"%s\" (%d,%d)",
                                         imageFile,
                                         image.bufferedImage.getWidth(),
                                         image.bufferedImage.getHeight()));
    }

    public int getImageSize() {
        return images.size();
    }

    public int getCMAPSize() {
        try {
            int[] cmap = getCMAP();

            return cmap.length;
        } catch (Exception e) {
            e.printStackTrace();
        }

        return 0;
    }

    public byte[] getImageBytes() throws Exception {
        int[] cmap = getCMAP();
        byte[] imagesBytes = coalesceAllImageBytes(cmap);

        return imagesBytes;
    }

    public byte[] getPaletteBytes() throws Exception {
        int[] cmap = getCMAP();
        byte[] paletteBytes = ImageUtils.getRGB1555Bytes(cmap);

        return paletteBytes;
    }

    public Iterator<TileSetImage> iterator() {
        ArrayList<TileSetImage> tileSetImages = new ArrayList<>();

        for (Image image : images) {
            TileSetImage tileSetImage = new TileSetImage(image.bufferedImage.getWidth(),
                                                         image.bufferedImage.getHeight());

            tileSetImages.add(tileSetImage);
        }

        return tileSetImages.iterator();
    }

    private byte[] coalesceAllImageBytes(int[] cmap) {
        ArrayList<byte[]> imageBytesList = new ArrayList<>();

        // int index = 0;

        for (Image image : images) {
            BufferedImage indexedImage =
                ImageUtils.convertToIndexed(image.bufferedImage, cmap);
            byte[] imageBytes = getImageBytes(indexedImage);

            imageBytesList.add(imageBytes);

            try {
                // ImageIO.write(indexedImage, "PNG", new File(String.format("aaa_indexed_%03d.png", index)));
                // index++;
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        int length = 0;

        for (byte[] imageBytes : imageBytesList) {
            length += imageBytes.length;
        }

        byte[] bytes = new byte[length];

        int i = 0;
        for (byte[] imageBytes : imageBytesList) {
            for (int j = 0; j < imageBytes.length; i++, j++) {
                bytes[i] = imageBytes[j];
            }
        }

        return bytes;
    }

    private byte[] getImageBytes(BufferedImage bufferedImage) {
        int imageWidth = bufferedImage.getWidth();
        int imageHeight = bufferedImage.getHeight();
        Raster raster = bufferedImage.getData();

        int[] pixels = null;
        pixels = raster.getPixels(0, 0, imageWidth, imageHeight, pixels);

        byte[] bytes = null;

        IndexColorModel indexColorModel = (IndexColorModel)bufferedImage.getColorModel();
        int colorCount = indexColorModel.getMapSize();

        if (colorCount <= 16) {
            bytes = new byte[pixels.length / 2];

            for (int i = 0, j = 0; i < pixels.length; i += 2, j++) {
                bytes[j] = (byte)(((pixels[i] & 0x0F) << 4) | (pixels[i + 1] & 0x0F));
            }
        } else if (colorCount <= 256) {
            bytes = new byte[pixels.length];

            for (int i = 0; i < pixels.length; i++) {
                bytes[i] = (byte)(pixels[i] & 0xFF);
            }
        }

        return bytes;
    }

    private int[] getCMAP() throws Exception {
        int cmapLength = cmapHashSet.size();

        if (cmapLength <= 16) {
            cmapLength = 16;
        } else if (cmapLength <= 256) {
            cmapLength = 256;
        } else {
            throw new IllegalArgumentException(String.format("There are too many colors: %d", cmapLength));
        }

        int[] cmap = new int[cmapLength];

        int i = 0;
        for (int color : cmapHashSet) {
            cmap[i] = color;

            // System.out.println(String.format("Color: #%3d. #%08X", i, color));

            i++;
        }

        return cmap;
    }
}
