/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.beans;

/**
 * The <code>EditChildException</code> class represents
 * <p><p/>
 * Date: 26.02.2006
 * Time: 17:27:37
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class EditChildException extends SCAGJspException {
    private final String editId;
    private final String parentId;

    public EditChildException(String editId, String parentId) {
        this.editId = editId;
        this.parentId = parentId;
    }

    public String getEditId() {
        return editId;
    }

    public String getParentId() {
        return parentId;
    }
}
