/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.protocol.commands;

import ru.sibinco.scag.backend.daemon.Command;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;

/**
 * The <code>ServiceCommand</code> class represents
 * <p><p/>
 * Date: 19.01.2006
 * Time: 10:51:25
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class ServiceCommand extends Command {

    public ServiceCommand(String commandName, String serviceId) throws SibincoException {
        super(commandName);
        document.getDocumentElement().setAttribute("service", StringEncoderDecoder.encode(serviceId));
    }
}
