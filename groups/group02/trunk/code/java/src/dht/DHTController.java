package dht;

import java.nio.*;
import java.security.MessageDigest;
import java.io.*;

public class DHTController {
	public static String TAG = "Controller";
	
	private final char CMD_PUT_DATA = 1;
	private final char CMD_GET_DATA = 2;
	private final char CMD_DUMP_DATA = 3;
	private final char CMD_TERMINATE = 4;
	private final char CMD_ACQUIRE_REQUEST = 5;
	private final char CMD_RELEASE_REQUEST = 6;

	private final char CMD_PUT_DATA_ACK = 11;
	private final char CMD_GET_DATA_ACK = 12;
	private final char CMD_GET_NO_DATA_ACK = 13;
	private final char CMD_DUMP_DATA_ACK = 14;
	private final char CMD_TERMINATE_ACK = 15;
	private final char CMD_TERMINATE_DENY = 16;
	private final char CMD_ACQUIRE_ACK = 17;
	private final char CMD_RELEASE_ACK = 18;

	private final char CMD_REGISTER_DONE = 21;
	private final char CMD_DEREGISTER_DONE = 22;
	private final char CMD_BLOCKS_MAINTAINED = 23;
	
	public static final int MAX_BLOCK_SIZE = 65535;
	public static final int MAX_BLOCK_PL_SIZE = 65531;
	String hostIP;
	String hostPort;
	String serverIP;
	String serverPort;
	NodeIO nodeIO;
	GUI gui;
	
	private String progBarStatus;
	private int progBarValue;
	private int progBarMax;
	
	private String[] dhtDir = {"testfile"};
	
	DHTController(String hostIP, String hostPort, String serverIP, String serverPort) {
		this.hostIP = hostIP;
		this.hostPort = hostPort;
		this.serverIP = serverIP;
		this.serverPort = serverPort;
		this.gui = GUI.gui;  // Needs thinking!! Needed to call the progress bar
		
		this.nodeIO = new NodeIO(this); 
		nodeIO.startNode();
		
	}
	
	
	/**
	 * Puts the file with given path name and name to the DHT
	 * 
	 * @param filePath
	 *	path to the file
	 * @param dhtFileName
	 * 	Name of the file in the DHT
	 * @return
	 *  0 if successful
	 *  1 if an error occurred with
	 *  2 if IOError occurs
	 */
	public int putFile(String filePath, String dhtFileName) {
		try {
			File file = new File(filePath);
			
			// How many blocks are needed
			long fileSize = file.length(); 	
			int totalBlocks = (int) (fileSize / (MAX_BLOCK_PL_SIZE));
			if (fileSize % (MAX_BLOCK_PL_SIZE) != 0) {
				totalBlocks++;
			}
			// Init progress bar
			startProgress(1, totalBlocks*2 + 1 , "Uploading file " + dhtFileName);

			InputStream fis = new FileInputStream(file);
			byte[] nextPayloadBuf;
			byte[] nextPayload;
			int bytesRead = 0;
			int response;
			// Put each file block to the DHT
			for (int blockNo=1; blockNo<=totalBlocks; blockNo++) {
				nextPayloadBuf = new byte[MAX_BLOCK_PL_SIZE];
				bytesRead = fis.read(nextPayloadBuf);
				nextPayload = new byte[bytesRead];
				System.arraycopy(nextPayloadBuf, 0, nextPayload, 0, bytesRead);
				response = putBlock(new DataBlock(dhtFileName, totalBlocks, blockNo, nextPayload));
				if (response != 0) {
					Log.warn(TAG, "Put file failed");
					fis.close();
					return 1;
				}
			}
			
			fis.close();
			
		} catch (IOException e) {
			Log.error(TAG, "IOException while downloading a file");
			e.printStackTrace();
			return 2;
		}
		// Everything worked
		return 0;
	}
	
