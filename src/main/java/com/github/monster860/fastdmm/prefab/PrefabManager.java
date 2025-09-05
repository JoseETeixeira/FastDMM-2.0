package com.github.monster860.fastdmm.prefab;

import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.json.JSONArray;
import org.json.JSONObject;
import org.json.JSONTokener;

import com.github.monster860.fastdmm.dmmmap.DMM;
import com.github.monster860.fastdmm.dmmmap.Location;
import com.github.monster860.fastdmm.dmmmap.TileInstance;
import com.github.monster860.fastdmm.objtree.ObjectTree;

/** Manages loading/saving prefabs for a given project (per DME folder). */
public class PrefabManager {
    private final DMM dmm;
    private final ObjectTree objTree;
    private final Path prefabPath;

    public final Map<String, Prefab> prefabs = new HashMap<>();

    public PrefabManager(DMM dmm, ObjectTree objTree, Path dmeDir) {
        this.dmm = dmm;
        this.objTree = objTree;
        this.prefabPath = dmeDir.resolve("prefabs.json");
    }

    public void load() {
        prefabs.clear();
        if (!Files.exists(prefabPath)) return;
        try (FileReader fr = new FileReader(prefabPath.toFile())) {
            JSONObject root = new JSONObject(new JSONTokener(fr));
            JSONArray arr = root.optJSONArray("prefabs");
            if (arr == null) return;
            for (int i = 0; i < arr.length(); i++) {
                JSONObject pj = arr.getJSONObject(i);
                String name = pj.getString("name");
                int w = pj.getInt("width");
                int h = pj.getInt("height");
                Prefab p = new Prefab(name, w, h);
                JSONObject tiles = pj.getJSONObject("tiles");
                for (String key : tiles.keySet()) {
                    String[] parts = key.split(",");
                    int x = Integer.parseInt(parts[0]);
                    int y = Integer.parseInt(parts[1]);
                    String tileStr = tiles.getString(key);
                    TileInstance ti = TileInstance.fromString(tileStr, objTree, dmm);
                    if (ti != null) p.tiles.put(new Location(x, y, 1), ti);
                }
                prefabs.put(name, p);
                // Rebuild preview (needs editor context; deferred until attach)
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    // Called after FastDMM ensures interface_dmi etc. so previews can be built
    public void rebuildPreviews(com.github.monster860.fastdmm.FastDMM editor) {
        for (Prefab p : prefabs.values()) {
            try { p.buildPreview(editor); } catch (Exception ignored) {}
        }
    }

    public void save() {
        try (FileWriter fw = new FileWriter(prefabPath.toFile())) {
            JSONArray arr = new JSONArray();
            for (Prefab p : prefabs.values()) {
                JSONObject pj = new JSONObject();
                pj.put("name", p.name);
                pj.put("width", p.width);
                pj.put("height", p.height);
                JSONObject tiles = new JSONObject();
                for (Map.Entry<Location, TileInstance> e : p.tiles.entrySet()) {
                    tiles.put(e.getKey().x + "," + e.getKey().y, e.getValue().toString());
                }
                pj.put("tiles", tiles);
                arr.put(pj);
            }
            JSONObject root = new JSONObject();
            root.put("prefabs", arr);
            fw.write(root.toString());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public Prefab create(String name, int w, int h) {
        Prefab p = new Prefab(name, w, h);
        prefabs.put(name, p);
        save();
        return p;
    }
}
