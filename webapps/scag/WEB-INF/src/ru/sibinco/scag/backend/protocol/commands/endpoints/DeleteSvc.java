/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.protocol.commands.endpoints;

import ru.sibinco.lib.backend.protocol.Command;
import ru.sibinco.lib.SibincoException;

/**
 * The <code>DeleteSvc</code> class represents
 * <p><p/>
 * Date: 12.10.2005
 * Time: 11:30:10
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class DeleteSvc extends Command
{
  public DeleteSvc(final String svcId) throws SibincoException
  {
    super("deleteSme", "file:///command_gw.dtd");
    createStringParam("systemId", svcId);
  }
}
