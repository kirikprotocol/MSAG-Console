/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package ru.sibinco.scag.beans.menu;

import java.util.Collection;

/**
 * The <code>Menu</code> class represents of menu instance class
 * wich will be used in this application
 * <p><p/>
 * Date: 04.07.2005
 * Time: 11:45:27
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Menu {

    private Collection menu = null;

    private static Menu instance = new Menu();

    /**
     * Method return instance of Menu object
     */
    public static Menu getInstance() {
        return instance;
    }

    /**
     * Private constructor
     */
    private Menu() {
        menu = (Collection) new TopMenu();
    }

    public Collection getMenu() {
        if (menu == null) {
            menu = (Collection) new TopMenu();
        }
        return menu;
    }

}

