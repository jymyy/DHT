package dht;
import javax.swing.*;
import java.awt.event.*;
import java.awt.Dimension;
import java.awt.GraphicsEnvironment;
import java.util.*;

public class GUI extends JFrame {
	
	DHTController controller;
	JProgressBar progressBar;
	int connected = 0;
	private JTextField filename = new JTextField(), dir = new JTextField();
	String file = "";
	String path = "";
	JTextArea log;
	JLabel logText;
	JPanel panel;
    JScrollPane directory;
    JLabel dirText;
	
	// Choosing a file to be put in to DHT
	class PutFile implements ActionListener {
		public void actionPerformed(ActionEvent e) {
			if (connected == 1) {
	    	JFileChooser c = new JFileChooser();
	    	
	    	// Demonstrate "Open" dialog:
	    	int rVal = c.showOpenDialog(GUI.this);
	    	if (rVal == JFileChooser.APPROVE_OPTION) {
	    		filename.setText(c.getSelectedFile().getName());
	    		
	    		file = c.getSelectedFile().getName();
	    		System.out.print("Name of the file: " + file + "\n");
	    		
	    		dir.setText(c.getCurrentDirectory().toString());
	    		
	    		path = c.getCurrentDirectory().toString();
	    		System.out.print("Name of the directory: " + path);
	    		//TODO: GIVE FILENAME AND PATH FOR CONTROLLER
	    		controller.getFile(file, path);
	     	}
	    	if (rVal == JFileChooser.CANCEL_OPTION) {
	    		filename.setText("You pressed cancel");
	    		dir.setText("");
	    	}
	    }
		
			else {
				JOptionPane.showMessageDialog(null, "Not connected to any server");
			}
		}
	  }
	
	// Saving a file from DHT for user
	class GetFile implements ActionListener {
		public void actionPerformed(ActionEvent e) {
			if (connected == 1) {
			JFileChooser c = new JFileChooser();
			int rVal = c.showSaveDialog(GUI.this);
			if (rVal == JFileChooser.APPROVE_OPTION) {
	    		filename.setText(c.getSelectedFile().getName());
	    		
	    		file = c.getSelectedFile().getName();
	    		System.out.print("Name of the file: " + file + "\n");
	    		
	    		dir.setText(c.getCurrentDirectory().toString());
	    		
	    		path = c.getCurrentDirectory().toString();
	    		System.out.print("Name of the directory: " + path);
	    		//TODO: GIVE FILENAME AND PATH FOR CONTROLLER
	    		controller.getFile(file, path);
	     	}
	    	if (rVal == JFileChooser.CANCEL_OPTION) {
	    		filename.setText("You pressed cancel");
	    		dir.setText("");
	    	}
		}
		
			else {
				JOptionPane.showMessageDialog(null, "Not connected to any server");
			}
		}
	}
	
	// Connection window
	class Connect implements ActionListener {
		JFrame connection = new JFrame("Connection");
		JTextField serverAddr = new JTextField(1);
		JTextField serverPort = new JTextField(1);
		JTextField hostAddr = new JTextField(1);
		JTextField hostPort = new JTextField(1);
		String saddr;
		String sport;
		String haddr;
		String hport;
		
		public void actionPerformed(ActionEvent e) {
			connection.setSize(250, 200);
			connection.setLocationRelativeTo(null);
			
			JLabel textLabel1 = new JLabel();
			JLabel textLabel2 = new JLabel();
			JLabel textLabel3 = new JLabel();
			JLabel textLabel4 = new JLabel();
			textLabel1.setBounds(10,10,100,20);
			textLabel1.setText("Server's address");
			textLabel2.setBounds(10,30,100,20);
			textLabel2.setText("Server's port");
			textLabel3.setBounds(10,50,100,20);
			textLabel3.setText("Host address");
			textLabel4.setBounds(10,70,100,20);
			textLabel4.setText("Host port");
			
			JPanel connectPanel = new JPanel();
		    getContentPane().add(connectPanel);
		    connectPanel.setLayout(null);
			serverAddr.setBounds(110,10,100,20);
			serverPort.setBounds(110,30,100,20);
			hostAddr.setBounds(110,50,100,20);
			hostPort.setBounds(110,70,100,20);
			JButton connect = new JButton("Connect");
			connect.setBounds(50,100,100,30);
			connectPanel.add(textLabel1);
			connectPanel.add(textLabel2);
			connectPanel.add(textLabel3);
			connectPanel.add(textLabel4);
			connectPanel.add(connect);
			connectPanel.add(serverAddr);
			connectPanel.add(serverPort);
			connectPanel.add(hostAddr);
			connectPanel.add(hostPort);
			connect.addActionListener(new ActionListener() {
		    	public void actionPerformed(ActionEvent event) {
		    		//TODO: GIVE INFO TO CONTROLLER
		    		saddr = serverAddr.getText();
		    		sport = serverPort.getText();
		    		haddr = hostAddr.getText();
		    		hport = hostPort.getText();
		    		System.out.print("Server address: " + saddr + " Server port: " + sport + " Host address: " + haddr + " Host port: " + hport);
		    		controller = new DHTController(haddr, hport, saddr, sport);
		    		connected = 1;
		    		directory(controller.getDHTdir());
		    		connection.setVisible(false);
		    		}
		    });
			
			connection.add(connectPanel);
			connection.setVisible(true);
			
		}
	}
	
