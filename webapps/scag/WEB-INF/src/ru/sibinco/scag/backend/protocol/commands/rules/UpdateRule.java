/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.protocol.commands.rules;

import ru.sibinco.scag.backend.rules.Rule;
import ru.sibinco.lib.SibincoException;

/**
 * The <code>UpdateRule</code> class represents
 * <p><p/>
 * Date: 06.10.2005
 * Time: 11:51:05
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class UpdateRule extends RuleCommand {

    public UpdateRule(Rule rule) throws SibincoException {
        super("updateRule", "file:///command_gw.dtd");
        fillParams(rule);
    }
}
