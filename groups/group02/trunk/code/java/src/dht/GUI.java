package dht;
import javax.swing.*;
import javax.swing.text.BadLocationException;
import javax.swing.text.DefaultCaret;
import javax.swing.text.Document;

import java.awt.event.*;
import java.awt.Desktop;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.GraphicsEnvironment;
import java.awt.GridLayout;
import java.awt.Rectangle;
import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintStream;
import java.util.*;

public class GUI extends JFrame {
	
	// Basic variables
	public static String TAG = "GUI";
	DHTController controller;
	JProgressBar progressBar;
	int connected = 0;
	private JTextField filename = new JTextField(), dir = new JTextField();
	String file = "";
	String path = "";
	JTextPane logPane;
	JLabel logText;
	JScrollPane logScroll;
	JPanel panel;
    JScrollPane directory;
    JLabel dirText;
    JList DHTdir;
    JPopupMenu popup;
    JMenuItem popupGet;
    JMenuItem popupDump;
    String dirFilename;
    DefaultCaret caret;
 
	
	// Choosing a file to be put into DHT
    
	class PutFile implements ActionListener {
		public void actionPerformed(ActionEvent e) {
			if (connected == 1) {
	    	JFileChooser c = new JFileChooser();
	    	
	    	// Demonstrate "Open" dialog:
	    	int rVal = c.showOpenDialog(GUI.this);
	    	if (rVal == JFileChooser.APPROVE_OPTION) {
	    		filename.setText(c.getSelectedFile().getName());
	    		
	    		file = c.getSelectedFile().getName();
	    		Log.info(TAG, "Name of the file: " + file);
	    		
	    		dir.setText(c.getCurrentDirectory().toString());
	    		
	    		path = c.getCurrentDirectory().toString();
	    		Log.info(TAG, "Directory: " + path);
				String searchName = (String)JOptionPane.showInputDialog(
						GUI.this, 
						"Give a filename to be saved ",
						"Get file",
						JOptionPane.PLAIN_MESSAGE,
						null,
						null,
						null);
	    		
	    		int response = controller.putFile(path+"/"+file, searchName);
				if (response == 0) {
					directory(controller.getDHTdir());
					JOptionPane.showMessageDialog(null, "Putting file " + file + " to DHT completed.");
				}
				else if (response == 1) {
					JOptionPane.showMessageDialog(null, "Putting file failed.");
				}
				else if (response == 3) {
					JOptionPane.showMessageDialog(null, "Filename already exists");
				}
				else {
					JOptionPane.showMessageDialog(null, "IOExceptation while putting file.");
				}
	     	}
	    	if (rVal == JFileChooser.CANCEL_OPTION) {
	    		filename.setText("You pressed cancel");
	    		dir.setText("");
	    	}
	    }
		
			else {
				JOptionPane.showMessageDialog(null, "Not connected to any node");
			}
		}
	  }
	
	// Saving a file from DHT for user
	class GetFile implements ActionListener {
		public void actionPerformed(ActionEvent e) {
			if (connected == 1) {
				String searchName = (String)JOptionPane.showInputDialog(
						GUI.this, 
						"Give a filename",
						"Get file",
						JOptionPane.PLAIN_MESSAGE,
						null,
						null,
						null); 
				
				JFileChooser c = new JFileChooser();
				int rVal = c.showSaveDialog(GUI.this);
				if (rVal == JFileChooser.APPROVE_OPTION) {
					filename.setText(c.getSelectedFile().getName());
	    		
					file = c.getSelectedFile().getName();
					Log.info(TAG, "Name of the file: " + file);
	    		
					dir.setText(c.getCurrentDirectory().toString());
	    		
					path = c.getCurrentDirectory().toString();
					Log.info(TAG, "Directory: " + path);
					//TODO: GIVE FILENAME AND PATH FOR CONTROLLER
					int response = controller.getFile(searchName, path, file);
					if (response == 0) {
						directory(controller.getDHTdir());
						JOptionPane.showMessageDialog(null, "Downloading file " + searchName + " completed.");
					}
					else if (response == 1) {
						JOptionPane.showMessageDialog(null, "File not found in DHT.");
					}
					else if (response == 2) {
						JOptionPane.showMessageDialog(null, "File corrupted, download aborted.");
					}
					else {
						JOptionPane.showMessageDialog(null, "Error.");
					}
				}
				if (rVal == JFileChooser.CANCEL_OPTION) {
					filename.setText("You pressed cancel");
					dir.setText("");
				}
			}
		
			else {
				JOptionPane.showMessageDialog(null, "Not connected to any node");
			}
		}
	}
	
	
	// Dump window
	class Dump implements ActionListener {
		
