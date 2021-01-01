package com.unnamed.satconv;

import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

// Format:
//   4B signature: "FON\0"
//   2B image byte size
public class FontConverter {
    private final static String FILE_SIGNATURE = "FON";

    private TileSetConverter tileSetConverter;

    public FontConverter() {
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

            stream.writeBytes(FILE_SIGNATURE);
            stream.writeByte(0x00);
            stream.writeShort(imagesBytes.length);

            stream.write(imagesBytes);
            stream.write(paletteBytes);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
