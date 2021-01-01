package com.unnamed.satconv;

import java.awt.image.BufferedImage;

public final class TileSetImage {
    private int width;
    private int height;

    private TileSetImage() {
    }

    public TileSetImage(int width, int height) {
        this.width = width;
        this.height = height;
    }

    public int getWidth() {
        return width;
    }

    public int getHeight() {
        return height;
    }
}