		public void actionPerformed(ActionEvent e) {
			if (connected == 1) {
				String searchName = (String)JOptionPane.showInputDialog(
						GUI.this, 
						"Give a filename",
						"Dump file",
						JOptionPane.PLAIN_MESSAGE,
						null,
						null,
						null);
		    		Log.info(TAG, "File to be dumped: " + searchName);
		    		int response = controller.dumpFile(searchName);
					if (response == 0) {
						directory(controller.getDHTdir());
						JOptionPane.showMessageDialog(null, "Dumping file completed.");
					}
					else if (response == 1) {
						JOptionPane.showMessageDialog(null, "File not found in DHT.");
					}
					else {
						JOptionPane.showMessageDialog(null, "Error.");
					}
		    }
			
			
			else {
				JOptionPane.showMessageDialog(null, "Not connected to any node");
			}
		}

	}
	
	// Connection window
	class Connect implements ActionListener {
		JTextField hostAddr = new JTextField(1);
		JTextField hostPort = new JTextField(1);
		String haddr;
		String hport;
		
		public void actionPerformed(ActionEvent e) {
			JLabel textLabel3 = new JLabel();
			JLabel textLabel4 = new JLabel();
			textLabel3.setBounds(10,50,100,20);
			textLabel3.setText("Host address");
			textLabel4.setBounds(10,70,100,20);
			textLabel4.setText("Host port");
			
			JPanel connectPanel = new JPanel();
			hostAddr.setBounds(110,50,100,20);
			hostPort.setBounds(110,70,100,20);
			
			//SETTING DEFAULT VALUES
			hostAddr.setText("localhost");
			hostPort.setText("2000");
			
			connectPanel.setLayout(new GridLayout(4,1));
			connectPanel.add(textLabel3);
			connectPanel.add(hostAddr);
			connectPanel.add(textLabel4);
			connectPanel.add(hostPort);
			int result = JOptionPane.showConfirmDialog(null, connectPanel,"", JOptionPane.OK_CANCEL_OPTION);
			if (result == JOptionPane.OK_OPTION) {
				//TODO: GIVE INFO TO CONTROLLER
				haddr = hostAddr.getText();
				hport = hostPort.getText();
				try {
					int hportint = Integer.parseInt(hport);
					controller = new DHTController(haddr, hport);
					connected = 1;
					directory(controller.refreshDHTdir());
					JOptionPane.showMessageDialog(null, "Connection completed.");
	
				} catch (Exception ex) {
					JOptionPane.showMessageDialog(null, "Connection to node failed. Be sure your ports and addresses are correct and node is running.");
				}
				}
			};
	}
	
	
	class Disconnect implements ActionListener {
		public void actionPerformed(ActionEvent event) {
    		//TODO: EXIT GENTLY
    		if (connected == 1) {
    		int terminate = controller.terminate();
    		if (terminate == 0) {
    			JOptionPane.showMessageDialog(null, "Disconnected.");
    			connected = 0;
    		}
    		else if (terminate == 1)
    		{
    			JOptionPane.showMessageDialog(null, "Termination denied.");
    			}
    		else if (terminate == -1) {
    			JOptionPane.showMessageDialog(null, "Termination failed.");
    			}
    		}
    		else {
    			JOptionPane.showMessageDialog(null, "Not connected to any node.");
    		}
    	}
	}
	
