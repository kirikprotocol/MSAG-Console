/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.beans;

/**
 * The <code>AddChildException</code> class represents
 * <p><p/>
 * Date: 26.02.2006
 * Time: 13:02:08
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class AddChildException extends SCAGJspException {

    private final String additDir;
    private final String parentId;

    public AddChildException(String additDir, String parentId) {
        this.additDir = additDir;
        this.parentId = parentId;
    }

    public String getAdditDir() {
        return additDir;
    }

    public String getParentId() {
        return parentId;
    }

}
