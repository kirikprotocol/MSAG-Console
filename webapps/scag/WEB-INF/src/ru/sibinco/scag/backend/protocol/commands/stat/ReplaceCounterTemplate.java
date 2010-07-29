package ru.sibinco.scag.backend.protocol.commands.stat;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.backend.daemon.Command;
import ru.sibinco.scag.backend.stat.counters.Counter;

/**
 * Created by IntelliJ IDEA.
 * User: stepanov
 * Date: 28.07.2010
 * Time: 19:43:05
 * To change this template use File | Settings | File Templates.
 */
public class ReplaceCounterTemplate extends Command {

    public ReplaceCounterTemplate(final Counter counter) throws SibincoException {
        super("replaceCounterTemplate", "command_gw.dtd");
        createStringParam("id", counter.getId());
    }

}
