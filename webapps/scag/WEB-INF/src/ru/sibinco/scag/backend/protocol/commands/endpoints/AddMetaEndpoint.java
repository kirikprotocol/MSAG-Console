package ru.sibinco.scag.backend.protocol.commands.endpoints;

import ru.sibinco.lib.SibincoException;

/**
 * Created by IntelliJ IDEA.
 * User: mixim
 * Date: 08.11.2007
 * Time: 18:47:16
 * To change this template use File | Settings | File Templates.
 */
public class AddMetaEndpoint  extends MetaCommand{

    public AddMetaEndpoint(final String[] pair) throws SibincoException {
        super("addMetaEndpoint", "command_gw.dtd");
        fillPair(pair);
    }
}
