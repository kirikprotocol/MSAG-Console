/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.daemon;

import ru.sibinco.scag.backend.protocol.commands.ServiceCommand;
import ru.sibinco.lib.SibincoException;

/**
 * The <code>CommandShutdownService</code> class represents
 * <p><p/>
 * Date: 19.01.2006
 * Time: 11:11:07
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class CommandShutdownService extends ServiceCommand {

    public CommandShutdownService(String serviceName) throws SibincoException {
        super("shutdown_service", serviceName);
    }
}
