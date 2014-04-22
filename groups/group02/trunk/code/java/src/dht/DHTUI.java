package dht;

import java.util.*;

public class DHTUI {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		DHTController ctr = new DHTController("localhost", "2000", "localhost", "1234");
		//ctr.putFile("/Users/tommi/DHTworkspace/DHT/src/dht/faili.rtf", "ploppi");
		//System.out.println(ctr.dumpFile("ploppi"));
		System.out.println(ctr.getFile("ploppi", "/Users/tommi/DHTworkspace/DHT/src/dht/"));
	}

}
