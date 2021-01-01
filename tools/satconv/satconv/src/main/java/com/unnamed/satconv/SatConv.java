package com.unnamed.satconv;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;

public class SatConv {
    private static final String OBJECTS_FILEPATH  = "objects.txt";
    private static final String TRIGGERS_FILEPATH = "triggers.txt";

    private class SprDef {
        String name;
        String filename;
    }

    private class LevelObjectWriterParams {
        public String filename;
        public String headerName;
        public String prefix;
        public String objectType;
    }

    public static void main(String[] args) {
        if (args.length == 0) {
            System.out.println("Usage: satconv [assets.txt]");
            return;
        }

        try {
            ArrayList<SprDef> sprDefs = new ArrayList<>();
            LevelObjects objectLevelObjects = populateLevelObjectList(OBJECTS_FILEPATH);
            LevelObjects triggerLevelObjects = populateLevelObjectList(TRIGGERS_FILEPATH);

            File listFile = new File(args[0]);
            try (BufferedReader reader = new BufferedReader(new FileReader(listFile))) {
                String line = reader.readLine();

                while (line != null) {
                    System.out.println(line);

                    // Sprite folder
                    if (line.charAt(0) == 's') {
                        File folder = new File(line.substring(2));
                        File[] fileList = folder.listFiles();

                        if (fileList.length > 0) {
                            Arrays.sort(fileList);

                            String sprName = line.substring(2).toUpperCase();
                            String sprFilename = sprName.substring(0, Math.min(sprName.length(), 8)) + ".SPR";

                            SpriteConverter spriteConverter = new SpriteConverter();

                            for (int i = 0; i < fileList.length; i++) {
                                spriteConverter.addImage(fileList[i]);
                            }

                            spriteConverter.write(sprFilename);

                            SprDef sprDef = new SatConv().new SprDef();
                            sprDef.name = sprName;
                            sprDef.filename = sprFilename;

                            sprDefs.add(sprDef);
                        }
                    } else if (line.charAt(0) == 'l') {
                        String tmxName = line.substring(1, line.indexOf('.')).toUpperCase().trim();
                        String tmxFilename = line.substring(1).trim();
                        String levelFilename = tmxName.substring(0, Math.min(tmxName.length(), 8)) + ".LVL";

                        LevelConverter levelConverter = new LevelConverter(objectLevelObjects,
                                                                           triggerLevelObjects,
                                                                           tmxFilename);

                        levelConverter.write(levelFilename);
                    } else if (line.charAt(0) == 'f') {
                        File folder = new File(line.substring(2));
                        File[] fileList = folder.listFiles();

                        if (fileList.length > 0) {
                            Arrays.sort(fileList);

                            String fonName = line.substring(2).toUpperCase();
                            String fonFilename = fonName.substring(0, Math.min(fonName.length(), 8)) + ".FON";

                            FontConverter fontConverter = new FontConverter();

                            for (int i = 0; i < fileList.length; i++) {
                                if (fileList[i].isFile()) {
                                    fontConverter.addImage(fileList[i]);
                                }
                            }

                            fontConverter.write(fonFilename);
                        }
                    }

                    line = reader.readLine();
                }
            }

            writeSprsCFile(objectLevelObjects);
            writeSprsHeaderFile(sprDefs);

            LevelObjectWriterParams objectsHeaderParams = new SatConv().new LevelObjectWriterParams();
            objectsHeaderParams.filename = "objects.h";
            objectsHeaderParams.headerName = "OBJECTS";
            objectsHeaderParams.prefix = "OBJECT_ID";
            objectsHeaderParams.objectType = "object_id";

            writeLevelObjectsHeaderFile(objectLevelObjects, objectsHeaderParams);

            LevelObjectWriterParams triggersHeaderParams = new SatConv().new LevelObjectWriterParams();
            triggersHeaderParams.filename = "triggers.h";
            triggersHeaderParams.headerName = "TRIGGERS";
            triggersHeaderParams.prefix = "TRIGGER_ID";
            triggersHeaderParams.objectType = "trigger_id";

            writeLevelObjectsHeaderFile(triggerLevelObjects, triggersHeaderParams);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static LevelObjects populateLevelObjectList(String filePath) throws IOException {
        LevelObjects levelObjectList = new LevelObjects();

        File objectsFile = new File(filePath);

        if (!objectsFile.exists()) {
            // XXX: Return an error?
            return levelObjectList;
        }

        int id = 0;

        try (BufferedReader reader = new BufferedReader(new FileReader(filePath))) {
            while (true) {
                String line = reader.readLine();

                if (line == null) {
                    break;
                }

                String trimmedLine = line.trim().toUpperCase();

                if (!trimmedLine.isEmpty()) {
                    LevelObject levelObject = new LevelObject();
                    levelObject.id = id;
                    levelObject.name = trimmedLine;

                    levelObjectList.put(trimmedLine, levelObject);

                    id++;
                }
            }
        }

        return levelObjectList;
    }

    private static void writeLevelObjectsHeaderFile(LevelObjects levelObjects, LevelObjectWriterParams params) {
        try {
            try (BufferedWriter writer = new BufferedWriter(new FileWriter(params.filename.trim()))) {
                String headerMacroName = String.format("%s_H", params.headerName.trim().toUpperCase());

                writer.write("/* AUTO-GENERATED */");
                writer.newLine();
                writer.newLine();
                writer.write(String.format("#ifndef %s", headerMacroName));
                writer.newLine();
                writer.write(String.format("#define %s", headerMacroName));
                writer.newLine();
                writer.newLine();

                // StringBuilder sb = new StringBuilder();

                writer.write(String.format("typedef enum %s {", params.objectType.trim()));
                writer.newLine();
                for (LevelObject levelObject : levelObjects.values()) {
                    writer.write(String.format("        %s_%-16s = %2d,",
                                               params.prefix.trim().toUpperCase(),
                                               levelObject.name,
                                               levelObject.id));
                    writer.newLine();
                }
                writer.write(String.format("        %s_%-16s = %2d",
                                           params.prefix.trim().toUpperCase(),
                                           "INVALID",
                                           -1));
                writer.newLine();

                writer.write(String.format("} %s_t;", params.objectType.trim()));
                writer.newLine();
                writer.newLine();

                writer.write(String.format("#endif /* !%s */", headerMacroName));
                writer.newLine();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static void writeSprsHeaderFile(ArrayList<SprDef> sprDefs) {
        try {
            try (BufferedWriter writer = new BufferedWriter(new FileWriter("sprs.h"))) {
                writer.write("/* AUTO-GENERATED */");
                writer.newLine();
                writer.newLine();
                writer.write("#ifndef ASSETS_SPRS_H");
                writer.newLine();
                writer.write("#define ASSETS_SPRS_H");
                writer.newLine();
                writer.newLine();

                writer.write("#define ASSET_SPR_FILENAME(id) (sprs_filename_mappings[(id)])");
                writer.newLine();
                writer.newLine();

                // StringBuilder sb = new StringBuilder();

                for (SprDef sprDef : sprDefs) {
                    writer.write(String.format("#define ASSET_SPR_%-16s\"%s\"", sprDef.name, sprDef.filename));
                    writer.newLine();
                }

                if (sprDefs.size() > 0) {
                    writer.newLine();
                }

                writer.write("extern const char *sprs_filename_mappings[];");
                writer.newLine();
                writer.newLine();

                writer.write("#endif /* !ASSETS_SPRS_H */");
                writer.newLine();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static void writeSprsCFile(LevelObjects levelObjects) {
        try {
            int maxLevelObjectId = -1;
            for (LevelObject levelObject : levelObjects.values()) {
                maxLevelObjectId = Math.max(maxLevelObjectId, levelObject.id);
            }

            String[] levelObjectNames = new String[maxLevelObjectId + 1];

            for (LevelObject levelObject : levelObjects.values()) {
                levelObjectNames[levelObject.id] = levelObject.name;
            }

            try (BufferedWriter writer = new BufferedWriter(new FileWriter("sprs.c"))) {
                writer.write("/* AUTO-GENERATED */");
                writer.newLine();
                writer.newLine();
                writer.write("#include \"sprs.h\"");
                writer.newLine();
                writer.newLine();

                writer.write("const char *sprs_filename_mappings[] = {");
                writer.newLine();
                for (int i = 0; i < levelObjectNames.length; i++) {
                    if (levelObjectNames[i] == null) {
                        writer.write(String.format("        NULL,"));
                    } else {
                        writer.write(String.format("        ASSET_SPR_%s,", levelObjectNames[i]));
                    }
                    writer.newLine();
                }

                writer.write("};");
                writer.newLine();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
