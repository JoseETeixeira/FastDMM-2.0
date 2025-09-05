package com.github.monster860.fastdmm.attached;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.*;

import com.github.monster860.fastdmm.FastDMM;
import com.github.monster860.fastdmm.dmmmap.Location;
import com.github.monster860.fastdmm.dmmmap.TileInstance;
import com.github.monster860.fastdmm.objtree.ObjInstance;

import org.json.JSONArray;
import org.json.JSONObject;

/**
 * Service to manage "Attached Tiles" placement rules and apply them during placement.
 * Rules file lives next to the DME as attached_tiles.json with structure:
 * {
 *   "version": 1,
 *   "rules": {
 *     "path/to/icon.dmi#state": { "N": ["/obj/foo"], "S": [], "E": [], "W": [] }
 *   }
 * }
 */
public class AttachedTileService {
    private final FastDMM editor;
    private File rulesFile;
    private JSONObject root;
    // No longer tracking recent bases; the direction tile equality rule is sufficient to avoid cross-overwrites.

    public AttachedTileService(FastDMM editor) {
        this.editor = editor;
        this.root = new JSONObject();
        JSONObject rules = new JSONObject();
        root.put("version", 1);
        root.put("rules", rules);
    }

    public void load(File dmeDir) {
        rulesFile = new File(dmeDir, "attached_tiles.json");
        if (!rulesFile.exists()) {
            save();
            return;
        }
        try (FileInputStream fis = new FileInputStream(rulesFile)) {
            byte[] data = new byte[(int) rulesFile.length()];
            int read = fis.read(data);
            if (read > 0) {
                String text = new String(data, 0, read, StandardCharsets.UTF_8);
                JSONObject parsed = new JSONObject(text);
                // Defensive defaults
                if (!parsed.has("version")) parsed.put("version", 1);
                if (!parsed.has("rules")) parsed.put("rules", new JSONObject());
                this.root = parsed;
            }
        } catch (Exception e) {
            e.printStackTrace();
            // Keep default empty root
        }
    }

