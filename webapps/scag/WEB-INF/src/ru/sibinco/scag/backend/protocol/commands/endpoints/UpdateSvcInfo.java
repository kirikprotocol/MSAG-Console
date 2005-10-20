/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.protocol.commands.endpoints;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.backend.endpoints.svc.Svc;

/**
 * The <code>UpdateSvcInfo</code> class represents
 * <p><p/>
 * Date: 12.10.2005
 * Time: 11:00:33
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class UpdateSvcInfo extends SvcCommand {

    public UpdateSvcInfo(final Svc svc) throws SibincoException {
        super("updateSmeInfo", "file:///command_gw.dtd");
        fillParams(svc);
    }
}