	/**
	 * Gets the given file from the DHT and saves it to given path
	 * @param dhtFileName
	 * @param path
	 * @param newFileName
	 * @return
	 * 	0 if successful or 
	 * 	1 if a file with that name wasn't found
	 *  2 if the file was corrupted (missing block)
	 */
	public int getFile(String dhtFileName, String path, String newFileName) {
		byte[] blockKey = getSHA1(dhtFileName +"-PART1");
		//Progress bar for searching
		startProgress(0, 2, "Searching the DHT for " + dhtFileName);
		
		Log.info(TAG, "Searching file named " + dhtFileName);
		// Get the first block
		byte[] block = getBlock(blockKey);
		
		if (block == null) { // Requested file was not found
			Log.info(TAG, "File named \""+ dhtFileName +"\" was not found");
			return 1;
		}
		
		int HeaderOffset = 48;
		try {
			File newFile = new File(path + newFileName);
			OutputStream fos = new FileOutputStream(newFile);
			fos.write(block, HeaderOffset, block.length - HeaderOffset);
			
			byte[] bTotalBlocks = new byte[2];
			System.arraycopy(bTotalBlocks, 0, block, 0, 2);
			ByteBuffer bb = ByteBuffer.wrap(bTotalBlocks);
			int totalBlocks = (int) bb.getShort(); 
			// Start progress bar for downloading
			startProgress(2, totalBlocks*3 + 2, "Downloading " + dhtFileName);
			int blockNo = 1;
			while (blockNo < totalBlocks) {
				blockNo++;
				blockKey = getSHA1(dhtFileName +"-PART" + Integer.toString(blockNo));
				block = getBlock(blockKey);
				if (block == null) {
					// File was missing a block
					Log.info(TAG, "File was missing a block -> Aborting");
					fos.close();
					return 2;
				}
				fos.write(block, HeaderOffset, block.length - HeaderOffset);
				addProgress();
			} 
			fos.close();
			
		} catch (Exception e) {
			e.printStackTrace();
		}
		return 0;
	}
	
	
	/**
	 * Dumps the file with given name from the DHT if it is found
	 * 
	 * @param fileName
	 * 	Name of the file in the DHT to be dumped
	 * @return 
	 * 	0 if dumping was successful or 
	 * 	1 if file with that name wasn't found
	 */
	public int dumpFile(String fileName) {
		startProgress(0, 2, "Searching the DHT for " + fileName);
		byte[] blockKey = getSHA1(fileName +"-PART1");
		byte[] firstBlock = getBlock(blockKey);
		if (firstBlock == null) {
			//First block not found
			Log.info(TAG, "File was not found.");
			return 1;
		}
		byte[] bTotalBlocks = new byte[2];
		System.arraycopy(bTotalBlocks, 0, firstBlock, 0, 2);
		ByteBuffer bb = ByteBuffer.wrap(bTotalBlocks);
		int totalBlocks = (int) bb.getShort(); 
		// Start progress bar for downloading
		startProgress(2, totalBlocks*2 + 2, "Dumping " + fileName);
		// Go through all blocks of the file
		int blockNo = 1;
		do {
			blockKey = getSHA1(fileName +"-PART" + Integer.toString(blockNo));
			dumpBlock(blockKey);
			addProgress(); // Notify that the operation has progressed
			blockNo++;
		} while (blockNo < totalBlocks);
		
		return 0;
	}
	
	/**
	 * Tries to terminate the node and disconnect from the DHT
	 * @return
	 * 	0 if terminating is successful
	 * 	1 if termination is denied
	 * -1 if terminating fails somehow
	 */
	public int terminate() {
		startProgress(0, 3, "Terminating connection");
		this.nodeIO.sendCommand(DataBlock.getCommand(CMD_TERMINATE, null));
		Log.debug(TAG, "Terminate command send, waiting for node's response");
		addProgress();
		byte[] nodeResponse = this.nodeIO.readCommand();
		addProgress();
		int responseCode = extractResponseCode(nodeResponse);
		
		if (responseCode == this.CMD_TERMINATE_ACK) {
			Log.debug(TAG, "Response: termination OK");
			this.nodeIO.disconnect();
			Log.info(TAG, "Node disconnected.");
			addProgress();
			return 0;
		}
		else if (responseCode == this.CMD_TERMINATE_DENY) {
			Log.info(TAG, "Termination denied.");
			return 1;
		}
		else {
			System.out.println("Termination failed");
			Log.error(TAG, "Error when trying to terminate.");
			return -1;
		}
		
	}
	
	
	/**
	 * Gets the directory of all files in the DHT
	 * @return
	 */
	public String[] getDHTdir() {
		// TODO directory operations
		return dhtDir;
	}
	
	public String[] refreshDHTdir() {
		// TODO directory operations
		return dhtDir;
	}
	
	private int getDir() {
		return 0;
	}
	
	private int dirUpdate(String newFile) {
		
		return 0;
	}
	
