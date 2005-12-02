/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.sme;

import ru.sibinco.lib.backend.util.StringEncoderDecoder;

/**
 * The <code>Category</code> class represents
 * <p><p/>
 * Date: 24.10.2005
 * Time: 16:54:03
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Category {

    public static final String[] columnNames = {
            "id", "name",
    };
    private long id;
    private String name;

    public Category(final long id, final String name) {
        this.id = id;
        this.name = StringEncoderDecoder.encode(name);
    }

    public long getId() {
        return id;
    }

    public String getName() {
        return name;
    }

    public void setName(final String name) {
        this.name = name;
    }

    public String toString() {
        return "Category{" +
                "id=" + id +
                ", name='" + name + "'" +
                "}";
    }
}