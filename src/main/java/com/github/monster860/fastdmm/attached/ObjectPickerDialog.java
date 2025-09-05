package com.github.monster860.fastdmm.attached;

import com.github.monster860.fastdmm.FastDMM;
import com.github.monster860.fastdmm.editing.ui.ObjectTreeRenderer;
import com.github.monster860.fastdmm.objtree.ObjectTree;
import com.github.monster860.fastdmm.objtree.ObjectTree.Item;

import javax.swing.*;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.tree.TreePath;
import javax.swing.tree.TreeSelectionModel;
import java.awt.*;
import java.util.ArrayList;
import java.util.List;

/**
 * Picker dialog that shows the actual Object Tree with icons and allows selecting one or more items.
 */
public class ObjectPickerDialog extends JDialog {
    private final FastDMM editor;
    private final JTextField search = new JTextField();
    private final JTree tree;
    private List<String> result = new ArrayList<>();

    public ObjectPickerDialog(Frame owner, FastDMM editor) {
        super(owner, "Select Object(s)", true);
        this.editor = editor;
        setMinimumSize(new Dimension(500, 600));

        // Build tree using the live ObjectTree model
        tree = new JTree(editor.objTree);
        tree.setCellRenderer(new ObjectTreeRenderer(editor));
        tree.getSelectionModel().setSelectionMode(TreeSelectionModel.DISCONTIGUOUS_TREE_SELECTION);
        expandRoot();

        buildUI();
    }

    private void buildUI() {
        setLayout(new BorderLayout(8,8));
        JPanel top = new JPanel(new BorderLayout(4,4));
        top.add(new JLabel("Search:"), BorderLayout.WEST);
        top.add(search, BorderLayout.CENTER);
        add(top, BorderLayout.NORTH);

        JScrollPane scroll = new JScrollPane(tree);
        add(scroll, BorderLayout.CENTER);

        JPanel btns = new JPanel(new FlowLayout(FlowLayout.RIGHT));
        JButton add = new JButton("Add Selected");
        JButton cancel = new JButton("Cancel");
        btns.add(add);
        btns.add(cancel);
        add(btns, BorderLayout.SOUTH);

        add.addActionListener(e -> {
            result = new ArrayList<>();
            TreePath[] paths = tree.getSelectionPaths();
            if (paths != null) {
                for (TreePath p : paths) {
                    Object last = p.getLastPathComponent();
                    if (last instanceof ObjectTree.Item) {
                        result.add(((Item) last).path);
                    }
                }
            }
            dispose();
        });
        cancel.addActionListener(e -> {
            result = new ArrayList<>();
            dispose();
        });

        search.getDocument().addDocumentListener(new DocumentListener() {
            @Override public void insertUpdate(DocumentEvent e) { applyFilter(search.getText()); }
            @Override public void removeUpdate(DocumentEvent e) { applyFilter(search.getText()); }
            @Override public void changedUpdate(DocumentEvent e) { applyFilter(search.getText()); }
        });
    }

    private void expandRoot() {
        for (int i = 0; i < tree.getRowCount(); i++) {
            tree.expandRow(i);
        }
    }

    private void applyFilter(String q) {
        String query = q == null ? "" : q.trim().toLowerCase();
        if (query.isEmpty()) {
            tree.clearSelection();
            return;
        }
        // Expand all to make matches visible
        expandAll();
        // Select the first item whose path contains the query
        Item match = findFirstMatch(query);
        if (match != null) {
            TreePath tp = buildPath(match);
            if (tp != null) {
                tree.setSelectionPath(tp);
                tree.scrollPathToVisible(tp);
            }
        }
    }

    private void expandAll() {
        int oldCount = -1;
        while (tree.getRowCount() != oldCount) {
            oldCount = tree.getRowCount();
            for (int i = 0; i < oldCount; i++) tree.expandRow(i);
        }
    }

    private Item findFirstMatch(String query) {
        ObjectTree ot = editor.objTree;
        if (ot == null) return null;
        // Prefer common roots
        String[] roots = new String[]{"/obj","/turf","/area","/mob"};
        for (String r : roots) {
            for (Item it : ot.items.values()) {
                if (it.path != null && it.path.startsWith(r) && it.path.toLowerCase().contains(query)) return it;
            }
        }
        for (Item it : ot.items.values()) {
            if (it.path != null && it.path.toLowerCase().contains(query)) return it;
        }
        return null;
    }

    private TreePath buildPath(Item item) {
        java.util.List<Object> nodes = new ArrayList<>();
        nodes.add(item);
        Item cur = item.parent;
        while (cur != null) {
            nodes.add(0, cur);
            cur = cur.parent;
        }
        nodes.add(0, editor.objTree); // root
        return new TreePath(nodes.toArray());
    }

    public List<String> getSelectedTypePaths() {
        return result;
    }
}
