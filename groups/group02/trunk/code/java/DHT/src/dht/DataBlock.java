package dht;

public class DataBlock {
	String blockName;
	int size;
	int totalBlocks;
	int blockNo;
	
	public DataBlock(String blockName, int size, int totalBlocks, int blockNo) {
		this.blockName = blockName;
		this.size = size;
		this.totalBlocks = totalBlocks;
		this.blockNo = blockNo;
		
	}
	
	public String getName() {
		return this.blockName;
	}
	
	public int getSize() {
		return this.size;
	}
	
	public int getTotalBlocks() {
		return this.totalBlocks;
	}
	public int getBlockNo() {
		return this.blockNo;
	}
}
