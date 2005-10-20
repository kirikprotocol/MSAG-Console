/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.protocol.commands.endpoints;

import ru.sibinco.scag.backend.endpoints.centers.Center;
import ru.sibinco.lib.SibincoException;

/**
 * The <code>UpdateCenter</code> class represents
 * <p><p/>
 * Date: 13.10.2005
 * Time: 17:26:00
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class UpdateCenter  extends CenterCommand{

    public UpdateCenter(final Center center) throws SibincoException {
        super("updateSmsc", "file:///command_gw.dtd");
        fillParams(center);
    }
}