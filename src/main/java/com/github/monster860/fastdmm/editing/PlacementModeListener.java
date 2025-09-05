package com.github.monster860.fastdmm.editing;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import com.github.monster860.fastdmm.FastDMM;
import com.github.monster860.fastdmm.editing.placement.PlacementMode;

public class PlacementModeListener implements ActionListener {
	private FastDMM editor;
	private PlacementMode mode;
	
	public PlacementModeListener(FastDMM editor, PlacementMode mode) {
		this.editor = editor;
		this.mode = mode;
	}
	
	@Override
	public void actionPerformed(ActionEvent e) {
		synchronized(editor) {
			if(editor.placementMode != null)
				editor.placementMode.flush(editor);
			editor.placementMode = mode;
		}
	}
}
