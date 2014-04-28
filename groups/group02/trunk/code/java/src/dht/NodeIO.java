package dht;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.*;
import java.nio.ByteBuffer;

public class NodeIO {
	private static String TAG = "NodeIO";

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
	
	
	public void startNode() throws IOException {

		/*
		String[] cmd = {"", this.controller.hostIP, this.controller.hostPort, 
				this.controller.serverIP, this.controller.serverPort};
		Runtime runTime = Runtime.getRuntime();
		runTime.exec(cmd);
		*/
		nodeSoc = this.connectNode();
		if (nodeSoc == null) {
			throw new IOException();
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
		byte[] bytesReadBuf = new byte[DataBlock.CMD_HEADER_LENGTH + DataBlock.MAX_BLOCK_SIZE];

		int bytesTotal = 0;
        int bytesRead = 0;
        int bytesMissing = DataBlock.CMD_HEADER_LENGTH;
		try {
            while (bytesMissing > 0) {
                bytesRead = this.inStream.read(bytesReadBuf, bytesTotal, bytesMissing);
                bytesTotal += bytesRead;
                bytesMissing -= bytesRead;
            }

            byte[] bBlockLength = new byte[2];
            System.arraycopy(bytesReadBuf, DataBlock.DATABLOCK_LEN_OFFSET, bBlockLength, 0, 2);
            bytesMissing = 256*(bBlockLength[0] & 0xFF) + (bBlockLength[1] & 0xFF); // Java has signed bytes so some trickery is needed

            while (bytesMissing > 0) {
                bytesRead = this.inStream.read(bytesReadBuf, bytesTotal, bytesMissing);
                bytesTotal += bytesRead;
                bytesMissing -= bytesRead;
            }

		} catch (IOException e) {
			
			e.printStackTrace();
		}
		byte[] bytesReadBufTrimmed = new byte[bytesTotal];
		System.arraycopy(bytesReadBuf, 0, bytesReadBufTrimmed, 0, bytesTotal);
        Log.debug(TAG, "Received " + bytesTotal + " bytes");
		return bytesReadBufTrimmed;
	}
	
}
