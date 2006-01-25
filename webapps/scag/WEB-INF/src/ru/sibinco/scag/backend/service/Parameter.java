/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.service;

import org.w3c.dom.Element;

/**
 * The <code>Parameter</code> class represents
 * <p><p/>
 * Date: 18.01.2006
 * Time: 16:51:18
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Parameter {
    protected Type type;
    protected String name;

    public Parameter(Element paramElem) {
        name = paramElem.getAttribute("name");
        type = Type.getInstance(paramElem.getAttribute("type"));
    }

    public boolean equals(Object obj) {
        if (obj != null && obj instanceof Parameter) {
            Parameter parameter = (Parameter) obj;
            return parameter.name.equals(name) && parameter.equals(type);
        }
        return false;
    }

    public Type getType() {
        return type;
    }

    public String getName() {
        return name;
    }
}
