/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.service;

import org.apache.log4j.Category;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;

import java.util.Arrays;
import java.util.Map;
import java.util.Collections;
import java.util.TreeMap;

/**
 * The <code>ServiceProvider</code> class represents
 * <p><p/>
 * Date: 06.02.2006
 * Time: 15:37:02
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class ServiceProvider {

    org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(ServiceProvider.class);

    public static final int NEW_STATUS = 0;
    public static final int DONE_STATUS = 1;

    private Long id;
    private String name;
    private String description;
    private int status = NEW_STATUS;
    private final Map services = Collections.synchronizedMap(new TreeMap());


    public ServiceProvider(final Long id, final String name) {
        this.id = id;
        this.name = StringEncoderDecoder.encode(name);

    }

    public ServiceProvider(final Long id) {
        this.id = id;
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



    public int getServicesSize() {
        return services == null ? 0 : services.size();
    }

    public Category getLogger() {
        return logger;
    }

    public void setLogger(Category logger) {
        this.logger = logger;
    }

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    public int getStatus() {
        return status;
    }

    public void setStatus(int status) {
        this.status = status;
    }

    public Map getServices() {
        return services;
    }

    

    public String toString() {
        return "ServiceProvider{" +
                "logger=" + logger +
                ", id=" + id +
                ", name='" + name + '\'' +
                ", description='" + description + '\'' +
                ", status=" + status +
                ", services=" + services +
                '}';
    }

    public boolean equals(Object o) {
        if (this == o)
            return true;
        if (o == null || getClass() != o.getClass())
            return false;

        final ServiceProvider that = (ServiceProvider) o;

        if (description != null ? !description.equals(that.description) : that.description != null)
            return false;
        if (!id.equals(that.id))
            return false;
        if (!name.equals(that.name))
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
}
