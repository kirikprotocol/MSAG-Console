package ru.novosoft.smsc.util;

import ru.novosoft.smsc.util.config.Config;

import java.io.IOException;
import java.util.*;

/**
 * Created by Serge Lugovoy.
 * Date: Mar 13, 2003
 * Time: 2:57:39 PM
 */

public class LocaleMessages {
    private static Map localeMessages = new HashMap();

    public static Config configFile = null;
    public static String DEFAULT_PREFERRED_LANGUAGE = "en";
    public static String SMSC_BUNDLE_NAME = "locales.messages";

    public static void init(Config conf) throws Exception, IOException {
        configFile = conf;
        DEFAULT_PREFERRED_LANGUAGE = configFile.getString("conf.defaultLang");
        SMSC_BUNDLE_NAME = configFile.getString("conf.bundleName");
    }

    public static Set getStrings(Locale locale, String prefix) {
        Set result = new HashSet();
        final int prefixLength = prefix.length();
        final ResourceBundle res = getResourceBundle(locale);
        if (res != null) {
            final Enumeration keys = res.getKeys();
            while (keys.hasMoreElements()) {
                String key = (String) keys.nextElement();
                if (key.startsWith(prefix)) {
                    result.add(key.substring(prefixLength));
                }
            }
        }
        return result;
    }

    public static String getString(Locale locale, String key) {
        String result = "";
        ResourceBundle res = getResourceBundle(locale);
        if (res != null) {
            try {
                result = res.getString(key);
            }
            catch (MissingResourceException ex) {
                try {
//                    System.err.println("locale resource not found: key=\"" + key + "\", locale=\"" + locale.getLanguage() + "\"");
                    res = getResourceBundle(new Locale(LocaleMessages.DEFAULT_PREFERRED_LANGUAGE));
                    result = res.getString(key);
                }
                catch (MissingResourceException exc) {
                    result = /*"ResourceNotFound: " + */key;
                }
            }
        }
        return result;
    }

    public static ResourceBundle getResourceBundle(Locale locale) {
        ResourceBundle res = (ResourceBundle) localeMessages.get(locale);
        if (res == null) res = (ResourceBundle) localeMessages.get(new Locale(locale.getLanguage(), locale.getCountry()));
        if (res == null) res = (ResourceBundle) localeMessages.get(new Locale(locale.getLanguage()));
        if (res == null) {
            try {
                res = ResourceBundle.getBundle(SMSC_BUNDLE_NAME, locale);
                localeMessages.put(locale, res);
            }
            catch (MissingResourceException e) {
                System.err.println("locale resource bundle file not found: locale=\"" + locale.getLanguage() + "\"");
                return null;
            }
        }
        return res;
    }
}