package dht;

import java.net.*;

public class NodeIO {
	DHTController controller;
	
	
	public NodeIO(DHTController controller) {
		this.controller = controller;
	
	}
	
	
	public void startNode() {
		//String[] cmd = {"", this.hostIP, this.hostPort, this.serverIP, this.serverPort};
	}
	
	public Socket connectNode() {
		return null;
	}
	
	public void disconnect() {
		
	}
	
	public int sendCommand(byte[] commmand) {
		return 0;
	}
	
	public byte[] readCommand() {
		
		return null;
	}
}
