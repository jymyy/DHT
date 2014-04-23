package dht;

import java.nio.*;

/**
 * DataBlock class represents a data block that can be given to the DHTnode
 **/
public class DataBlock {
	//Command block
	private static final int KEY_OFFSET = 0;				// key [20]
	private static final int CMD_TYPE_OFFSET = 20;		// Command Type [2]
	private static final int DATABLOCK_LEN_OFFSET = 22;	// payload length [2]
	private static final int DATABLOCK_OFFSET = 24;		// datablock [MAX_BLOCK_SIZE = 65535]
	//Data block
	private static final int TOTALBLOCKS_OFFSET = 0;		// Total number of block in the file [2]
	private static final int BLOCKNO_OFFSET = 2;			// Block number of this block [2]
	private static final int DATA_OFFSET = 4;				// Data [MAX_BLOCK_SIZE - 4 = 65531]
	
	public static final int MAX_BLOCK_SIZE = 65535;
	public static final int CMD_HEADER_LENGHT = 44;
	
	byte[] blockKey; // SHA1
	int size;
	int totalBlocks;
	int blockNo;
	byte[] dataBlock; 
	

	public DataBlock(String fileName, int totalBlocks, int blockNo, byte[] payload) {
		
		this.blockKey = DHTController.getSHA1( fileName + "-PART" + Integer.toString(blockNo) );
		
		this.totalBlocks = totalBlocks;
		this.blockNo = blockNo;
		this.size = 2 + 2 + payload.length;
		
		this.dataBlock = new byte[this.size];
		
		// Datablock (payload)
			// 2 bytes Number of blocks
			// 2 bytes Number of this block
			// payload [MAX_BLOCK_SIZE - 4]
		
		ByteBuffer bb1 = ByteBuffer.allocate(2);
		ByteBuffer bb2 = ByteBuffer.allocate(2);
		
		//Big-endian as default
		bb1.putShort((short)this.totalBlocks);
		bb2.putShort((short)this.blockNo);
		
		byte[] b = bb1.array();
		System.arraycopy(b, 0, this.dataBlock, TOTALBLOCKS_OFFSET, 2);
		b = bb2.array();
		System.arraycopy(b, 0, this.dataBlock, BLOCKNO_OFFSET, 2);
		System.arraycopy(payload, 0, this.dataBlock, DATA_OFFSET, payload.length);
		
	}
	
	/**
	 * getCommandBlock returns the data block with the command headers for the DHTnode 
	 **/
	public byte[] getPutBlock(char commandType) {
		
		byte[] commandBlock = new byte[20 + 2 + 2 + this.size];
		
		byte[] bytes;
		ByteBuffer b;
		
		//Put key
		System.arraycopy(this.blockKey, 0, commandBlock, KEY_OFFSET, 20);
		
		// Put command
		b = ByteBuffer.allocate(2);
		b.putChar(commandType);
		bytes = b.array();
		System.arraycopy(bytes, 0, commandBlock, CMD_TYPE_OFFSET, 2);
		
		// Put payload size
		b = ByteBuffer.allocate(2);
		b.putChar((char) this.size);
		bytes = b.array();
		System.arraycopy(bytes, 0, commandBlock, DATABLOCK_LEN_OFFSET, 2);
		
		//Put block
		System.arraycopy(this.dataBlock, 0, commandBlock, DATABLOCK_OFFSET, this.size);

		return commandBlock;
	}
	
	
	/**
	 * Returns a command for get or dump
	 * @param commandType
	 * @param key
	 * @return
	 */
	public static byte[] getCommand(char commandType, byte[] key) {
		
		if (key == null) {
			key = new byte[20];
		}
		
		byte[] command = new byte[24];
		byte[] bytes;
		
		//Put key
		ByteBuffer b;
		System.arraycopy(key, 0, command, KEY_OFFSET, 20);
				
		// Put command
		b = ByteBuffer.allocate(2);
		b.putChar(commandType);
		bytes = b.array();
		System.arraycopy(bytes, 0, command, CMD_TYPE_OFFSET, 2);
				
		// Put payload size (=0)
		b = ByteBuffer.allocate(2);
		b.putChar((char) 0);
		bytes = b.array();
		System.arraycopy(bytes, 0, command, DATABLOCK_LEN_OFFSET, 2);
		
		return command;
	}
	
}
