package ru.sibinco.scag.backend.protocol.commands.stat;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.backend.daemon.Command;
import ru.sibinco.scag.backend.stat.counters.CATable;

/**
 * Created by IntelliJ IDEA.
 * User: stepanov
 * Date: 09.08.2010
 * Time: 12:00:54
 * To change this template use File | Settings | File Templates.
 */
public class ReplaceCounterActions extends Command {

    public ReplaceCounterActions(final CATable ca_table) throws SibincoException {
        super("replaceCounterActions", "command_gw.dtd");
        createStringParam("id", ca_table.getId());
    }
}
