/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.protocol.commands.endpoints;

import ru.sibinco.lib.backend.protocol.Command;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.backend.endpoints.centers.Center;

/**
 * The <code>CenterCommand</code> class represents
 * <p><p/>
 * Date: 13.10.2005
 * Time: 14:30:44
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class CenterCommand  extends Command {

    public CenterCommand(String commandName, String systemId) throws SibincoException {
        super(commandName, systemId);
    }

    protected void fillParams(final Center center) {
        createStringParam("systemId", center.getId());
        createStringParam("bindSystemId", center.getBindSystemId());
        createStringParam("bindPassword", center.getBindPassword());
        createIntParam("timeout", center.getTimeout());
        createStringParam("mode", center.getModeStr());
        createStringParam("host", center.getHost());
        createIntParam("port", center.getPort());
        createStringParam("altHost", center.getAltHost());
        createIntParam("altPort", center.getAltPort());
        createIntParam("uid", center.getUid());
        createIntParam("providerId", center.getProvider().getId());
   }
}