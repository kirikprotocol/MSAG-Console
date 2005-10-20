/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.protocol.commands.endpoints;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.backend.endpoints.svc.Svc;

/**
 * The <code>AddSvc</code> class represents
 * <p><p/>
 * Date: 06.10.2005
 * Time: 11:24:23
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class AddSvc extends SvcCommand{

    public AddSvc(final Svc svc) throws SibincoException {
        super("addSme", "file:///command_gw.dtd");
        fillParams(svc);
    }
}
