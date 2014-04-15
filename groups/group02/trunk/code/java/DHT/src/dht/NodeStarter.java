package dht;

import java.net.Socket;

public class NodeStarter {
	
	public NodeStarter() {
        
    }
	
	public Socket start() {
		this.startNode(); /* (3) */
		Socket socketToNode = new Socket();
		
		return socketToNode;
	}
	
	
    native void startNode(); /* (1) */
    static {
        System.loadLibrary("DHTnode"); /* (2) */
    }
    
	
}
