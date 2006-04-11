/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.protocol.commands.rules;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.backend.daemon.Command;

/**
 * The <code>RemoveRule</code> class represents
 * <p><p/>
 * Date: 06.10.2005
 * Time: 11:44:45
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class RemoveRule extends Command {

    public RemoveRule(final String ruleId, final String transport) throws SibincoException {
        super("removeRule", "command_gw.dtd");
        createStringParam("serviceId", ruleId);
        createStringParam("transport", transport);      
    }
}
