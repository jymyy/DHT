package dht;

import java.net.Socket;

public class DHTController {
	
	NodeStarter nodeStarter;
	Socket nodeSocket;
	
	DHTController() {
		nodeStarter = new NodeStarter();
		nodeSocket = nodeStarter.start();
		
	}
	
	public int sendToNode() {
		return 0;
	}
	
	public byte[] readFromNode() {
		return null;
	}
	
	
	public int putFile(DataBlock block) {
		return 0;
	}
	
	public int getFile(DataBlock block) {
		return 0;
	}
	
	public int dumpFile(DataBlock block) {
		return 0;
	}
	
	
	public int putBlock(DataBlock block) {
		return 0;
	}
	
	public int getBlock(DataBlock block) {
		return 0;
	}
	
	public int dumpBlock(DataBlock block) {
		return 0;
	}
	
	public int disconnect(DataBlock block) {
		return 0;
	}
	
	
	
	public byte[] sha1() {
		return null;
	}
	
}
