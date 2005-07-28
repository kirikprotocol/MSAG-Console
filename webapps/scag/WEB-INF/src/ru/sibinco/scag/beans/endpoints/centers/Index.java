/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package ru.sibinco.scag.beans.endpoints.centers;

import ru.sibinco.scag.beans.TabledBeanImpl;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.lib.bean.TabledBean;

import java.util.Collection;

/**
 * The <code>Index</code> class represents
 * <p><p/>
 * Date: 15.07.2005
 * Time: 12:08:56
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Index extends TabledBeanImpl implements TabledBean {


    protected Collection getDataSource() {
        return appContext.getCenterManager().getCenters().values();
    }

    protected void delete() throws SCAGJspException {
        //To change body of implemented methods use File | Settings | File Templates.
    }
}
