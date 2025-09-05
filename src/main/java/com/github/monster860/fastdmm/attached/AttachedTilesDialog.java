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
import com.github.monster860.fastdmm.objtree.ObjectTree;

/**
 * Minimal UI to edit attached tile rules for the currently selected instance type/icon.
 */
public class AttachedTilesDialog extends JDialog {
    private final AttachedTileService service;
    private final String rulesKey;
    private final FastDMM editor;
    // One selected type per direction (or none)
    private String northType;
    private String southType;
    private String eastType;
    private String westType;
    private JButton northBtn = new JButton();
    private JButton southBtn = new JButton();
    private JButton eastBtn = new JButton();
    private JButton westBtn = new JButton();
    private JLabel centerPreview = new JLabel();
    private JLabel titleLabel = new JLabel();
    private JButton lastFocusedBtn;

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
    // Title row with the key
    titleLabel.setText("Attached Tiles for: " + rulesKey);
    titleLabel.setBorder(BorderFactory.createEmptyBorder(6, 8, 6, 8));
    content.add(titleLabel, BorderLayout.NORTH);

    // Center layout: N on top center; W, center preview, E; S bottom center
    JPanel center = new JPanel(new GridBagLayout());
    GridBagConstraints gbc = new GridBagConstraints();
    gbc.insets = new Insets(6,6,6,6);
    gbc.gridx = 1; gbc.gridy = 0; // top center
    center.add(panel("North (N)", northBtn), gbc);

    gbc.gridx = 0; gbc.gridy = 1; // middle left
    center.add(panel("West (W)", westBtn), gbc);

    // Center tile preview
    gbc.gridx = 1; gbc.gridy = 1;
    JPanel previewPanel = new JPanel(new BorderLayout());
    JLabel centerLbl = new JLabel("Center Tile", SwingConstants.CENTER);
    previewPanel.add(centerLbl, BorderLayout.NORTH);
    centerPreview.setHorizontalAlignment(SwingConstants.CENTER);
    centerPreview.setPreferredSize(new Dimension(160,160));
    previewPanel.add(centerPreview, BorderLayout.CENTER);
    center.add(previewPanel, gbc);

    gbc.gridx = 2; gbc.gridy = 1; // middle right
    center.add(panel("East (E)", eastBtn), gbc);

    gbc.gridx = 1; gbc.gridy = 2; // bottom center
    center.add(panel("South (S)", southBtn), gbc);

    content.add(center, BorderLayout.CENTER);

        JPanel buttons = new JPanel(new FlowLayout(FlowLayout.LEFT));
        JButton save = new JButton(new AbstractAction("Save") {
            @Override public void actionPerformed(ActionEvent e) { save(); }
        });
        JButton revert = new JButton(new AbstractAction("Revert") {
            @Override public void actionPerformed(ActionEvent e) { load(); }
        });
        JButton clearAll = new JButton(new AbstractAction("Clear All") {
            @Override public void actionPerformed(ActionEvent e) { clearAll(); }
        });
        JButton deleteRule = new JButton(new AbstractAction("Delete Rule") {
            @Override public void actionPerformed(ActionEvent e) { clearRule(); dispose(); }
        });
        buttons.add(save);
        buttons.add(revert);
        buttons.add(clearAll);
        buttons.add(deleteRule);
    content.add(buttons, BorderLayout.SOUTH);

    setContentPane(content);

        // Track which direction button was last interacted with
        FocusListener fl = new FocusAdapter() {
            @Override public void focusGained(java.awt.event.FocusEvent e) {
                if (e.getComponent() instanceof JButton) {
                    lastFocusedBtn = (JButton) e.getComponent();
                }
            }
        };
        northBtn.addFocusListener(fl);
        southBtn.addFocusListener(fl);
        eastBtn.addFocusListener(fl);
        westBtn.addFocusListener(fl);

        // Clicking a direction button opens picker directly too
    northBtn.addActionListener(e -> { lastFocusedBtn = northBtn; onPick(); });
    southBtn.addActionListener(e -> { lastFocusedBtn = southBtn; onPick(); });
    eastBtn.addActionListener(e -> { lastFocusedBtn = eastBtn; onPick(); });
    westBtn.addActionListener(e -> { lastFocusedBtn = westBtn; onPick(); });

