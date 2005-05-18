/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package ru.novosoft.util.menu;

import java.util.ResourceBundle;
import java.util.MissingResourceException;

public class Common {

    private Common() {
    }

    public static ResourceBundle messages = ResourceBundle.getBundle("locales.messages");


    public static String tryGetResourceString(String string) {
        try {
            return Common.messages.getString(string);
        } catch (MissingResourceException e) {
            return string;
        }
    }

}
