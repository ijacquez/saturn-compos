package com.unnamed.satconv;

import java.awt.image.BufferedImage;
import java.awt.image.IndexColorModel;
import java.awt.image.Raster;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import javax.imageio.ImageIO;

// Format:
//   4B signature: "SPR\0"
//   2B image count
//   2B image byte size
//   2B image palette byte size
//   2B   image width
//   2B   image height
//   2B   image width
//   2B   image height
//   ...
//   *B image bytes
//   *B image palette bytes
public class SpriteConverter {
    private final static String FILE_SIGNATURE = "SPR";

    private TileSetConverter tileSetConverter;

    public SpriteConverter() {
        tileSetConverter = new TileSetConverter();
    }

    public void addImage(File imageFile) throws IOException {
        tileSetConverter.addImage(imageFile);
    }

    public void write(String filename) throws Exception {
        if (tileSetConverter.getImageSize() == 0) {
            return;
        }

        try (DataOutputStream stream = new DataOutputStream(new FileOutputStream(filename))) {
            byte[] imagesBytes = tileSetConverter.getImageBytes();
            byte[] paletteBytes = tileSetConverter.getPaletteBytes();
            int cmapSize = tileSetConverter.getCMAPSize();

            stream.writeBytes(FILE_SIGNATURE);
            stream.writeByte(0x00);
            stream.writeShort(tileSetConverter.getImageSize());
            stream.writeShort(imagesBytes.length);
            stream.writeShort(cmapSize);

            for (TileSetImage image : tileSetConverter) {
                stream.writeShort(image.getWidth());  // 2B image width
                stream.writeShort(image.getHeight()); // 2B image height
            }

            stream.write(imagesBytes);
            stream.write(paletteBytes);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
