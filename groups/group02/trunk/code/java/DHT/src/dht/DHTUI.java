package dht;
import java.util.*;

public class DHTUI {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		Runtime runtime = Runtime.getRuntime();
		
		try {
			Process process = runtime.exec("/tommi/group02/trunk/code/c/dhtnode");
		}
		catch(Exception e) {
			e.printStackTrace();
			System.out.println("");
		}
	}

}
