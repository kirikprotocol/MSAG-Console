/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.daemon;

import ru.sibinco.scag.backend.service.ServiceInfo;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import org.w3c.dom.Element;

/**
 * The <code>CommandAddService</code> class represents
 * <p><p/>
 * Date: 19.01.2006
 * Time: 10:40:52
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class CommandAddService extends Command {

    protected CommandAddService(final ServiceInfo serviceInfo) throws SibincoException {

        super("add_service");
        logger.debug("Add services \"" + serviceInfo.getId() + "\" (" + serviceInfo.getHost() + ")");

        final Element serviceElem = document.createElement("service");
        serviceElem.setAttribute("id", StringEncoderDecoder.encode(serviceInfo.getId()));
        serviceElem.setAttribute("args", StringEncoderDecoder.encode(serviceInfo.getArgs()));
        serviceElem.setAttribute("status", serviceInfo.getStatusStr());
        serviceElem.setAttribute("autostart", serviceInfo.isAutostart() ? "true" : "false");
    }
}