	// Dump window
	class Dump implements ActionListener {
		JFrame dumping = new JFrame("Dump file");
		JTextField input = new JTextField(1);
		String inputText;
		
		public void actionPerformed(ActionEvent e) {
			if (connected == 1) {
			dumping.setSize(250, 150);
			dumping.setLocationRelativeTo(null);
			
			JPanel dumpPanel = new JPanel();
			JLabel textLabel = new JLabel();
			textLabel.setBounds(10,10,100,20);
			textLabel.setText("Give a filename");
		    getContentPane().add(dumpPanel);
		    dumpPanel.setLayout(null);
		    dumpPanel.setSize(500, 500);
		    dumpPanel.setLocation(0,0);
			input.setBounds(110,10,100,20);
			JButton dump = new JButton("Dump file");
			dump.setBounds(50,40,100,30);
			dumpPanel.add(textLabel);
			dumpPanel.add(dump);
			dumpPanel.add(input);
			dump.addActionListener(new ActionListener() {
		    	public void actionPerformed(ActionEvent event) {
		    		//TODO: GIVE NAME OF THE FILE (inputText) TO CONTROLLER
		    		inputText = input.getText();
		    		System.out.print(inputText);
		    		controller.dumpFile(inputText);
		    		dumping.setVisible(false);
		    		}
		    });
			
			dumping.add(dumpPanel);
			dumping.setVisible(true);
			
		}
			
		else {
				JOptionPane.showMessageDialog(null, "Not connected to any server");
			}
		}

	}
	
	// Progress bar
	
	public void progress(int progress, int maxValue) {
		JPanel progPanel = new JPanel();
		progPanel.setBorder(BorderFactory.createEmptyBorder(40,40,40,40));
		progPanel.setLayout(new BoxLayout(progPanel, BoxLayout.Y_AXIS));
		progressBar = new JProgressBar(progress, maxValue);
		progressBar.setMaximumSize(new Dimension(150, 20));
		progressBar.setMinimumSize(new Dimension(150, 20));
		progressBar.setPreferredSize(new Dimension(150, 20));
		progressBar.setAlignmentX(0f);
	}
	
	
	// Initalization for main window and GUI
	
	public void directory(String[] list) {
        JList dir = new JList(list);
        directory.getViewport().add(dir);
	    panel.add(directory);
	    panel.add(dirText);

	}
	
	public void log(String msg) {

        log.append(msg);
        log.setBounds(500, 100, 400, 300);
	    panel.add(log);
	    panel.add(logText);
	}
	
