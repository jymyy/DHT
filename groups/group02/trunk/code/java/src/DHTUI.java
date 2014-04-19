package dht;
import java.util.*;

public class DHTUI {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		DHTController ctr = new DHTController("foo", "foo", "foo", "foo");
		//ctr.putFile("/Users/tommi/DHTworkspace/DHT/src/dht/faili.rtf", "ploppi");
		System.out.println(ctr.dumpFile("ploppi"));
		System.out.println(ctr.getFile("ploppi", "/Users/tommi/DHTworkspace/DHT/src/dht/"));
	}

}
