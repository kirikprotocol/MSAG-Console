/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.beans;

/**
 * The <code>AddSiteSubjectException</code> class represents
 * <p><p/>
 * Date: 26.05.2006
 * Time: 13:22:24
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class AddSiteSubjectException extends SCAGJspException {

    private String transportId;
    private String subjectType;


    public AddSiteSubjectException(String transportId, String subjectType) {
        this.transportId = transportId;
        this.subjectType = subjectType;
    }

    public String getTransportId() {
        return transportId;
    }

    public String getSubjectType() {
        return subjectType;
    }
}
