/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.protocol.commands.rules;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.backend.rules.Rule;

/**
 * The <code>AddRule</code> class represents
 * <p><p/>
 * Date: 05.10.2005
 * Time: 13:53:04
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class AddRule extends RuleCommand{

    protected AddRule(Rule rule) throws SibincoException {
        super("addRule", "file:///command_gw.dtd");
        fillParams(rule);
    }
}
