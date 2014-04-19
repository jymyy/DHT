package dht;


import java.nio.*;
import java.security.MessageDigest;
import java.io.*;

public class DHTController {
	int MAX_BLOCK_SIZE = 65535;
	String hostIP;
	String hostPort;
	String serverIP;
	String serverPort;
	
	DHTController(String hostIP, String hostPort, String serverIP, String serverPort) {
		this.hostIP = hostIP;
		this.hostPort = hostPort;
		this.serverIP = serverIP;
		this.serverPort = serverPort;
		
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
			
			for (int blockNo=1; blockNo<=totalBlocks; blockNo++) {
				nextPayload = null;
				bytesRead = fis.read(nextPayload, 0, this.MAX_BLOCK_SIZE-4);
				putBlock(new DataBlock(dhtFileName, totalBlocks, blockNo, nextPayload));
			}
			
			fis.close();
			
		} catch (Exception e) {
			e.printStackTrace();
		}
		
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
	 *  2 if the file was corrupted
	 */
	public int getFile(String fileName, String path) {
		byte[] blockKey = getSHA1(fileName +"-PART1");
		byte[] block = getBlock(blockKey);
		
		if (block == null) {
			return 1;
		}
		
		int offset = 48;
		
		try {
			File newFile = new File(path + fileName);
			OutputStream fos = new FileOutputStream(newFile);
			fos.write(block, offset, block.length - offset);
			
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
					fos.close();
					return 2;
				}
				fos.write(block, offset, block.length - offset);
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
	
	
	
	private int putBlock(DataBlock block) {
		System.out.println("Datablock send.");
		return 0;
	}
	
	private byte[] getBlock(byte[] blockKey) {
		System.out.println("Got a datablock.");
		return null;
	}
	
	private int dumpBlock(byte[] blockKey) {
		System.out.println("Datablock dumped.");
		return 0;
	}
	
	public int disconnect(DataBlock block) {
		return 0;
	}
	
	
	
	public static byte[] getSHA1(String hashable) {
		byte[] key = null;
		try {
			MessageDigest cript = MessageDigest.getInstance("SHA-1");
	        cript.reset();
	        cript.update(hashable.getBytes("utf8"));
	        key = cript.digest();
	        
	        System.out.println("Hashed a key!");
	        
		} catch (Exception e) {
			e.printStackTrace();
		}	
		return key;
	}
	
	public static void addProgress() {
		
	}
	
}