    // Initialize the center preview from rulesKey
    refreshCenterPreview();
    }

    private JPanel panel(String title, JButton btn) {
        JPanel p = new JPanel(new BorderLayout());
        JLabel lbl = new JLabel(title);
        p.add(lbl, BorderLayout.NORTH);
    btn.setHorizontalAlignment(SwingConstants.CENTER);
    // Larger buttons to showcase bigger icons
    btn.setPreferredSize(new Dimension(160, 160));
        p.add(btn, BorderLayout.CENTER);
        return p;
    }

    private void load() {
        Map<String, java.util.List<String>> data = service.getRulesForKey(rulesKey);
        northType = firstOrNull(data.get("N"));
        southType = firstOrNull(data.get("S"));
        eastType = firstOrNull(data.get("E"));
        westType = firstOrNull(data.get("W"));
        refreshButtonIcons();
    refreshCenterPreview();
    }

    private void save() {
        Map<String, java.util.List<String>> data = new HashMap<>();
        data.put("N", toList(northType));
        data.put("S", toList(southType));
        data.put("E", toList(eastType));
        data.put("W", toList(westType));
        service.setRulesForKey(rulesKey, data);
    }

    private void clearRule() {
        service.clearRulesForKey(rulesKey);
    }

    private void clearAll() {
        northType = southType = eastType = westType = null;
        refreshButtonIcons();
    }

    private java.util.List<String> toList(String typeOrNull) {
        java.util.List<String> out = new ArrayList<>();
        if (typeOrNull != null && !typeOrNull.trim().isEmpty()) out.add(typeOrNull.trim());
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
        String chosen = types.get(0); // single selection for a direction
        JButton target = focusedButton();
        if (target == null) target = northBtn; // default
        if (target == northBtn) northType = chosen;
        else if (target == southBtn) southType = chosen;
        else if (target == eastBtn) eastType = chosen;
        else if (target == westBtn) westType = chosen;
        refreshButtonIcons();
    }

    private JButton focusedButton() {
        if (northBtn.isFocusOwner()) return northBtn;
        if (southBtn.isFocusOwner()) return southBtn;
        if (eastBtn.isFocusOwner()) return eastBtn;
        if (westBtn.isFocusOwner()) return westBtn;
        if (lastFocusedBtn != null) return lastFocusedBtn;
        return null;
    }

    private void refreshButtonIcons() {
        setButtonIcon(northBtn, northType);
        setButtonIcon(southBtn, southType);
        setButtonIcon(eastBtn, eastType);
        setButtonIcon(westBtn, westType);
    }

    private void setButtonIcon(JButton btn, String typePath) {
        if (typePath == null || typePath.trim().isEmpty()) {
            btn.setIcon(null);
            btn.setText("(none)");
            return;
        }
        ObjectTree.Item item = editor.objTree.get(typePath);
        if (item == null) {
            btn.setIcon(null);
            btn.setText(typePath);
            return;
        }
        com.github.monster860.fastdmm.dmirender.DMI dmi = editor.getDmi(item.getIcon(), false);
        if (dmi != null) {
            String iconState = item.getIconState();
            com.github.monster860.fastdmm.dmirender.IconSubstate sub = dmi.getIconState(iconState).getSubstate(item.getDir());
            // Use a much larger icon preview (~96px or full icon height if smaller)
            int target = Math.max(128, dmi.height);
            btn.setIcon(sub.getScaledToHeight(target));
            btn.setText("");
        } else {
            btn.setIcon(null);
            btn.setText(item.parentlessName());
        }
        btn.setToolTipText(typePath);
    }

    private void refreshCenterPreview() {
        // rulesKey format: path#state
        String path = rulesKey;
        String icon = null, state = null;
        int idx = path.indexOf('#');
        if (idx >= 0) { icon = path.substring(0, idx); state = path.substring(idx+1); }
        if (icon == null || state == null) { centerPreview.setIcon(null); centerPreview.setText(""); return; }
        com.github.monster860.fastdmm.dmirender.DMI dmi = editor.getDmi(icon, false);
        if (dmi == null) { centerPreview.setIcon(null); centerPreview.setText(""); return; }
        try {
            com.github.monster860.fastdmm.dmirender.IconSubstate sub = dmi.getIconState(state).getSubstate(2);
            centerPreview.setIcon(sub.getScaledToHeight(Math.max(128, dmi.height)));
            centerPreview.setText("");
        } catch (Exception ex) {
            centerPreview.setIcon(null);
            centerPreview.setText("");
        }
        centerPreview.setToolTipText(rulesKey);
    }

    private String firstOrNull(java.util.List<String> list) {
        if (list == null || list.isEmpty()) return null;
        return list.get(0);
    }
}
