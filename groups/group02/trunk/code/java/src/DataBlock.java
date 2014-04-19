package dht;

import java.nio.*;

/**
 * DataBlock class represents a data block that can be given to the DHTnode
 **/
public class DataBlock {
	int MAX_BLOCK_SIZE = 65535;
	int CMD_HEADER_LENGHT = 44;
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
	public byte[] getCommandBlock(int commandType) {
		//Command
			// Command Type [2]
			// key [20]
			// payload length [2]
			// datablock [MAX_BLOCK_SIZE = 65535]
		
		byte[] commandBlock = new byte[24 + this.MAX_BLOCK_SIZE];
		
		byte[] bytes;
		ByteBuffer b;
		
		
		// Put command
		b = ByteBuffer.allocate(2);
		b.putInt(commandType);
		bytes = b.array();
		System.arraycopy(bytes, 0, commandBlock, 0, 2);
		
		//Put key
		System.arraycopy(this.blockKey, 0, commandBlock, 2, 20);
		
		// Put payload size
		b = ByteBuffer.allocate(2);
		b.putInt(this.size);
		bytes = b.array();
		System.arraycopy(bytes, 0, commandBlock, 22, 2);
		
		//Put block
		System.arraycopy(this.block, 0, commandBlock, 24, this.size);
		
		
		return commandBlock;
	}
	
	
}
