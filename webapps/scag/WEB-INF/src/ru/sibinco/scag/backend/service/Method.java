/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.service;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.util.Map;
import java.util.HashMap;

/**
 * The <code>Method</code> class represents
 * <p><p/>
 * Date: 18.01.2006
 * Time: 16:32:05
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Method {

    protected Type type;
    protected String name;
    protected Map params = new HashMap();

    public Method(Element methodElem) {

        name = methodElem.getAttribute("name");
        type = Type.getInstance(methodElem.getAttribute("type"));
        NodeList list = methodElem.getElementsByTagName("param");
        for (int i = 0; i < list.getLength(); i++) {
            Element paramElem = (Element) list.item(i);
            Parameter parameter = new Parameter(paramElem);
            params.put(parameter.getName(), parameter);

        }
    }

    public boolean equals(Object obj) {
        if (obj != null && obj instanceof Method) {
            Method method = (Method) obj;
            return name.equals(method.name) &&
                    type.equals(method.type) &&
                    params.equals(method.params);
        }
        return false;
    }

    public Type getType() {
        return type;
    }

    public String getName() {
        return name;
    }

    public Map getParams() {
        return params;
    }
}
