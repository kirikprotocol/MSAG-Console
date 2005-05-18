/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package ru.novosoft.util.menu;

import java.util.Collection;


public class Menu {

    private Collection menu = null;

    private static Menu instance = new Menu();

    public static Menu getInstance() {
        return instance;
    }

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
