package DHT;

import java.util.*;
import java.io.*;

public class Runtest {
    
    public static void main(String[] args) {
        try {
            String[] cmd = {"/home/henri/Desktop/DHT/group02/trunk/code/c/dhtnode", "localhost", "3000", "localhost", "1234"};
            List cmdlist = Arrays.asList(cmd);
            ProcessBuilder builder = new ProcessBuilder(cmdlist);
            builder.redirectErrorStream(true);  // Merge stdout and stderr
            Process process = builder.start();

            InputStream is = process.getInputStream();  // Input: stream from external process to Java
            InputStreamReader isr = new InputStreamReader(is);
            BufferedReader br = new BufferedReader(isr);
            String line;
            while ((line = br.readLine()) != null) {
                System.out.println(line);
            }

            int exitVal = process.waitFor();    // Terminating Java process appears to terminate external process too
            System.out.println("Exit value: " + exitVal);
        } catch (Exception e) {
            e.printStackTrace();
        }
        
    }
}