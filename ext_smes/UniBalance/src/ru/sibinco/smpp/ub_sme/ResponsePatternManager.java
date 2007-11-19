package ru.sibinco.smpp.ub_sme;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Properties;

public class ResponsePatternManager {
    private final static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(ResponsePatternManager.class);
    private static ResponsePatternManager responsePatternManager = new ResponsePatternManager();

    private Properties properties = new Properties();


    synchronized public String getPattern(String key, String defaultValue) {
        return properties.getProperty(key, defaultValue);
    }

    synchronized public String getPattern(String key) {
        return properties.getProperty(key);
    }

    synchronized public void loadPatterns(File responsePatternConfigFile) {
        Properties patternConfig;
        try {
            patternConfig = new Properties();
            patternConfig.load(new FileInputStream(responsePatternConfigFile));
        } catch (IOException e) {
            logger.error("Exception occured during loading response pattern configuration from " + responsePatternConfigFile.getName(), e);
            throw new InitializationException("Exception occured during loading response pattern configuration.", e);
        }
        properties = patternConfig;
    }

    public static ResponsePatternManager getResponsePatternManager() {
        return responsePatternManager;
    }
}
