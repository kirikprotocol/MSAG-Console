/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.daemon;

import ru.sibinco.scag.backend.service.ServiceCommand;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import org.w3c.dom.Element;

/**
 * The <code>CommandSetServiceStartupParameters</code> class represents
 * <p><p/>
 * Date: 19.01.2006
 * Time: 11:05:51
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class CommandSetServiceStartupParameters extends ServiceCommand {

    public CommandSetServiceStartupParameters(String serviceId, int port, String args) throws SibincoException {

        super("set_service_startup_parameters", serviceId);

        Element serviceElem = document.createElement("service");
        serviceElem.setAttribute("id", StringEncoderDecoder.encode(serviceId));
        serviceElem.setAttribute("port", StringEncoderDecoder.encode(String.valueOf(port)));
        serviceElem.setAttribute("args", StringEncoderDecoder.encode(String.valueOf(args)));
        document.getDocumentElement().appendChild(serviceElem);
    }
}
