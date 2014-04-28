package dht;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.*;

public class NodeIO {
	private static String TAG = "NodeIO";

	byte[] toNodeShake = new byte[] {0x47, 0x21};
	byte[] fromNodeShake = new byte[] {0x47, 0x3f};
	
	DHTController controller;
	Socket nodeSocket;
	OutputStream outStream = null;
	InputStream inStream = null;
	
	
	public NodeIO(DHTController controller) {
		this.controller = controller;
	}

    /**
     * Connect to node using controller's parameters (controller is supplied in constructor).
     * @return True if connection was created succesfully
     */
	public Boolean connectNode() {
		int hostPort;
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
                return true;
			} else {
				Log.error(TAG, "Invalid handshake response from node");
			}
		} catch (UnknownHostException e) {
			Log.error(TAG, "Failed to resolve node address");
		} catch (IOException e) {
            Log.error(TAG, "Failed to open connection to node");
        }
		return false;
	}

    /**
     * Close connection to node
     */
	public void disconnect() {
		
		if (! this.nodeSocket.isClosed()) {
			try {
				this.nodeSocket.close();
			} catch (IOException ioe) {
				ioe.printStackTrace();
			}
		}
		
	}
	
	/**
	 * Send a command to the DHTnode
	 * @param command Command to send
	 */
	public void sendCommand(byte[] command) {
		
		try {
            if (nodeSocket.isClosed()) {
                Log.warn(TAG, "Socket to node is closed");
            } else {
                this.outStream.write(command);
            }

		} catch (IOException e) {
		    Log.error(TAG, "Error when sending command to node");
			e.printStackTrace();
		}
	}
	
	/**
	 * Read a response command from DHTnode
	 * @return Array of read bytes
	 */
	public byte[] readCommand() {
        if (this.nodeSocket.isClosed()) {
            Log.warn(TAG, "Socket to node is closed");
            return new byte[0];
        }

		byte[] bytesReadBuf = new byte[DataBlock.CMD_HEADER_LENGTH + DataBlock.MAX_BLOCK_SIZE];

		int bytesTotal = 0;
        int bytesRead = 0;
        int bytesMissing = DataBlock.CMD_HEADER_LENGTH;
		try {
            // Read header
            while (bytesMissing > 0) {
                bytesRead = this.inStream.read(bytesReadBuf, bytesTotal, bytesMissing);
                bytesTotal += bytesRead;
                bytesMissing -= bytesRead;
            }

            // Check payload length
            byte[] bBlockLength = new byte[2];
            System.arraycopy(bytesReadBuf, DataBlock.DATABLOCK_LEN_OFFSET, bBlockLength, 0, 2);
            bytesMissing = 256*(bBlockLength[0] & 0xFF) + (bBlockLength[1] & 0xFF); // Java has signed bytes so some trickery is needed

            // Read rest of the block (nothing if payload length is 0)
            while (bytesMissing > 0) {
                bytesRead = this.inStream.read(bytesReadBuf, bytesTotal, bytesMissing);
                bytesTotal += bytesRead;
                bytesMissing -= bytesRead;
            }

            byte[] bytesReadBufTrimmed = new byte[bytesTotal];
            System.arraycopy(bytesReadBuf, 0, bytesReadBufTrimmed, 0, bytesTotal);
            Log.debug(TAG, "Received " + bytesTotal + " bytes");
            return bytesReadBufTrimmed;

		} catch (IOException e) {
			Log.error(TAG, "Error reading from node");
			e.printStackTrace();
            return new byte[0];
		}

	}
	
}