	class Exit implements ActionListener {
		public void actionPerformed(ActionEvent event) {
	    	if (connected == 1) {
	    		Object[] options = {"Normally", "Abnormally", "Cancel"};
	    		int reply = JOptionPane.showOptionDialog(GUI.this, "Exit via disconnecting normally or abnormally?", "Exit", 
	    				JOptionPane.YES_NO_CANCEL_OPTION, JOptionPane.QUESTION_MESSAGE,
	    				null, options, options[2]);
	    		if (reply == JOptionPane.NO_OPTION) {
	    				System.exit(0);
	    		}
	    		else if (reply == JOptionPane.YES_OPTION) {
	    			int terminate = controller.terminate();
	    			if (terminate == 0) {
	    				System.exit(0);
	    			}
	    			else if (terminate == 1)
	    			{
	    				JOptionPane.showMessageDialog(null, "Termination denied.");
	    			}
	    			else if (terminate == -1) {
	    				JOptionPane.showMessageDialog(null, "Termination failed.");
	    			}
	        	}
	    		else {
	    			
	    		}
	    	}
	        else {
	        	JOptionPane.showMessageDialog(null, "Not connected to any node.");
	        	System.exit(0);
	        	}
	        }
		}
		
	

	
	
	public void directory(String[] list) {
		DHTdir = new JList(list);
        directory.getViewport().add(DHTdir);
	    panel.add(directory);
	    panel.add(dirText);
	    MouseListener mouseListener = new MouseAdapter() {
	    	public void mouseClicked(MouseEvent mouseEvent) {
	    		JList theList = (JList) mouseEvent.getSource();
	    		if (SwingUtilities.isRightMouseButton(mouseEvent)) {
	    			int index = theList.locationToIndex(mouseEvent.getPoint());
	    			if (index >= 0) {
	    				Object o = theList.getModel().getElementAt(index);
	    				dirFilename = o.toString();
	    				popup.show(mouseEvent.getComponent(), mouseEvent.getX(), mouseEvent.getY());
	    			}
	    		}
	    	}
	    };
	
	    DHTdir.addMouseListener(mouseListener);

	}
	
	
	
	private void updateTextPane(final String text) {
        Document doc = logPane.getDocument();
        try {
          doc.insertString(doc.getLength(), text, null);
        } catch (BadLocationException e) {
          throw new RuntimeException(e);
        }
        logPane.setCaretPosition(doc.getLength() - 1);
		Rectangle paneRect = logPane.getBounds();
		paneRect.x = 0;
		paneRect.y = 0;
		logPane.paintImmediately(paneRect);

	  }

	  private void redirectSystemStreams() {
	    OutputStream out = new OutputStream() {
	      @Override
	      public void write(final int b) throws IOException {
	        updateTextPane(String.valueOf((char) b));
	      }

	      @Override
	      public void write(byte[] b, int off, int len) throws IOException {
	        updateTextPane(new String(b, off, len));
	      }

	      @Override
	      public void write(byte[] b) throws IOException {
	        write(b, 0, b.length);
	      }
	    };

	    System.setOut(new PrintStream(out, true));
	    System.setErr(new PrintStream(out, true));
	  }
	
