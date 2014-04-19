package DHT.src.dht;

import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Date;


/**
 * Static class for logging info to stderr.
 *
 * Usage:
 * Each class should declare a static string TAG (max length 10) that identifies a given class.
 * Logging itself is done by calling Log.level methods with TAG and desired message and variables.
 */
public class Log {
    static String logFormat = "%s | %-5s | %-10s | ";
    static SimpleDateFormat sdf = new SimpleDateFormat("HH:mm:ss");

    private static void logPrint (String tag, String level, Object... args) {
        System.err.format(logFormat, getTimeStr(), level, tag);
        System.err.format(args[0] + "\n", Arrays.copyOfRange(args, 1, args.length));
    }

    private static String getTimeStr() {
        return sdf.format(new Date());
    }

    public static void error(String tag, Object... args) {
        logPrint(tag, "ERROR", args);
    }

    public static void warn(String tag, Object... args) {
        logPrint(tag, "WARN", args);
    }

    public static void info(String tag, Object... args) {
        logPrint(tag, "INFO", args);
    }

    public static void debug(String tag, Object... args) {
        logPrint(tag, "DEBUG", args);
    }
}
