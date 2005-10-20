/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.protocol.commands.rules;

import ru.sibinco.lib.backend.protocol.Command;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.backend.rules.Rule;

/**
 * The <code>RuleCommand</code> class represents
 * <p><p/>
 * Date: 04.10.2005
 * Time: 17:32:35
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class RuleCommand extends Command {

    public RuleCommand(final String commandName, final String systemId) throws SibincoException {
        super(commandName, systemId);
    }

    protected void fillParams(final Rule rule){
        createStringParam("id", String.valueOf(rule.getId().longValue()));
    }
}
