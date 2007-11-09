package ru.sibinco.scag.backend.protocol.commands.endpoints;

import ru.sibinco.lib.SibincoException;

/**
 * Created by IntelliJ IDEA.
 * User: mixim
 * Date: 08.11.2007
 * Time: 18:55:30
 * To change this template use File | Settings | File Templates.
 */
public class RemoveMetaEndpoint  extends MetaCommand{

    public RemoveMetaEndpoint(final String[] pair) throws SibincoException {
        super("removeMetaEndpoint", "command_gw.dtd");
        fillPair(pair);
    }
}
