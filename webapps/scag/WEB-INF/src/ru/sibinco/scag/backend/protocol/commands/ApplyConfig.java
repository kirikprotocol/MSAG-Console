/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.protocol.commands;

import ru.sibinco.scag.backend.daemon.Command;
import ru.sibinco.lib.SibincoException;

/**
 * Created by IntelliJ IDEA.
 * User: igor
 * Date: 17.07.2006
 * Time: 14:08:53
 * To change this template use File | Settings | File Templates.
 */
public class ApplyConfig  extends Command {
  public ApplyConfig() throws SibincoException {
    super("applyConfig","command_gw.dtd");
  }
}
