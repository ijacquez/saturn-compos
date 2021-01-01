package com.unnamed.satconv;

import java.awt.image.BufferedImage;
import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.Set;

import javax.imageio.ImageIO;

import org.mapeditor.core.Map;
import org.mapeditor.core.MapLayer;
import org.mapeditor.core.MapObject;
import org.mapeditor.core.ObjectGroup;
import org.mapeditor.core.Properties;
import org.mapeditor.core.Tile;
import org.mapeditor.core.TileLayer;
import org.mapeditor.io.TMXMapReader;

public class LevelConverter {
    private static final String HEADER_FILE_SIG = "LVL";

    private static final int LAYER_COUNT = 4;

    private static final String LAYER_NAME_PLAYFIELD = "playfield";
    private static final String LAYER_NAME_NEAR      = "near";
    private static final String LAYER_NAME_FAR       = "far";
    private static final String LAYER_NAME_OVERLAP   = "overlap";

    private static final String OBJECTGROUP_NAME_OBJECTS  = "objects";
    private static final String OBJECTGROUP_NAME_TRIGGERS = "triggers";

    private static final String TILE_PROP_NAME_COLLISION             = "collision";
    private static final String TILE_PROP_DEFAULT_VALUE_COLLISION    = "false";

    private static final String TILE_PROP_NAME_INSTANT_KILL          = "instantKill";
    private static final String TILE_PROP_DEFAULT_VALUE_INSTANT_KILL = "false";

    private static final String LAYER_PROP_NAME_BPP                  = "bpp";
    private static final String LAYER_PROP_DEFAULT_VALUE_BPP         = "4";

    private static final String LAYER_PROP_NAME_CROP_WIDTH           = "cropWidth";
    private static final String LAYER_PROP_DEFAULT_VALUE_CROP_WIDTH  = "-1";

    private static final String LAYER_PROP_NAME_CROP_HEIGHT          = "cropHeight";
    private static final String LAYER_PROP_DEFAULT_VALUE_CROP_HEIGHT = "-1";

    private enum TileProperty {
        COLLISION,
        INSTANT_KILL
    }

    private enum LayerProperty {
        BPP,
        CROP_WIDTH,
        CROP_HEIGHT
    }

    private enum LayerFlags {
        LAYER_FLAG_NONE
    }

    private class Layer {
        public TileLayer tileLayer;

        public int width;
        public int height;
        public int bpp;
        public int cropWidth;
        public int cropHeight;

        public ArrayList<Tile> referencedTiles = new ArrayList<>();

        // Options
        public boolean useRawMapValues;
    }

    private enum MapValueFlags {
        MAP_VALUE_FLAG_COLLISION,
        MAP_VALUE_FLAG_INSTANT_KILL,
        MAP_VALUE_FLAG_TRIGGER
    }

    private static class MapValue {
        public int tileId;
        public int pnd;
        public int triggerIndex;
        public EnumSet<MapValueFlags> flags = EnumSet.noneOf(MapValueFlags.class);
    }

    private Map map;
    private TMXMapReader mapReader;

    private TileLayer tileLayerPlayfield;
    private TileLayer tileLayerNear;
    private TileLayer tileLayerFar;
    private TileLayer tileLayerOverlap;

    private ObjectGroup objectGroupObjects;
    private ObjectGroup objectGroupTriggers;

    private LevelObjects triggerLevelObjects;
    private LevelObjects objectLevelObjects;

    private LevelConverter() {
    }

    public LevelConverter(LevelObjects objectLevelObjects,
                          LevelObjects triggerLevelObjects,
                          String tmxFilePath) throws Exception {
        mapReader = new TMXMapReader();
        map = mapReader.readMap(tmxFilePath);

        this.objectLevelObjects = objectLevelObjects;
        this.triggerLevelObjects = triggerLevelObjects;

        for (MapLayer layer : map.getLayers()) {
            String layerName = layer.getName().trim().toLowerCase();

            if (layer instanceof TileLayer) {
                if (layerName.equals(LAYER_NAME_PLAYFIELD)) {
                    tileLayerPlayfield = (TileLayer)layer;
                } else if (layerName.equals(LAYER_NAME_NEAR)) {
                    tileLayerNear = (TileLayer)layer;
                } else if (layerName.equals(LAYER_NAME_FAR)) {
                    tileLayerFar = (TileLayer)layer;
                } else if (layerName.equals(LAYER_NAME_OVERLAP)) {
                    tileLayerOverlap = (TileLayer)layer;
                }
            } else if (layer instanceof ObjectGroup) {
                if (layerName.equals(OBJECTGROUP_NAME_OBJECTS)) {
                    objectGroupObjects = (ObjectGroup)layer;
                } else if (layerName.equals(OBJECTGROUP_NAME_TRIGGERS)) {
                    objectGroupTriggers = (ObjectGroup)layer;
                }
            }
        }
    }

