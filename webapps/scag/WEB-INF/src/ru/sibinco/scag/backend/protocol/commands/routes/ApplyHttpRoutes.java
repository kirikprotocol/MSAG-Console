/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.protocol.commands.routes;

import ru.sibinco.scag.backend.protocol.commands.endpoints.CenterCommand;
import ru.sibinco.lib.SibincoException;

/**
 * The <code>ApplyHttpRoutes</code> class represents
 * <p><p/>
 * Date: 30.06.2006
 * Time: 17:07:53
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class ApplyHttpRoutes  extends CenterCommand {

    public ApplyHttpRoutes() throws SibincoException {
        super("applyHttpRoutes","command_gw.dtd");
    }
}
