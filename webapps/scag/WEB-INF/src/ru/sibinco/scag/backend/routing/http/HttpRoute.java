/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.routing.http;

import org.apache.log4j.Logger;
import ru.sibinco.scag.backend.service.Service;

/**
 * The <code>HttpRoute</code> class represents
 * <p><p/>
 * Date: 06.05.2006
 * Time: 15:17:58
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class HttpRoute {

    private final Logger logger = Logger.getLogger(this.getClass());

    private String name;
    private Service service;
    private boolean enabled = false;



    

    public String getName() {
        return name;
    }

    public void setName(final String name) {
        this.name = name;
    }

    public Service getService() {
        return service;
    }

    public void setService(final Service service) {
        this.service = service;
    }

    public boolean isEnabled() {
        return enabled;
    }

    public void setEnabled(final boolean enabled) {
        this.enabled = enabled;
    }

}
