package ru.sibinco.smpp.ub_sme;

import ru.sibinco.smpp.ub_sme.util.SMCXProfileCommandHandler;

import java.io.IOException;
import java.util.Properties;

/**
 * User: pasha
 * Date: 27.11.2007
 * Time: 21:19:42
 */
public class ProfileAbonentManager {
    private final static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(ProfileAbonentManager.class);
    private String profileHost = null;
    private int profilePort = -1;
    private String profileUser = null;
    private String profilePass = null;
    private boolean lowerEncoding = false;
    private static ProfileAbonentManager profileAbonentManager;
    private int connectTimeout;
    private int readTimeout;

    public ProfileAbonentManager(Properties config) {
        String s = config.getProperty("profile.host");
        if (s == null || s.length() == 0) {
            throw new InitializationException("profile.host property is not defined in config");
        }
        profileHost = s;

        s = config.getProperty("profile.port");
        try {
            profilePort = Integer.parseInt(s);
        } catch (Exception e) {
            throw new InitializationException("profile.port property is wrong or not defined in config");
        }

        s = config.getProperty("profile.username");
        if (s == null || s.length() == 0) {
            throw new InitializationException("profile.username property is not defined in config");
        }
        profileUser = s;

        s = config.getProperty("profile.password");
        if (s == null || s.length() == 0) {
            throw new InitializationException("profile.password property is not defined in config");
        }
        profilePass = s;
        lowerEncoding = Boolean.valueOf(config.getProperty("profile.lower.encoding", "false")).booleanValue();
        connectTimeout = Integer.parseInt(config.getProperty("profile.connect.timeout", "10000"));
        readTimeout = Integer.parseInt(config.getProperty("profile.read.timeout", "10000"));
        if (logger.isDebugEnabled()) {
            logger.debug("ProfileAbonentManager inited");
        }
    }

    public static void init(Properties config) {
        profileAbonentManager = new ProfileAbonentManager(config);
    }

    public static ProfileAbonentManager getProfileAbonentLang() {
        return profileAbonentManager;
    }

    public String getLangProfile(String abonent) {
        SMCXProfileCommandHandler xpcHandler = new SMCXProfileCommandHandler();
        xpcHandler.setConnectTimeout(connectTimeout);
        xpcHandler.setReadTimeout(readTimeout);

        boolean success = false;
        String encoding = null;
        try {

            xpcHandler.connect(profileHost, profilePort);
            xpcHandler.authentication(profileUser, profilePass);
            String command =
                    "view profile " + abonent;
            if (logger.isDebugEnabled()) logger.debug("sending command: " + command);
            SMCXProfileCommandHandler.Response resp = xpcHandler.sendCommand(command);
            success = resp.isSuccess() && "100".equals(resp.getStatus());
            if (success) {
                encoding = getEncoding(resp);
            }

        } catch (Exception e) {
            logger.error("error altering profile for abonent " + abonent, e);
        } finally {
            try {
                xpcHandler.close();
            } catch (IOException ioe) {
                if (logger.isDebugEnabled()) logger.debug("error closing profile handler", ioe);
            }
        }
        return encoding;
    }

    private String getEncoding(SMCXProfileCommandHandler.Response resp) {
        if (resp.getResponseLines() == null || resp.getResponseLines().length < 1) {
            logger.warn("zero sized reply from profile");
            return null;
        }
        String line = resp.getResponseLines()[0];
        int i = line.indexOf("Encoding:");
        if (i == -1) {
            i = line.indexOf("encoding:");
        }
        if (i == -1) {
            logger.error("cannot parse encoding from profile response");
//            dumpResponse(resp);
            return null;
        }
        line = line.substring(i);
        i = line.indexOf(":");
        line = line.substring(++i).trim();
        i = line.indexOf(" ");
        if (i == -1) {
            i = line.length() - 1;
        }
        line = line.substring(0, i).trim();
        if (line.length() == 0) {
            logger.error("cannot parse encoding from profile response");
//            dumpResponse(resp);
            return null;
        }
        if (lowerEncoding) {
            line = line.toLowerCase();
        }
        return line;
    }
}
