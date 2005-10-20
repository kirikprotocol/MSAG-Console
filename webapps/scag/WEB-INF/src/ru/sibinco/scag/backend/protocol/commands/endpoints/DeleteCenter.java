/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.protocol.commands.endpoints;

import ru.sibinco.lib.backend.protocol.Command;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.backend.endpoints.centers.Center;

/**
 * The <code>DeleteCenter</code> class represents
 * <p><p/>
 * Date: 14.10.2005
 * Time: 11:31:00
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class DeleteCenter extends Command {
    public DeleteCenter(final Center center) throws SibincoException {
        super("deleteSmsc", "file:///command_gw.dtd");
        createStringParam("systemId", center.getId());
        createStringParam("password", center.getPassword());
        createIntParam("timeout", center.getTimeout());
        createStringParam("mode", center.getModeStr());
        createIntParam("providerId", center.getProvider().getId());
    }
}
