/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.protocol.commands.endpoints;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.backend.endpoints.centers.Center;

/**
 * The <code>AddCenter</code> class represents
 * <p><p/>
 * Date: 13.10.2005
 * Time: 14:29:47
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class AddCenter  extends CenterCommand{

    public AddCenter(final Center center) throws SibincoException {
        super("addSmsc", "file:///command_gw.dtd");
        fillParams(center);
    }
}

