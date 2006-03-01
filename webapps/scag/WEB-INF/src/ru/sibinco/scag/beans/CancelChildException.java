/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.beans;

/**
 * The <code>CancelChildException</code> class represents
 * <p><p/>
 * Date: 26.02.2006
 * Time: 17:57:50
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class CancelChildException  extends SCAGJspException{

    private final String path;

    public CancelChildException(String path) {
        this.path = path;
    }

    public String getPath() {
        return path;
    }

}
