/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.daemon;

import ru.sibinco.scag.backend.service.ServiceCommand;
import ru.sibinco.lib.SibincoException;

/**
 * The <code>CommandRemoveService</code> class represents
 * <p><p/>
 * Date: 19.01.2006
 * Time: 11:04:19
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class CommandRemoveService extends ServiceCommand {

    public CommandRemoveService(String serviceName) throws SibincoException {
        super("remove_service", serviceName);
    }
}