    public void write(String filename) throws IOException {
        Layer layerPlayfield = getLayer(tileLayerPlayfield);
        Layer layerNear = getLayer(tileLayerNear);
        Layer layerFar = getLayer(tileLayerFar);
        Layer layerOverlap = getLayer(tileLayerOverlap);

        layerPlayfield.useRawMapValues = true;
        layerNear.useRawMapValues = false;
        layerFar.useRawMapValues = false;
        layerOverlap.useRawMapValues = false;

        byte[] layerPlayfieldBytes = getLayerBytes(layerPlayfield);
        byte[] layerNearBytes = getLayerBytes(layerNear);
        byte[] layerFarBytes = getLayerBytes(layerFar);
        byte[] layerOverlapBytes = getLayerBytes(layerOverlap);
        byte[] objectsBytes = getObjectsBytes(objectGroupObjects);

        try (DataOutputStream stream = new DataOutputStream(new FileOutputStream(filename))) {
            stream.writeBytes(HEADER_FILE_SIG);
            stream.writeByte(0x00);
            // XXX: Hard coded: In the objects layer, look for an object named "PlayerStart"
            stream.writeInt(FixedPointUtils.toFixed(120.0f));
            // XXX: Hard coded
            stream.writeInt(FixedPointUtils.toFixed(300.0f));
            stream.writeInt((layerPlayfieldBytes != null) ? layerPlayfieldBytes.length : 0);
            stream.writeInt((layerNearBytes != null) ? layerNearBytes.length : 0);
            stream.writeInt((layerFarBytes != null) ? layerFarBytes.length : 0);
            stream.writeInt((layerOverlapBytes != null) ? layerOverlapBytes.length : 0);
            stream.writeInt((objectsBytes != null) ? objectsBytes.length : 0);

            if (layerPlayfieldBytes != null) {
                System.out.println(String.format("Writing playfield layer at offset 0x%08X", stream.size()));
                stream.write(layerPlayfieldBytes);
            }

            if (layerNearBytes != null) {
                System.out.println(String.format("Writing near layer at offset 0x%08X", stream.size()));
                stream.write(layerNearBytes);
            }

            if (layerFarBytes != null) {
                System.out.println(String.format("Writing far layer at offset 0x%08X", stream.size()));
                stream.write(layerFarBytes);
            }

            if (layerOverlapBytes != null) {
                System.out.println(String.format("Writing overlap layer at offset 0x%08X", stream.size()));
                stream.write(layerOverlapBytes);
            }

            if (objectsBytes != null) {
                System.out.println(String.format("Writing objects at offset 0x%08X", stream.size()));
                stream.write(objectsBytes);
            }
        }
    }

    private byte[] getLayerBytes(Layer layer) throws IOException {
        if (layer == null) {
            return null;
        }

        ByteArrayOutputStream outputStream = new ByteArrayOutputStream();

        try (DataOutputStream stream = new DataOutputStream(outputStream)) {
            BufferedImage tileSetImage = ImageUtils.getTileSetImage(layer.referencedTiles);
            BufferedImage indexedImage = ImageUtils.convertToIndexed(tileSetImage, layer.bpp);
            TileConverter tileConverter = new TileConverter(indexedImage);

            byte[] mapBytes = getMapBytes(layer);
            byte[] cgBytes = tileConverter.toTilesByteArray();
            byte[] paletteBytes = tileConverter.toPaletteByteArray();

            // For debugging
            // ImageIO.write(tileSetImage, "PNG", new File(String.format("aaa_%s.png", layer.tileLayer.getName())));
            // ImageIO.write(indexedImage, "PNG", new File(String.format("aaa_indexed_%s.png", layer.tileLayer.getName())));
            // Files.write(Paths.get(String.format("aaa_%s.bytes", tileLayer.getName())), paletteBytes);

            int mapByteSize = mapBytes.length;
            int cgByteSize = cgBytes.length;
            int paletteCount = (layer.bpp == 4) ? 16 : 256;
            // LayerFlags flags = LayerFlags.LAYER_FLAG_NONE;

            System.out.println(String.format("Header: %s", layer.tileLayer.getName()));
            System.out.println(String.format("Header: width: %d", layer.width));
            System.out.println(String.format("Header: height: %d", layer.height));
            System.out.println(String.format("Header: mapByteSize: %d", mapByteSize));
            System.out.println(String.format("Header: cgByteSize: %d", cgByteSize));
            System.out.println(String.format("Header: paletteCount: %d", paletteCount));
            // System.out.println(String.format("Header: flags: %d", flags.value));

            stream.writeShort(layer.width);
            stream.writeShort(layer.height);
            stream.writeInt(mapByteSize);
            stream.writeInt(cgByteSize);
            stream.writeShort(paletteCount);
            stream.writeInt(0xBEEFCAFE);
            // stream.writeInt(flags.value);

            stream.write(mapBytes);
            stream.write(cgBytes);
            stream.write(paletteBytes);
        }

        return outputStream.toByteArray();
    }