	private int dirDelete(String removableFile) {
		return 0;
	}
	
	
	
	/**
	 * Puts the given DataBlock to the DHT.
	 * Each call increments progress bar twice.
	 * @param block
	 * @return
	 * 	0 if successful
	 * 	-1 if put fails somehow
	 */
	private int putBlock(DataBlock block) {
		byte[] cmd = block.getPutBlock(CMD_PUT_DATA);
		this.nodeIO.sendCommand(cmd);
		Log.debug(TAG, "Put command send, waiting for response.");
		addProgress();
		byte[] nodeResponse = this.nodeIO.readCommand();
		char responseCode = extractResponseCode(nodeResponse);
		addProgress();
		if (responseCode == this.CMD_PUT_DATA_ACK) {
			Log.debug(TAG, "Response: put OK.");
			return 0;
		}
		else {
			return -1;
		}
		
	}
	
	/**
	 * Requests a block with given key from DHT
	 * and returns the response block.
	 * Each call increments progress bar twice.
	 * @param blockKey
	 * @return
	 * 	Node's response byte[] if successful
	 * 	null if block not found
	 * 	null if get just failed
	 */
	private byte[] getBlock(byte[] blockKey) {
		
		this.nodeIO.sendCommand(DataBlock.getCommand(CMD_GET_DATA, blockKey));
		Log.debug(TAG, "Get command send, waiting for nodes response.");
		addProgress();
		byte [] nodeResponse = this.nodeIO.readCommand();
		addProgress();
		char responseCode = extractResponseCode(nodeResponse);
		
		if (responseCode == this.CMD_GET_DATA_ACK) {
			Log.debug(TAG, "Response: get OK");
			return nodeResponse;
		}
		else if (responseCode == this.CMD_GET_NO_DATA_ACK) {
			Log.debug(TAG, "Response: No data");
			return null;
		}
		else {
			Log.debug(TAG, "Response: Unknown");
			return null;
		}
	}
	
	/**
	 * Dumps block with given key from the DHT.
	 * Each call increments progress bar twice.
	 * @param blockKey
	 * @return
	 *  0 if successful
	 *  -1 if failed some how
	 */
	private int dumpBlock(byte[] blockKey) {
		
		this.nodeIO.sendCommand(DataBlock.getCommand(this.CMD_DUMP_DATA, blockKey));
		Log.debug(TAG, "Dump command send, waiting for node's response.");
		addProgress();
		byte [] nodeResponse = this.nodeIO.readCommand();
		int responseCode = extractResponseCode(nodeResponse);
		addProgress();
		if (responseCode == this.CMD_DUMP_DATA_ACK) {
			Log.debug(TAG, "Response: dump OK");
			return 0;
		}
		else {
			Log.debug(TAG, "Response: Unknown");
			return -1;
		}
		
	}
	
	
	/**
	 * Computes SHA1 hashes from given strings
	 * @param hashable
	 * @return
	 * 	byte[20] hashKey
	 */
	public static byte[] getSHA1(String hashable) {
		byte[] hashKey = null;
		try {
			MessageDigest cript = MessageDigest.getInstance("SHA-1");
	        cript.reset();
	        cript.update(hashable.getBytes("utf8"));
	        hashKey = cript.digest();
	        
	        System.out.println("Hashed a key!");
	        
		} catch (Exception e) {
			e.printStackTrace();
		}	
		return hashKey;
	}
	
	private void startProgress(int progValue, int maxValue, String newProgBarStatus) {
		if (this.gui != null) {
			progBarStatus = newProgBarStatus;
			progBarValue = progValue;
			progBarMax = maxValue;
			gui.progress(progValue, maxValue, progBarStatus);
		}
		
	}

	private void addProgress() {
		progBarValue++;
		if (progBarValue > progBarMax) {
			progBarStatus = "";
			progBarValue = 0;
			progBarMax = 0;
		}
		else {
			gui.progress(progBarValue, progBarMax, progBarStatus);
		}
	}
	

	// Returns the responseCode extracted from DHTnode's response command
	private char extractResponseCode(byte[] nodeResponse) {
		byte[] responseCodeArr = new byte[2];
		System.arraycopy(nodeResponse, 20, responseCodeArr, 0, 2);
		ByteBuffer wrapped = ByteBuffer.wrap(responseCodeArr); // big-endian by default
		char responseCode = wrapped.getChar();
		return responseCode;
	}
	
}
