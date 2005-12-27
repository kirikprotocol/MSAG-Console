/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package ru.novosoft.util.menu;

import java.io.Serializable;
import java.util.Collection;
import java.util.Iterator;

public class MenuItem implements Serializable {

    public static final int ITEM_ID_TOP = 0;
	public static byte VISIBLE_IN_ALL = 1;
	public static byte VISIBLE_IN_SINGLE = 2;
	public static byte VISIBLE_IN_HA = 3;

	private int id;
    private int parentId;
    private String key;
    private String caption;
    private String uri;
    private String target;
    private String onclick;

	private byte visibleMask = VISIBLE_IN_ALL;
	// 1 - visible in all installation types
	// 2 - visible in single
	// 3 - visible in ha

	private Collection subMenu;
    int subMenuWidth;

    public MenuItem(String key, String uri, byte mask) {
        this.key = key;
        this.uri = uri;
		this.visibleMask = mask;
	}

    public MenuItem(String key, String uri, String target) {
        this.key = key;
        this.uri = uri;
        this.target = target;
    }

    public MenuItem(String key, String uri, Collection subMenu) {
        this.key = key;
        this.uri = uri;
        this.subMenu = subMenu;
    }

    public MenuItem(int id, int parentId, String caption, String uri) {
        this.id = id;
        this.parentId = parentId;
        this.caption = caption;
        this.uri = uri;
    }

    public MenuItem() {
        this.id = 0;
        this.parentId = 0;
        this.key = null;
        this.caption = null;
        this.uri = null;
        this.subMenu = null;
    }

    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    public int getParentId() {
        return parentId;
    }

    public void setParentId(int parentId) {
        this.parentId = parentId;
    }

    public String getKey() {
        return key;
    }

    public void setKey(String key) {
        this.key = key;
    }

    public String getCaption() {
        if (key != null) {
            try {
                return Common.messages.getString(key);
            } catch (Exception e) {
                return key;
            }
        }
        return caption;
    }

    public void setCaption(String caption) {
        this.caption = caption;
    }

    public String getUri() {
        return uri;
    }

    public void setUri(String uri) {
        this.uri = uri;
    }

    public Collection getSubMenu() {
        return subMenu;
    }

    public void setSubMenu(Collection subMenu) {
        this.subMenu = subMenu;
    }

    public int getSubMenuWidth() {
        subMenuWidth = 0;
        Collection items = getSubMenu();
        for(Iterator it = items.iterator(); it.hasNext();){
            MenuItem item = (MenuItem) it.next();
            if(subMenuWidth < (item.getCaption().length() + 2) * 7){
                subMenuWidth = (item.getCaption().length() + 2) * 7;
            }
        }
        return subMenuWidth;
    }

    public void setSubMenuWidth(int subMenuWidth) {
        this.subMenuWidth = subMenuWidth;
    }

    public String getTarget() {
        return target;
    }

    public void setTarget(String target) {
        this.target = target;
    }

    public String getOnclick() {
        return onclick;
    }

    public void setOnclick(String onclick) {
        this.onclick = onclick;
    }

	public byte getVisibleMask() {
		return visibleMask;
	}
}
