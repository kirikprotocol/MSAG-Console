/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.routing.http.placement;

import ru.sibinco.lib.SibincoException;

/**
 * The <code>Placement</code> class represents
 * <p><p/>
 * <p/>
 * Date: 04.07.2006
 * Time: 15:21:57
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Placement {

    private String type;
    private String name;


    public Placement(String type, String name) throws SibincoException {
        if (type == null)
            throw new NullPointerException("Type is null");
        if (name == null)
            throw new NullPointerException("Name is null");

        this.type = type;
        this.name = name;
    }

    public String getType() {
        return type.trim();
    }

    public void setType(final String type) {
        this.type = type;
    }

    public String getName() {
        return name.trim();
    }

    public void setName(final String name) {
        this.name = name;
    }

    public String getId() {
        return getName();
    }

    public void setId(final String id) {
        setName(id);
    }

    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        final Placement placement = (Placement) o;

        if (!name.equals(placement.name)) return false;
        if (!type.equals(placement.type)) return false;

        return true;
    }

    public int hashCode() {
        int result;
        result = type.hashCode();
        result = 29 * result + name.hashCode();
        return result;
    }


    public String toString() {
        return "Placement{" +
                "type='" + type + '\'' +
                ", name='" + name + '\'' +
                '}';
    }
}
