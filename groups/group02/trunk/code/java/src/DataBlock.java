package dht;

import java.nio.*;

/**
 * DataBlock class represents a data block that can be given to the DHTnode
 **/
public class DataBlock {
	private static final int keyOffset = 0;			// key [20]
	private static final int cmdOffset = 20;			// Command Type [2]
	private static final int plLenOffset = 22;		// payload length [2]
	private static final int dataBlockOffset = 24;	// datablock [MAX_BLOCK_SIZE = 65535]
	
	public static final int MAX_BLOCK_SIZE = 65535;
	public static final int CMD_HEADER_LENGHT = 44;
	byte[] blockKey; // SHA1
	int size;
	int totalBlocks;
	int blockNo;
	byte[] payload;
	byte[] block; 
	

	public DataBlock(String fileName, int totalBlocks, int blockNo, byte[] payload) {
		
		this.blockKey = DHTController.getSHA1( fileName + "-PART" + Integer.toString(blockNo) );
		
		this.totalBlocks = totalBlocks;
		this.blockNo = blockNo;
		this.payload  = payload;
		
		this.block = new byte[65535];
		
		// Datablock
				// 2 bytes Number of blocks
				// 2 bytes Number of this block
				// payload [MAX_BLOCK_SIZE - 4]
		
		ByteBuffer b1 = ByteBuffer.allocate(2);
		ByteBuffer b2 = ByteBuffer.allocate(2);
		
		//Big-endian as default
		b1.putInt(this.totalBlocks);
		b2.putInt(this.blockNo);
		
		
		byte[] b = b1.array();
		System.arraycopy(b, 0, this.block, 0, 2);
		b = b2.array();
		System.arraycopy(b, 0, this.block, 2, 2);
		System.arraycopy(payload, 0, this.block, 4, payload.length);
		
	}
	
	/**
	 * getCommandBlock returns the data block with the command headers for the DHTnode 
	 **/
	public byte[] getPutBlock(int commandType) {
		
		byte[] commandBlock = new byte[24 + this.MAX_BLOCK_SIZE];
		
		byte[] bytes;
		ByteBuffer b;
		
		//Put key
		System.arraycopy(this.blockKey, 0, commandBlock, DataBlock.keyOffset, 20);
		
		// Put command
		b = ByteBuffer.allocate(2);
		b.putShort((short)commandType);
		bytes = b.array();
		System.arraycopy(bytes, 0, commandBlock, DataBlock.cmdOffset, 2);
		
		
		
		// Put payload size
		b = ByteBuffer.allocate(2);
		b.putShort((short)this.size);
		bytes = b.array();
		System.arraycopy(bytes, 0, commandBlock, DataBlock.plLenOffset, 2);
		
		//Put block
		System.arraycopy(this.block, 0, commandBlock, DataBlock.dataBlockOffset, this.size);

		
		return commandBlock;
	}
	
	/**
	 * Returns a command for get or dump
	 * @param commandType
	 * @param key
	 * @return
	 */
	public static byte[] getCommand(int commandType, byte[] key) {
		
		byte[] command = new byte[24];
		byte[] bytes;
		
		//Put key
		ByteBuffer b;
		System.arraycopy(key, 0, command, DataBlock.keyOffset, 20);
				
		// Put command
		b = ByteBuffer.allocate(2);
		b.putShort( (short)commandType);
		bytes = b.array();
		System.arraycopy(bytes, 0, command, DataBlock.cmdOffset, 2);
				
		// Put payload size (=0)
		b = ByteBuffer.allocate(2);
		b.putShort((short)0);
		bytes = b.array();
		System.arraycopy(bytes, 0, command, DataBlock.plLenOffset, 2);
		
		return command;
	}
	
}
