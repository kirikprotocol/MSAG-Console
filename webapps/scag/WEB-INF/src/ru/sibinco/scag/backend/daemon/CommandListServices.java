/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.daemon;

import ru.sibinco.lib.SibincoException;

/**
 * The <code>CommandListServices</code> class represents
 * <p><p/>
 * Date: 19.01.2006
 * Time: 10:55:56
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class CommandListServices extends Command {

    public CommandListServices() throws SibincoException {
        super("list_services");
    }
}
