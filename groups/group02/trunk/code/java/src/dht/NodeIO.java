package dht;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.*;

public class NodeIO {
	public static String TAG = "NodeIO";

	boolean printSends = false;

	byte[] toNodeShake = new byte[] {0x47, 0x21};
	byte[] fromNodeShake = new byte[] {0x47, 0x3f};
	
	DHTController controller;
	Socket nodeSoc;
	OutputStream outStream = null;
	InputStream inStream = null;
	
	
	public NodeIO(DHTController controller) {
		this.controller = controller;
	}
	
	
	public void startNode() {
		try {
			/*
			String[] cmd = {"", this.controller.hostIP, this.controller.hostPort, 
					this.controller.serverIP, this.controller.serverPort};
			Runtime runTime = Runtime.getRuntime();
			runTime.exec(cmd);
			*/
			nodeSoc = this.connectNode();
			
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	public Socket connectNode() {
		Socket nodeSocket = null;
		int hostPort = 0;
		try {
			hostPort = Integer.parseInt(this.controller.hostPort);
			nodeSocket = new Socket(this.controller.hostIP, hostPort);
			
			this.outStream = nodeSocket.getOutputStream();
			this.inStream = nodeSocket.getInputStream();
			byte[] shakeBuf = new byte[2];
			
			this.outStream.write(toNodeShake);
			this.inStream.read(shakeBuf);
			if (shakeBuf[0] == fromNodeShake[0] && shakeBuf[1] == fromNodeShake[1]) {
				Log.info(TAG, "Connected to node");
			} else {
				Log.info(TAG, "Failed to connect to node");
			}

			
		} catch (Exception e) {
			e.printStackTrace();
		}
		
		return nodeSocket;
	}
	
	//Terminates the connection
	public void disconnect() {
		if (! this.nodeSoc.isClosed()) {
			try {
				this.nodeSoc.close();
			} catch (IOException ioe) {
				ioe.printStackTrace();
			}
		}
		
	}
	
	/**
	 * Tries to send a command to the DHTnode
	 * @param command
	 * @return
	 */
	public int sendCommand(byte[] command) {
		
		try {
			if (printSends) {
				for (int i=0; i<command.length; i++) {
					System.out.print((char)command[i]);
				}
				System.out.println();
				System.out.println(":---SEND TO NODE.");
			}
			else {
				this.outStream.write(command);
			}
		} catch (IOException e) {
			//Log.error("NodeIO", e);
			e.printStackTrace();
			return -1;
		}
		
		return 0;
	}
	
	/**
	 * Tries to read a response command from DHTnode
	 * @return
	 */
	public byte[] readCommand() {
		byte[] bytesRead = new byte[DataBlock.CMD_HEADER_LENGHT + DataBlock.MAX_BLOCK_SIZE];
		int cmdMaxLen = DataBlock.CMD_HEADER_LENGHT + DataBlock.MAX_BLOCK_SIZE;
		
		int offset = 0;
		try {
			do {
				offset = offset + this.inStream.read(bytesRead, offset, cmdMaxLen);
			} while (offset > 0);
			
		} catch (IOException e) {
			
			e.printStackTrace();
		}
		return bytesRead;
	}
	
}