	public GUI() {
		  
		// Setting up main window
		setTitle("DHT GUI");
		setSize(950,600);
		setLocationRelativeTo(null);
		panel = new JPanel();
	    getContentPane().add(panel);
	    panel.setLayout(null);
		
		// Setting up listeners
		addWindowListener(new WindowAdapter() {
		  	public void windowClosing(WindowEvent e) {
			   System.exit(0);
		  	}
		});
		
		// Setting connection button
	    JButton connectionButton = new JButton("Connect");
	    connectionButton.setBounds(200, 10, 100, 30);
	    panel.add(connectionButton);
	    connectionButton.addActionListener(new Connect());
	    connectionButton.setToolTipText("Connect to server");
	    
	    
	    // Setting put button
	    JButton putButton = new JButton("Put");
	    putButton.setBounds(300, 10, 100, 30);
	    putButton.addActionListener(new PutFile());
	    panel.add(putButton);
	    putButton.setToolTipText("Put data to DHT");
	    
	    
	    // Setting get button
	    JButton getButton = new JButton("Get");
	    getButton.setBounds(400, 10, 100, 30);
	    getButton.addActionListener(new GetFile());
	    panel.add(getButton);
	    getButton.setToolTipText("Get data from DHT");
	    
	    
	    // Setting dump button
	    JButton dumpButton = new JButton("Dump");
	    dumpButton.setBounds(500, 10, 100, 30);
	    panel.add(dumpButton);
	    dumpButton.addActionListener(new Dump());
	    dumpButton.setToolTipText("Dump data from DHT");

	    
	    // Setting disconnect button
	    JButton disconnectButton = new JButton("Disconnect");
	    disconnectButton.setBounds(600, 10, 120, 30);
	    disconnectButton.addActionListener(new ActionListener() {
	    	public void actionPerformed(ActionEvent event) {
	    		//TODO: EXIT GENTLY
	    		if (connected == 1) {
	    		int terminate = controller.terminate();
	    		if (terminate == 0) {
	    		System.exit(0);
	    		}
	    		else if (terminate == 1)
	    		{
	    			JOptionPane.showMessageDialog(null, "Termination denied.");
	    		}
	    		else {
	    			JOptionPane.showMessageDialog(null, "Termination failed.");
	    		}
	    	}
	    	else {
	    		JOptionPane.showMessageDialog(null, "Not connected to any server.");
	    	}
	    	}
	    });
	    
	    panel.add(disconnectButton);
	    disconnectButton.setToolTipText("Leave the server gently");
	    
	    // Setting directory refresh button
	    JButton refreshButton = new JButton("Refresh");
	    refreshButton.setBounds(150,80,100,20);
	    refreshButton.addActionListener(new ActionListener() {
	    	public void actionPerformed(ActionEvent event) {
	    		if (connected == 1) {
	    		directory(controller.getDHTdir());
	    	}
	    		else {
	    			JOptionPane.showMessageDialog(null, "Not connected to any server.");
	    		}
	    	}
	    });
	    refreshButton.setToolTipText("Refresh DHT directory");
	    panel.add(refreshButton);
	    
	    // Setting directory
	    
		dirText = new JLabel();
		dirText.setBounds(50,80,100,20);
		dirText.setText("DHT Directory");
	    directory = new JScrollPane();
        directory.setBounds(50, 100, 400, 300);
	    panel.add(directory);
	    panel.add(dirText);
	    
	    // Setting log window
		logText = new JLabel();
		logText.setBounds(500,80,100,20);
		logText.setText("Log");
	    log = new JTextArea();
        log.setBounds(500, 100, 400, 300);
        log("LOGI TOIMII");
	    
	    // Setting upper menu
	    JMenuBar menubar = new JMenuBar();
	    JMenu fileMenu = new JMenu("File");
	    JMenu viewMenu = new JMenu("View");
	    JMenu helpMenu = new JMenu("Help");
	    JMenu logMenu = new JMenu("Log");
	    JMenuItem eMenuItem = new JMenuItem("Exit");
	    JMenuItem connectItem = new JMenuItem("Connect");
	    JMenuItem putItem = new JMenuItem("Put");
	    JMenuItem getItem = new JMenuItem("Get");
	    JMenuItem dumpItem = new JMenuItem("Dump");
	    JMenuItem disconnectItem = new JMenuItem("Disconnect");
	    JMenuItem dMenuItem = new JMenuItem("Documentation");
	    JCheckBoxMenuItem showc = new JCheckBoxMenuItem("Show C log");
	    JCheckBoxMenuItem showj = new JCheckBoxMenuItem("Show Java log");
	    JCheckBoxMenuItem showf = new JCheckBoxMenuItem("Full screen");
	    fileMenu.add(connectItem);
	    fileMenu.add(putItem);
	    fileMenu.add(getItem);
	    fileMenu.add(dumpItem);
	    fileMenu.add(disconnectItem);
	    fileMenu.add(eMenuItem);
	    logMenu.add(showc);
	    logMenu.add(showj);
	    viewMenu.add(showf);
	    viewMenu.add(logMenu);
	    helpMenu.add(dMenuItem);
	    menubar.add(fileMenu);
	    menubar.add(viewMenu);
	    menubar.add(Box.createHorizontalGlue());
	    menubar.add(helpMenu);
	    setJMenuBar(menubar);
	    
	    
	    // Default close
	    setDefaultCloseOperation(EXIT_ON_CLOSE);
	    
	    // Show GUI
	    this.add(panel);
	    this.setVisible(true);
	    }
	
	  public static void main(String[] args) {
	    new GUI();
	  }
}

