/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.operators;

import java.util.Map;
import java.util.Collections;
import java.util.TreeMap;

/**
 * The <code>Operator</code> class represents
 * <p><p/>
 * Date: 02.03.2006
 * Time: 13:54:12
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Operator {

    private Long id;
    private String name;
    private String description;

    private final Map masks = Collections.synchronizedMap(new TreeMap());

    public Operator(Long id) {
        this.id = id;
    }

    public Operator(Long id, String name) {
        this.id = id;
        this.name = name;
    }


    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    public Map getMasks() {
        return masks;
    }


    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        final Operator operator = (Operator) o;

        if (description != null ? !description.equals(operator.description) : operator.description != null) return false;
        if (!id.equals(operator.id)) return false;
        if (!name.equals(operator.name)) return false;

        return true;
    }

    public int hashCode() {
        int result;
        result = id.hashCode();
        result = 29 * result + name.hashCode();
        result = 29 * result + (description != null ? description.hashCode() : 0);
        return result;
    }


    public String toString() {
        return "Operator{" +
                "id=" + id +
                ", name='" + name + '\'' +
                ", description='" + description + '\'' +
                ", masks=" + masks +
                '}';
    }
}
