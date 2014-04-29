package dht;

import java.awt.Rectangle;

import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JProgressBar;

 
public class Progressbar {
	JProgressBar progressBar;
	int maxValue;
	int init;
	String status;
    JFrame progFrame;
    JPanel progPanel;
    JLabel textLabel;
    
    /**
    * Initialization for progress bar
    * int init is the first value of the process under performance (usually zero)
    * int maxValue is maximum value of the process under performance
    * String status is the "name" of process under performance (Connecting, terminating etc.)
    */
	
	Progressbar(int init, int maxValue, String status) {
		// Setting frame and panel for progress bar, and creating progress bar
		this.init = init;
		this.maxValue = maxValue;
		this.status = status;
		this.textLabel = new JLabel();
		this.textLabel.setBounds(10,10,100,20);
		this.textLabel.setText(status);
		this.progFrame = new JFrame("Progress");
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
	
	/**
	* Updating progress bar with new progress value.
	* If value of progress >= maximum value of progress, progress bar is set invisible.
	*/
	
	public void update(int progress) {
		this.progressBar.setValue(progress);
		Rectangle progressRect = this.progressBar.getBounds(); // WITHOUT THESE
		progressRect.x = 0;									   // LINES OF CODE 
		progressRect.y = 0;									   // PROGRESS BAR		
		this.progressBar.paintImmediately( progressRect );    // WOULDN'T UPDATE
		this.textLabel.paintImmediately( progressRect );      // ITSELF VISIBLY
		if (progress >= this.maxValue) {
			this.progFrame.setVisible(false);
		}
	}
	
	/**
	* If the process faces unseen end (progress stops at < 100%) or node's leave from the 
	* server is denied (termination progress stops at 67%), this method is called.
	* In this case as progress has been stopped for some reason, GUI informs the user for that in proper way.
	* This method only sets progress bar invisible.
	*/
	
	public void interrupt() {
		this.progFrame.setVisible(false);
	}
}
