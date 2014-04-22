package dht;

import java.nio.*;
import java.security.MessageDigest;
import java.io.*;

public class DHTController {
	private final int CMD_PUT_DATA = 1;
	private final int CMD_GET_DATA = 2;
	private final int CMD_DUMP_DATA = 3;
	private final int CMD_TERMINATE = 4;
	private final int CMD_ACQUIRE_REQUEST = 5;
	private final int CMD_RELEASE_REQUEST = 6;

	private final int CMD_PUT_DATA_ACK = 11;
	private final int CMD_GET_DATA_ACK = 12;
	private final int CMD_GET_NO_DATA_ACK = 13;
	private final int CMD_DUMP_DATA_ACK = 14;
	private final int CMD_TERMINATE_ACK = 15;
	private final int CMD_TERMINATE_DENY = 16;
	private final int CMD_ACQUIRE_ACK = 17;
	private final int CMD_RELEASE_ACK = 18;

	private final int CMD_REGISTER_DONE = 21;
	private final int CMD_DEREGISTER_DONE = 22;
	private final int CMD_BLOCKS_MAINTAINED = 23;
	
	public static final int MAX_BLOCK_SIZE = 65535;
	String hostIP;
	String hostPort;
	String serverIP;
	String serverPort;
	NodeIO nodeIO;
	
	private String[] dhtDir = {"testfile"};
	
