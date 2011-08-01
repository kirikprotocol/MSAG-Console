package mobi.eyeline.dcpgw;

import mobi.eyeline.dcpgw.exeptions.InitializationException;
import org.apache.log4j.Logger;

import java.util.Properties;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry  Nikolaevich
 * Date: 30.07.11
 * Time: 22:40
 */
public class Utils {

    private static Logger log = Logger.getLogger(Utils.class);

    public static String getProperty(Properties properties, String name) throws InitializationException {
        String s = properties.getProperty(name);
        if (s == null || s.isEmpty()) {
            log.error("Property " + name + " is not defined or is empty.");
            throw new InitializationException("Property '" + name + "' is not defined or is empty.");
        }
        return s;
    }

    public static String getProperty(Properties properties, String name, String default_value){
        String s = properties.getProperty(name);
        if (s == null || s.isEmpty()) {
            log.error("Property '" + name + "' is not defined or is empty, set default value to '" + s+"'.");
            return default_value;
        } else {
            log.debug("Set property '"+name+"' to '"+s+"'.");
            return s;
        }
    }

    public static int getProperty(Properties properties, String name, int default_value){
        String s = properties.getProperty(name);
        if (s == null || s.isEmpty()) {
            log.error("Property " + name + " is not defined or is empty, set default value to " + default_value);
            return default_value;
        } else {
            log.debug("Set property '"+name+"' to '"+s+"'.");
            return Integer.parseInt(s);
        }

    }

}
