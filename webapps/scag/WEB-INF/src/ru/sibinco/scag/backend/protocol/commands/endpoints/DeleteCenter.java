/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.protocol.commands.endpoints;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.backend.endpoints.centers.Center;
import ru.sibinco.scag.backend.daemon.Command;

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
        super("deleteSmsc", "command_gw.dtd");
        createStringParam("systemId", center.getId());
        createStringParam("bindSystemId", center.getBindSystemId());
        createStringParam("bindPassword", center.getBindPassword());
        createStringParam("systemType", center.getSystemType());      
        createIntParam("timeout", center.getTimeout());
        createStringParam("mode", center.getModeStr());
        createIntParam("providerId", -1/*center.getProvider().getId()*/);
        createStringParam("addressRange", center.getAddressRange());
    }
}
