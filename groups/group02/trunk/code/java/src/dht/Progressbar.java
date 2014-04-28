package dht;

import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JProgressBar;

// Progress bar

public class Progressbar {
	JProgressBar progressBar;
	int maxValue;
	int init;
	String status;
    JFrame progFrame;
    JPanel progPanel;
    JLabel textLabel;
	
	Progressbar(int init, int maxValue, String status) {
		this.init = init;
		this.maxValue = maxValue;
		this.status = status;
		this.textLabel = new JLabel();
		this.textLabel.setBounds(10,10,100,20);
		this.textLabel.setText(status);
		this.progFrame = new JFrame();
		this.progFrame.setSize(250, 150);
		this.progFrame.setLocationRelativeTo(null);
		this.progPanel = new JPanel();
		this.progPanel.setBorder(BorderFactory.createEmptyBorder(40,40,40,40));
		this.progPanel.setLayout(new BoxLayout(progPanel, BoxLayout.Y_AXIS));
		this.progressBar = new JProgressBar(this.init, this.maxValue);
		this.progressBar.setSize(150, 40);
		this.progressBar.setAlignmentX(0f);
		this.progressBar.setValue(this.init);
		this.progressBar.setStringPainted(true);
		this.progFrame.add(this.progPanel);
		this.progPanel.add(this.progressBar);
		this.progPanel.add(this.textLabel);
		if (this.init < this.maxValue) {
			this.progFrame.setVisible(true);
		}
		else {
			this.progFrame.setVisible(false);
			
		}
	}
	
	public void update(int progress) {
		this.progressBar.setValue(progress);
		this.progressBar.setStringPainted(true);
		if (progress < this.maxValue) {
			this.progFrame.setVisible(true);
		}
		else {
			this.progFrame.setVisible(false);

		}
	}
	
	public void fail() {
		this.progFrame.setVisible(false);
	}
}
