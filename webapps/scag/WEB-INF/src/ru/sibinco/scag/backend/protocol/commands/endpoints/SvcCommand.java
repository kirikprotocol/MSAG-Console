/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.protocol.commands.endpoints;

import ru.sibinco.lib.backend.protocol.Command;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.backend.endpoints.svc.Svc;

/**
 * The <code>SvcCommand</code> class represents
 * <p><p/>
 * Date: 06.10.2005
 * Time: 11:25:53
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class SvcCommand extends Command {

    public SvcCommand(String commandName, String systemId) throws SibincoException {
        super(commandName, systemId);
    }

    protected void fillParams(final Svc svc) {
        createStringParam("systemId", svc.getId());
        createStringParam("password", svc.getPassword());
        createIntParam("timeout", svc.getTimeout());
        createStringParam("mode", svc.getModeStr());
        createIntParam("providerId", svc.getProvider().getId());
   }
}