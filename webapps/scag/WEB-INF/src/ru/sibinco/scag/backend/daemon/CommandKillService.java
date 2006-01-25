/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.daemon;

import ru.sibinco.scag.backend.service.ServiceCommand;
import ru.sibinco.lib.SibincoException;

/**
 * The <code>CommandKillService</code> class represents
 * <p><p/>
 * Date: 19.01.2006
 * Time: 10:50:23
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class CommandKillService extends ServiceCommand {

    public CommandKillService(String serviceName) throws SibincoException {
        super("kill_service", serviceName);
    }
}
