/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.protocol.commands.rules;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.protocol.Command;
import ru.sibinco.scag.backend.rules.Rule;

/**
 * The <code>RemoveRule</code> class represents
 * <p><p/>
 * Date: 06.10.2005
 * Time: 11:44:45
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class RemoveRule extends Command {

    public RemoveRule(final String ruleId) throws SibincoException {
        super("removeRule", "file:///command_gw.dtd");
        createStringParam("ruleId", ruleId);
    }
}