    private byte[] getObjectsBytes(ObjectGroup objectGroup) throws IOException {
        if (objectGroup == null) {
            return null;
        }

        ByteArrayOutputStream outputStream = new ByteArrayOutputStream();

        try (DataOutputStream stream = new DataOutputStream(outputStream)) {
            for (MapObject mapObject : objectGroup.getObjects()) {
                String mapObjectName = mapObject.getName().trim().toUpperCase();
                int x = (int)mapObject.getBounds().getX();
                int y = (int)mapObject.getBounds().getY();
                int width = (int)mapObject.getBounds().getWidth();
                int height = (int)mapObject.getBounds().getHeight();

                if (objectLevelObjects.containsKey(mapObjectName)) {
                    LevelObject levelObject = objectLevelObjects.get(mapObjectName);

                    stream.writeShort(levelObject.id);
                    // XXX: Flags
                    stream.writeInt(0xBEEFCAFE);
                    stream.writeInt(FixedPointUtils.toFixed(x + (width / 2)));
                    stream.writeInt(FixedPointUtils.toFixed(y - height));

                    System.out.println(String.format("Adding %s at (%d,%d)", mapObjectName, x, y));
                }
            }
        }

        return outputStream.toByteArray();
    }

    private static boolean getTileBooleanProperty(Tile tile, TileProperty tileProperty) {
        Properties properties = tile.getProperties();

        switch (tileProperty) {
        case COLLISION:
            return Boolean.parseBoolean(properties.getProperty(TILE_PROP_NAME_COLLISION,
                                                               TILE_PROP_DEFAULT_VALUE_COLLISION));
        case INSTANT_KILL:
            return Boolean.parseBoolean(properties.getProperty(TILE_PROP_NAME_INSTANT_KILL,
                                                               TILE_PROP_DEFAULT_VALUE_INSTANT_KILL));
        default:
            throw new IllegalArgumentException("Property is not an boolean type");
        }
    }

    private static int getLayerIntegerProperty(TileLayer tileLayer, LayerProperty layerProperty) {
        Properties properties = tileLayer.getProperties();

        switch (layerProperty) {
        case BPP:
            return Integer.parseInt(properties.getProperty(LAYER_PROP_NAME_BPP,
                                                           LAYER_PROP_DEFAULT_VALUE_BPP));
        case CROP_WIDTH:
            return Integer.parseInt(properties.getProperty(LAYER_PROP_NAME_CROP_WIDTH,
                                                           LAYER_PROP_DEFAULT_VALUE_CROP_WIDTH));
        case CROP_HEIGHT:
            return Integer.parseInt(properties.getProperty(LAYER_PROP_NAME_CROP_HEIGHT,
                                                           LAYER_PROP_DEFAULT_VALUE_CROP_HEIGHT));
        default:
            throw new IllegalArgumentException("Property is not an integer type");
        }
    }