	DHTController(String hostIP, String hostPort, String serverIP, String serverPort) {
		this.hostIP = hostIP;
		this.hostPort = hostPort;
		this.serverIP = serverIP;
		this.serverPort = serverPort;
		
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
			int totalBlocks = (int) (fileSize / (this.MAX_BLOCK_SIZE-4));
			if (fileSize % (this.MAX_BLOCK_SIZE-4) != 0) {
				totalBlocks++;
			}
			
			InputStream fis = new FileInputStream(file);
			
			byte[] nextPayload;
			int bytesRead = 0;
			int response;
			// Put each file block to the DHT
			for (int blockNo=1; blockNo<=totalBlocks; blockNo++) {
				nextPayload = null;
				bytesRead = fis.read(nextPayload, 0, this.MAX_BLOCK_SIZE-4);
				response = putBlock(new DataBlock(dhtFileName, totalBlocks, blockNo, nextPayload));
				if (response != 0) {
					// block putting fails
					return 1;
				}
			}
			
			fis.close();
			
		} catch (IOException e) {
			
			e.printStackTrace();
			return 2;
		}
		// Everything worked
		return 0;
	}
	
	/**
	 * Gets the given file from the DHT and saves it to given path
	 * 
	 * creates the file even if its corrupted needs fixing
	 * 
	 * @param fileName
	 * @param path
	 * 
	 * @return
	 * 	0 if successful or 
	 * 	1 if a file with that name wasn't found
	 *  2 if the file was corrupted (missing block)
	 */
	public int getFile(String fileName, String path) {
		byte[] blockKey = getSHA1(fileName +"-PART1");
		byte[] block = getBlock(blockKey);
		
		if (block == null) {
			//File was not found
			return 1;
		}
		
		int HeaderOffset = 48;
		
		try {
			File newFile = new File(path + fileName);
			OutputStream fos = new FileOutputStream(newFile);
			fos.write(block, HeaderOffset, block.length - HeaderOffset);
			
			byte[] bTotalBlocks = new byte[2];
			System.arraycopy(bTotalBlocks, 0, block, 0, 2);
			ByteBuffer bb = ByteBuffer.wrap(bTotalBlocks);
			int totalBlocks = bb.getInt(); 
			
			int blockNo = 1;
			while (blockNo < totalBlocks) {
				blockNo++;
				blockKey = getSHA1(fileName +"-PART" + Integer.toString(blockNo));
				block = getBlock(blockKey);
				if (block == null) {
					// File was missing a block
					// TODO remove all blocks if the file is corrupted
					fos.close();
					return 2;
				}
				fos.write(block, HeaderOffset, block.length - HeaderOffset);
				addProgress(); // Notify that the operation has progressed
				
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

		byte[] blockKey = getSHA1(fileName +"-PART1");
		byte[] firstBlock = getBlock(blockKey);
		if (firstBlock == null) {
			//First block not found
			return 1;
		}
		byte[] bTotalBlocks = new byte[2];
		System.arraycopy(bTotalBlocks, 0, firstBlock, 0, 2);
		ByteBuffer bb = ByteBuffer.wrap(bTotalBlocks);
		int totalBlocks = bb.getInt(); 
				
		// Goes through all blocks of the file
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
		
		this.nodeIO.sendCommand(DataBlock.getCommand(this.CMD_TERMINATE, null));
		byte[] nodeResponse = this.nodeIO.readCommand();
		int responseCode = extractResponseCode(nodeResponse);
		
		System.out.println("Asking for termination. Waiting for response...");
		if (responseCode == this.CMD_TERMINATE_ACK) {
			System.out.println("Response OK.");
			this.nodeIO.disconnect();
			return 0;
		}
		else if (responseCode == this.CMD_TERMINATE_DENY) {
			System.out.println("Termination denied.");
			return 1;
		}
		else {
			System.out.println("Termination failed");
			return -1;
		}
		
	}
	
	
	/**
	 * Gets the directory of all files in the DHT
	 * @return
	 */
	public String[] getDHTdir() {
		return dhtDir;
	}
	
	
	private int dirUpdate(String newFile) {
		return 0;
	}
	
	private int dirDelete(String removableFile) {
		return 0;
	}
	
	
	
	/**
	 * Puts the given DataBlock to the DHT
	 * @param block
	 * @return
	 * 	0 if successful
	 * 	-1 if put fails somehow
	 */
	private int putBlock(DataBlock block) {
		
		System.out.println("Sending dataBlock...");
		
		byte[] cmd = block.getPutBlock(CMD_PUT_DATA);
		this.nodeIO.sendCommand(cmd);
		byte[] nodeResponse = this.nodeIO.readCommand();
		int responseCode = extractResponseCode(nodeResponse);
		
		System.out.println("Datablock send. Waiting for response...");
		if (responseCode == this.CMD_PUT_DATA_ACK) {
			System.out.println("Response OK.");
			return 0;
		}
		else {
			System.out.println("Putting failed");
			return -1;
		}
		
	}
	
	/**
	 * Requests a block with given key from DHTnode
	 * and returns the response to 
	 * @param blockKey
	 * @return
	 * 	Node's response byte[] if successful
	 * 	null if block not found
	 * 	null if get just failed
	 */
	private byte[] getBlock(byte[] blockKey) {
		
		this.nodeIO.sendCommand(DataBlock.getCommand(CMD_GET_DATA, blockKey));
		byte [] nodeResponse = this.nodeIO.readCommand();
		
		int responseCode = extractResponseCode(nodeResponse);
		
		System.out.println("Datablock requested. Waiting for response...");
		if (responseCode == this.CMD_GET_DATA_ACK) {
			System.out.println("Response OK.");
			return nodeResponse;
		}
		else if (responseCode == this.CMD_GET_NO_DATA_ACK) {
			System.out.println("Block not found.");
			return null;
		}
		else {
			System.out.println("Get failed");
			return null;
		}
	}
	
	/**
	 * 
	 * @param blockKey
	 * @return
	 *  0 if successful
	 *  -1 if failed some how
	 */
	private int dumpBlock(byte[] blockKey) {
		
		this.nodeIO.sendCommand(DataBlock.getCommand(this.CMD_DUMP_DATA, blockKey));
		byte [] nodeResponse = this.nodeIO.readCommand();
		
		int responseCode = extractResponseCode(nodeResponse);
		
		System.out.println("Dumping Datablock. Waiting for response...");
		if (responseCode == this.CMD_DUMP_DATA_ACK) {
			System.out.println("Response OK.");
			return 0;
		}
		else {
			System.out.println("Dump failed");
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
	
	
	public static void addProgress() {
		
	}
	
	// Returns the responseCode extracted from DHTnode's response command
	private int extractResponseCode(byte[] nodeResponse) {
		byte[] responseCodeArr = new byte[2];
		System.arraycopy(nodeResponse, 0, responseCodeArr, 0, 2);
		ByteBuffer wrapped = ByteBuffer.wrap(responseCodeArr); // big-endian by default
		int responseCode = wrapped.getInt();
		return responseCode;
	}
	
}