    public void save() {
        if (rulesFile == null) return;
        try (FileOutputStream fos = new FileOutputStream(rulesFile)) {
            byte[] out = root.toString(2).getBytes(StandardCharsets.UTF_8);
            fos.write(out);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static String normalizeKey(ObjInstance obj) {
        String icon = obj.getIcon();
        String state = obj.getIconState();
        if (icon == null) icon = "";
        if (state == null) state = "";
        icon = icon.replace('\\', '/');
        return icon + "#" + state;
    }

    private JSONObject rulesRoot() {
        if (!root.has("rules")) root.put("rules", new JSONObject());
        return root.getJSONObject("rules");
    }

    public Map<String, List<String>> getRulesFor(ObjInstance obj) {
        return getRulesForKey(normalizeKey(obj));
    }

    public Map<String, List<String>> getRulesForKey(String key) {
        Map<String, List<String>> out = new HashMap<>();
        out.put("N", new ArrayList<>());
        out.put("S", new ArrayList<>());
        out.put("E", new ArrayList<>());
        out.put("W", new ArrayList<>());
        JSONObject rules = rulesRoot();
        if (!rules.has(key)) return out;
        JSONObject byDir = rules.getJSONObject(key);
        for (String dir : Arrays.asList("N","S","E","W")) {
            if (byDir.has(dir)) {
                JSONArray arr = byDir.getJSONArray(dir);
                for (int i = 0; i < arr.length(); i++) {
                    out.get(dir).add(arr.getString(i));
                }
            }
        }
        return out;
    }

    public void setRulesForKey(String key, Map<String, List<String>> byDir) {
        JSONObject rules = rulesRoot();
        JSONObject obj = new JSONObject();
        for (String dir : Arrays.asList("N","S","E","W")) {
            List<String> list = byDir.get(dir);
            if (list != null) obj.put(dir, new JSONArray(list));
            else obj.put(dir, new JSONArray());
        }
        rules.put(key, obj);
        save();
    }

    public void clearRulesForKey(String key) {
        JSONObject rules = rulesRoot();
        if (rules.has(key)) {
            rules.remove(key);
            save();
        }
    }

    public void applyAttachments(Location origin, ObjInstance base) {
        if (editor == null || editor.dmm == null) return;
        // Use a visited set to prevent infinite cycles: key by loc + rule key
        Set<String> visited = new HashSet<>();
        // Capture original context: the tile key at the initial origin and the original base type
        String rootOriginKey = editor.dmm.map.get(origin);
        String rootBaseType = base != null ? base.typeString() : null;
        applyAttachmentsInternal(origin, base, visited, 0, rootOriginKey, rootBaseType);
    }

    private void applyAttachmentsInternal(Location origin, ObjInstance base, Set<String> visited, int depth, String rootOriginKey, String rootBaseType) {
        if (editor == null || editor.dmm == null) return;
        // Cap depth defensively to avoid unbounded recursion in pathological rule graphs
        if (depth > 8) return;

        String ruleKey = normalizeKey(base);
        String visitKey = origin.x + "," + origin.y + "," + origin.z + "|" + ruleKey;
        if (!visited.add(visitKey)) return;

        Map<String, List<String>> byDir = getRulesForKey(ruleKey);
        if (byDir.isEmpty()) return;

        // Neighbor locations
        Map<String, Location> neighbors = new HashMap<>();
        neighbors.put("N", new Location(origin.x, origin.y + 1, origin.z));
        neighbors.put("S", new Location(origin.x, origin.y - 1, origin.z));
    // Note: UI orientation requires E/W to be mirrored relative to map coords
    neighbors.put("E", new Location(origin.x - 1, origin.y, origin.z));
    neighbors.put("W", new Location(origin.x + 1, origin.y, origin.z));

    // Tile key at current origin (not used for skip anymore; we compare against root)
    String originKey = editor.dmm.map.get(origin);

        for (Map.Entry<String, List<String>> entry : byDir.entrySet()) {
            String dir = entry.getKey();
            Location l = neighbors.get(dir);
            if (l == null) continue;
            if (l.x < editor.dmm.minX || l.x > editor.dmm.maxX || l.y < editor.dmm.minY || l.y > editor.dmm.maxY)
                continue;
            String neighborKey = editor.dmm.map.get(l);
            if (neighborKey == null) continue;
            TileInstance ti = editor.dmm.instances.get(neighborKey);
            if (ti == null) continue;
            // Directional skips (both per-level and root-level):
            // A) If neighbor tile content equals the current origin's tile, skip
            if (originKey != null && originKey.equals(neighborKey)) continue;
            // B) If neighbor tile contains the current base type, skip
            String currentBaseType = (base != null) ? base.typeString() : null;
            if (currentBaseType != null && !currentBaseType.isEmpty()) {
                boolean neighborHasCurrentBase = false;
                for (ObjInstance oi : ti.objs) {
                    if (oi != null && oi.istype(currentBaseType)) { neighborHasCurrentBase = true; break; }
                }
                if (neighborHasCurrentBase) continue;
            }
            // C) If neighbor tile content equals the original tile's content, skip
            if (rootOriginKey != null && rootOriginKey.equals(neighborKey)) continue;
            // D) If neighbor tile already contains the original base type being placed, skip
            if (rootBaseType != null && !rootBaseType.isEmpty()) {
                boolean neighborHasRootBase = false;
                for (ObjInstance oi : ti.objs) {
                    if (oi != null && oi.istype(rootBaseType)) { neighborHasRootBase = true; break; }
                }
                if (neighborHasRootBase) continue;
            }

            for (String typePath : entry.getValue()) {
                if (typePath == null || typePath.trim().isEmpty()) continue;
                // Already present?
                boolean exists = false;
                for (ObjInstance i : ti.objs) {
                    if (i != null && i.istype(typePath)) { exists = true; break; }
                }
                if (exists) continue;
                // Get template
                com.github.monster860.fastdmm.objtree.ObjectTree.Item item = editor.objTree.get(typePath);
                if (item == null) continue;
                // Turf/Area: allow, but skip exact same type duplicates
                boolean isTurf = item.path.startsWith("/turf");
                boolean isArea = item.path.startsWith("/area");
                if (isTurf || isArea) {
                    boolean sameTypeExists = false;
                    for (ObjInstance i : ti.objs) {
                        if (i == null) continue;
                        if (i.istype(item.path)) { sameTypeExists = true; break; }
                    }
                    if (sameTypeExists) continue;
                }
                String newKey = ti.addObject(item);
                editor.dmm.putMap(l, newKey);

                // Recurse: apply attachments of the item we just placed, preserving root context
                applyAttachmentsInternal(l, item, visited, depth + 1, rootOriginKey, rootBaseType);
            }
        }
    }
}