    private Layer getLayer(TileLayer tileLayer) {
        if (tileLayer == null) {
            return null;
        }

        Layer layer = new Layer();

        layer.tileLayer = tileLayer;
        layer.bpp = getLayerIntegerProperty(tileLayer, LayerProperty.BPP);
        layer.cropWidth = getLayerIntegerProperty(tileLayer, LayerProperty.CROP_WIDTH);
        layer.cropHeight = getLayerIntegerProperty(tileLayer, LayerProperty.CROP_HEIGHT);
        layer.width = (layer.cropWidth < 0) ? tileLayer.getWidth() : layer.cropWidth;
        layer.height = (layer.cropHeight < 0) ? tileLayer.getHeight() : layer.cropHeight;
        layer.referencedTiles = new ArrayList<>();

        Set<Integer> tileIdHashSet = new HashSet<>();

        for (int y = 0; y < layer.height; y++) {
            for (int x = 0; x < layer.width; x++) {
                Tile tile = tileLayer.getTileAt(x, y);

                if ((tile != null) && !tileIdHashSet.contains(tile.getId())) {
                    tileIdHashSet.add(tile.getId());

                    layer.referencedTiles.add(tile);

                    System.out.println(String.format("Adding %d", tile.getId()));
                }
            }
        }

        return layer;
    }

    private byte[] getMapBytes(Layer layer) throws IOException {
        TileLayer tileLayer = layer.tileLayer;

        int width = layer.width;
        int height = layer.height;

        System.out.println(String.format("Cropping at (%dx%d)", width, height));

        MapValue[] mapValues = new MapValue[width * height];

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                Tile tile = tileLayer.getTileAt(x, y);

                MapValue mapValue = new MapValue();
                mapValue.pnd = calculatePNDValue(layer, x, y);

                if (layer.useRawMapValues && (tile != null)) {
                    int pixelX = x * tile.getWidth();
                    int pixelY = y * tile.getHeight();

                    // System.out.println(String.format("pixel(%d,%d)", pixelX, pixelY));

                    MapObject mapObject = objectGroupTriggers.getObjectAt(pixelX, pixelY);

                    mapValue.tileId = tile.getId();

                    if (getTileBooleanProperty(tile, TileProperty.COLLISION)) {
                        mapValue.flags.add(MapValueFlags.MAP_VALUE_FLAG_COLLISION);
                    }

                    if (getTileBooleanProperty(tile, TileProperty.INSTANT_KILL)) {
                        mapValue.flags.add(MapValueFlags.MAP_VALUE_FLAG_INSTANT_KILL);
                    }

                    if (mapObject != null) {
                        String mapObjectName = mapObject.getName().trim().toUpperCase();

                        if (triggerLevelObjects.containsKey(mapObjectName)) {
                            LevelObject levelObject = triggerLevelObjects.get(mapObjectName);

                            mapValue.flags.add(MapValueFlags.MAP_VALUE_FLAG_TRIGGER);

                            mapValue.triggerIndex = levelObject.id;

                            // System.out.println(String.format("[1;31m(%d,%d) -> %s[m", x, y, mapObjectName));
                        }
                    }
                }

                mapValues[x + (width * y)] = mapValue;

                int flagsValue = EnumUtils.encode(mapValue.flags);

                if (mapValue.flags.contains(MapValueFlags.MAP_VALUE_FLAG_TRIGGER)) {
                    System.out.print("^");
                } else if (flagsValue != 0) {
                    System.out.print(flagsValue);
                } else {
                    System.out.print(".");
                }
            }
            System.out.println("");
        }

        ByteArrayOutputStream outputStream = new ByteArrayOutputStream();

        try (DataOutputStream stream = new DataOutputStream(outputStream)) {
            for (MapValue mapValue : mapValues) {
                stream.writeShort(mapValue.pnd);

                if (layer.useRawMapValues) {
                    int value = EnumUtils.encode(mapValue.flags);

                    stream.writeByte(value);
                    stream.writeByte(mapValue.triggerIndex);
                }
            }
        }

        return outputStream.toByteArray();
    }

    private static short calculatePNDValue(Layer layer, int x, int y) {
        TileLayer tileLayer = layer.tileLayer;
        Tile tile = tileLayer.getTileAt(x, y);
        int flags = tileLayer.getFlagsAt(x, y);

        int rawTileId = 0;

        if (tile != null) {
            rawTileId = layer.referencedTiles.indexOf(tile);
        }

        short tileId = (short)(rawTileId & 0x03FF);

        boolean horizontallyFlipped = (flags & (int)TMXMapReader.FLIPPED_HORIZONTALLY_FLAG) != 0;
        boolean verticallyFlipped = (flags & (int)TMXMapReader.FLIPPED_VERTICALLY_FLAG) != 0;
        short flipBits = 0x0000;

        if (horizontallyFlipped) {
            flipBits |= 0x0400;
        }

        if (verticallyFlipped) {
            flipBits |= 0x0800;
        }

        if (layer.bpp == 8) {
            tileId <<= 1;
        }

        short value = (short)(tileId | flipBits);

        return value;
    }
}
