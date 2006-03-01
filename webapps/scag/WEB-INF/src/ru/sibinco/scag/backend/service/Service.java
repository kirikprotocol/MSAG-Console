/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.service;

import ru.sibinco.lib.backend.util.StringEncoderDecoder;

/**
 * The <code>Service</code> class represents
 * <p><p/>
 * Date: 14.02.2006
 * Time: 13:32:20
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Service {

    private Long id;
    private String name;
    private String description;

    public Service(final Long id, final String name) {
        this.id = id;
        this.name = StringEncoderDecoder.encode(name);
    }

    public Service(Long id) {
        this.id = id;
    }

    public Service(String name, String description) {
        this.name = name;
        this.description = description;
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

    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        final Service service = (Service) o;

        if (description != null ? !description.equals(service.description) : service.description != null)
            return false;
        if (!id.equals(service.id))
            return false;
        if (!name.equals(service.name))
            return false;

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
        return "Service{" +
                "id=" + id +
                ", name='" + name + '\'' +
                ", description='" + description + '\'' +
                '}';
    }
}
