/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.daemon;

import ru.sibinco.scag.backend.protocol.commands.ServiceCommand;
import ru.sibinco.lib.SibincoException;

/**
 * The <code>CommandStartService</code> class represents
 * <p><p/>
 * Date: 19.01.2006
 * Time: 11:12:33
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class CommandStartService extends ServiceCommand {

    public CommandStartService(String serviceName) throws SibincoException {
        super("start_service", serviceName);
    }
}
