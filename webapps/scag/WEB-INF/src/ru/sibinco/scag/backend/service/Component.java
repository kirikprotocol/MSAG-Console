/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.service;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.util.Map;
import java.util.HashMap;

/**
 * The <code>Component</code> class represents
 * <p><p/>
 * Date: 18.01.2006
 * Time: 16:28:31
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Component {

    protected String name;
    protected Map methods = new HashMap();

    public Component(Element element) {
        this.name = element.getAttribute("name");
        NodeList list = element.getElementsByTagName("method");
        for (int i = 0; i < list.getLength(); i++) {
            Element methodElem = (Element) list.item(i);
            Method newMethod = new Method(methodElem);
            methods.put(newMethod.getName(), newMethod);
        }
    }

    public String getName() {
        return name;
    }

    public Map getMethods() {
        return methods;
    }
}
