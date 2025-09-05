package com.github.monster860.fastdmm.attached;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.FocusAdapter;
import java.awt.event.FocusListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.util.*;

import com.github.monster860.fastdmm.FastDMM;

/**
 * Minimal UI to edit attached tile rules for the currently selected instance type/icon.
 */
public class AttachedTilesDialog extends JDialog {
    private final AttachedTileService service;
    private final String rulesKey;
    private final JTextArea north = new JTextArea(5, 24);
    private final JTextArea south = new JTextArea(5, 24);
    private final JTextArea east = new JTextArea(5, 24);
    private final JTextArea west = new JTextArea(5, 24);
    private final FastDMM editor;
    private JTextArea lastFocused;

    public AttachedTilesDialog(Frame owner, AttachedTileService service, String rulesKey) {
        super(owner, "Attached Tiles", true);
        this.service = service;
        this.rulesKey = rulesKey;
        this.editor = (owner instanceof FastDMM) ? (FastDMM) owner : null;
        buildUI();
        load();
        pack();
        setLocationRelativeTo(owner);
    }

    private void buildUI() {
        JPanel content = new JPanel(new BorderLayout(8,8));
    JPanel center = new JPanel(new GridLayout(2,2,8,8));
    center.add(panel("North (N)", north));
    center.add(panel("South (S)", south));
    center.add(panel("East (E)", east));
    center.add(panel("West (W)", west));
        content.add(center, BorderLayout.CENTER);

        JLabel hint = new JLabel("Enter one type path per line, e.g. /obj/door");
        content.add(hint, BorderLayout.NORTH);

    JPanel buttons = new JPanel(new FlowLayout(FlowLayout.RIGHT));
        JButton pick = new JButton(new AbstractAction("Pick...") {
            @Override public void actionPerformed(ActionEvent e) { onPick(); }
        });
        JButton save = new JButton(new AbstractAction("Save") {
            @Override public void actionPerformed(ActionEvent e) { save(); dispose(); }
        });
        JButton clear = new JButton(new AbstractAction("Clear Rule") {
            @Override public void actionPerformed(ActionEvent e) { clearRule(); dispose(); }
        });
        JButton cancel = new JButton(new AbstractAction("Close") {
            @Override public void actionPerformed(ActionEvent e) { dispose(); }
        });
        buttons.add(pick);
        buttons.add(clear);
        buttons.add(save);
    buttons.add(cancel);
    content.add(buttons, BorderLayout.SOUTH);

    setContentPane(content);

        // Track focus so the picker knows where to insert
        FocusListener fl = new FocusAdapter() {
            @Override public void focusGained(java.awt.event.FocusEvent e) {
                if (e.getComponent() instanceof JTextArea) {
                    lastFocused = (JTextArea) e.getComponent();
                }
            }
        };
        north.addFocusListener(fl);
        south.addFocusListener(fl);
        east.addFocusListener(fl);
        west.addFocusListener(fl);
    }

    private JPanel panel(String title, JTextArea ta) {
        JPanel p = new JPanel(new BorderLayout());
        JLabel lbl = new JLabel(title);
        p.add(lbl, BorderLayout.NORTH);
        p.add(new JScrollPane(ta), BorderLayout.CENTER);

        // Clicking the label or panel sets focus to the textarea so the picker targets the right field
        MouseAdapter focusSetter = new MouseAdapter() {
            @Override public void mouseClicked(MouseEvent e) {
                ta.requestFocusInWindow();
            }
        };
        lbl.addMouseListener(focusSetter);
        p.addMouseListener(focusSetter);
        return p;
    }

    private void load() {
        Map<String, java.util.List<String>> data = service.getRulesForKey(rulesKey);
        north.setText(String.join("\n", data.get("N")));
        south.setText(String.join("\n", data.get("S")));
        east.setText(String.join("\n", data.get("E")));
        west.setText(String.join("\n", data.get("W")));
    }

    private void save() {
        Map<String, java.util.List<String>> data = new HashMap<>();
        data.put("N", toList(north.getText()));
        data.put("S", toList(south.getText()));
        data.put("E", toList(east.getText()));
        data.put("W", toList(west.getText()));
        service.setRulesForKey(rulesKey, data);
    }

    private void clearRule() {
        service.clearRulesForKey(rulesKey);
    }

    private java.util.List<String> toList(String text) {
        java.util.List<String> out = new ArrayList<>();
        for (String line : text.split("(\r\n|\r|\n)")) {
            String s = line.trim();
            if (!s.isEmpty()) out.add(s);
        }
        return out;
    }

    private void onPick() {
        if (editor == null) {
            JOptionPane.showMessageDialog(this, "Editor not available for picking.", "Error", JOptionPane.ERROR_MESSAGE);
            return;
        }
        ObjectPickerDialog dlg = new ObjectPickerDialog((Frame) SwingUtilities.getWindowAncestor(this), editor);
        dlg.setLocationRelativeTo(this);
        dlg.setVisible(true);
        java.util.List<String> types = dlg.getSelectedTypePaths();
        if (types == null || types.isEmpty()) return;
    JTextArea target = focusedArea();
        if (target == null) target = north; // default
        appendLines(target, types);
    }

    private JTextArea focusedArea() {
    if (north.isFocusOwner()) return north;
    if (south.isFocusOwner()) return south;
    if (east.isFocusOwner()) return east;
    if (west.isFocusOwner()) return west;
    if (lastFocused != null) return lastFocused;
    return null;
    }

    private void appendLines(JTextArea area, java.util.List<String> lines) {
        String existing = area.getText();
        StringBuilder sb = new StringBuilder(existing == null ? "" : existing.trim());
        for (String l : lines) {
            if (sb.length() > 0) sb.append(System.lineSeparator());
            sb.append(l);
        }
        area.setText(sb.toString());
    }
}