    // Initialization for main window and GUI
	public GUI() {
		
		  
		// Setting up main window
		setTitle("DHT GUI");
		setSize(1150,600);
		setLocationRelativeTo(null);
		panel = new JPanel();
	    getContentPane().add(panel);
	    panel.setLayout(null);
		
		// Setting up exit listener
		addWindowListener(new WindowAdapter() {
		  	public void windowClosing(WindowEvent e) {
		    	if (connected == 1) {
		    		Object[] options = {"Normally", "Abnormally"};
		    		int reply = JOptionPane.showOptionDialog(GUI.this, "Exit via disconnecting normally or abnormally?", "Exit", 
		    				JOptionPane.YES_NO_OPTION, JOptionPane.QUESTION_MESSAGE,
		    				null, options, options[1]);
		    		if (reply == JOptionPane.NO_OPTION) {
		    				System.exit(0);
		    		}
		    		else if (reply == JOptionPane.YES_OPTION) {
		    			int terminate = controller.terminate();
		    			if (terminate == 0) {
		    				System.exit(0);
		    			}
		    			else if (terminate == 1)
		    			{
		    				JOptionPane.showMessageDialog(null, "Termination denied.");
		    			}
		    			else if (terminate == -1) {
		    				JOptionPane.showMessageDialog(null, "Termination failed.");
		    			}
		        	}
		    	}
		        else {
		        	System.exit(0);
		        	}
		        }
		});
		
		// Setting connection button
	    JButton connectionButton = new JButton("Connect");
	    connectionButton.setBounds(300, 10, 100, 30);
	    panel.add(connectionButton);
	    connectionButton.addActionListener(new Connect());
	    connectionButton.setToolTipText("Connect to node");
	    
	    
	    // Setting put button
	    JButton putButton = new JButton("Put");
	    putButton.setBounds(400, 10, 100, 30);
	    putButton.addActionListener(new PutFile());
	    panel.add(putButton);
	    putButton.setToolTipText("Put data to DHT");
	    
	    
	    // Setting get button
	    JButton getButton = new JButton("Get");
	    getButton.setBounds(500, 10, 100, 30);
	    getButton.addActionListener(new GetFile());
	    panel.add(getButton);
	    getButton.setToolTipText("Get data from DHT");
	    
	    
	    // Setting dump button
	    JButton dumpButton = new JButton("Dump");
	    dumpButton.setBounds(600, 10, 100, 30);
	    panel.add(dumpButton);
	    dumpButton.addActionListener(new Dump());
	    dumpButton.setToolTipText("Dump data from DHT");

	    
	    // Setting disconnect button
	    JButton disconnectButton = new JButton("Disconnect");
	    disconnectButton.setBounds(700, 10, 120, 30);
	    disconnectButton.addActionListener(new Disconnect());
	    
	    panel.add(disconnectButton);
	    disconnectButton.setToolTipText("Leave the node gently");
	    
	    // Setting directory refresh button
	    JButton refreshButton = new JButton("Refresh");
	    refreshButton.setBounds(150,80,100,20);
	    refreshButton.addActionListener(new ActionListener() {
	    	public void actionPerformed(ActionEvent event) {
	    		if (connected == 1) {
	    			directory(controller.refreshDHTdir());
	    	}
	    		else {
	    			JOptionPane.showMessageDialog(null, "Not connected to any node.");
	    		}
	    	}
	    });
	    refreshButton.setToolTipText("Refresh DHT directory");
	    panel.add(refreshButton);
	    
	    // Setting directory & it's popup menu
	    
		dirText = new JLabel();
		dirText.setBounds(50,80,100,20);
		dirText.setText("DHT Directory");
	    directory = new JScrollPane();
        directory.setBounds(50, 100, 400, 300);
	    panel.add(directory);
	    panel.add(dirText);
	    
	    popup = new JPopupMenu();
	    popupGet = new JMenuItem("Get");
	    popupDump = new JMenuItem("Dump");
	    
	    popup.add(popupGet);
	    popup.add(popupDump);
	    panel.add(popup);
	    
	    // Popup menu's listeners for get and dump
	    
	    popupGet.addActionListener(new ActionListener() {
		    public void actionPerformed(ActionEvent e) {
		    	Log.info(TAG, "File to get from directory " + dirFilename);
		    	directory(controller.getDHTdir());
		    	JFileChooser c = new JFileChooser();
				int rVal = c.showSaveDialog(GUI.this);
				if (rVal == JFileChooser.APPROVE_OPTION) {
					filename.setText(c.getSelectedFile().getName());
	    		
					file = c.getSelectedFile().getName();
					Log.info(TAG, "Name of the file: " + file);
	    		
					dir.setText(c.getCurrentDirectory().toString());
	    		
					path = c.getCurrentDirectory().toString();
					Log.info(TAG, "Directory: " + path);
					//TODO: GIVE FILENAME AND PATH FOR CONTROLLER
					int response = controller.getFile(dirFilename, path, file);
					if (response == 0) {
						directory(controller.getDHTdir());
						JOptionPane.showMessageDialog(null, "Downloading file " + dirFilename + " completed.");
					}
					else if (response == 1) {
						JOptionPane.showMessageDialog(null, "File not found in DHT.");
					}
					else if (response == 2) {
						JOptionPane.showMessageDialog(null, "File corrupted, download aborted.");
					}
					else {
						JOptionPane.showMessageDialog(null, "Error.");
					}
				}
				if (rVal == JFileChooser.CANCEL_OPTION) {
					filename.setText("You pressed cancel");
					dir.setText("");
				}
		    }
	    });
	    
	    popupDump.addActionListener(new ActionListener() {
		    public void actionPerformed(ActionEvent e) {
		    	Log.info(TAG, "File to be dumped from directory " + dirFilename);
		    	controller.dumpFile(dirFilename);
		    	directory(controller.getDHTdir());
		    }
	    });
	    
	    // Setting log window
		logText = new JLabel();
		logText.setBounds(500,80,100,20);
		logText.setText("Log");
	    logPane = new JTextPane();
        logPane.setFont(new Font(Font.MONOSPACED, Font.PLAIN, 12));
        panel.add(logText);
		logScroll = new JScrollPane(logPane);
        logScroll.setBounds(500, 100, 600, 300);
        panel.add(logScroll);
        logPane.setEditable(false);
        redirectSystemStreams();
        caret = (DefaultCaret)logPane.getCaret();
        caret.setUpdatePolicy(DefaultCaret.ALWAYS_UPDATE);

        
        
	    
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
	    showc.setSelected(true);
	    showj.setSelected(true);
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
	    
	    connectItem.addActionListener(new Connect());
	    putItem.addActionListener(new PutFile());
	    getItem.addActionListener(new GetFile());
	    dumpItem.addActionListener(new Dump());
	    disconnectItem.addActionListener(new Disconnect());
	    eMenuItem.addActionListener(new Exit());
	    
	    showf.addActionListener(new ActionListener() {
	    	public void actionPerformed(ActionEvent e) {
	    		setExtendedState(JFrame.MAXIMIZED_BOTH);
	    	}
	    });
	    
	    showc.addActionListener(new ActionListener() {
	    	public void actionPerformed(ActionEvent e) {
	    		//TODO: SHOW / HIDE C LOG!
	    	}
	    });
	    
	    showj.addActionListener(new ActionListener() {
	    	public void actionPerformed(ActionEvent e) {
	    		//TODO: SHOW / HIDE JAVA LOG!
	    	}
	    });
	    
	    dMenuItem.addActionListener(new ActionListener() {
	    public void actionPerformed(ActionEvent e) {
	    try {
	    	// TODO: CHANGE LOCATION!
	    	File documentation = new File("home/group02/releases/iteration1/doc/iteration1.pdf");
	    	Desktop.getDesktop().open(documentation);
	    } catch (Exception ex) {
	    	JOptionPane.showMessageDialog(null, "Documentation file not found.");
	    	}
	    	}
	    });
	    

	    
	    // Show GUI
	    this.add(panel);
	    this.setVisible(true);
	}
	
	public static void main(String[] args) {
		new GUI();
	}
	
}

