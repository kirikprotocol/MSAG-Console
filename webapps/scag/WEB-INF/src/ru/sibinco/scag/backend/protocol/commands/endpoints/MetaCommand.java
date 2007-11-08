package ru.sibinco.scag.backend.protocol.commands.endpoints;

import ru.sibinco.scag.backend.daemon.Command;
import ru.sibinco.scag.backend.endpoints.meta.MetaEndpoint;
import ru.sibinco.lib.SibincoException;

/**
 * Created by IntelliJ IDEA.
 * User: mixim
 * Date: 08.11.2007
 * Time: 15:55:42
 * To change this template use File | Settings | File Templates.
 */
public class MetaCommand  extends Command {

    public MetaCommand(String commandName, String systemId) throws SibincoException {
        super(commandName, systemId);
    }

    protected void fillParams(final MetaEndpoint meta) {

        createStringParam( "systemId", meta.getId() );
        createStringParam( "type", meta.getType() );
        createStringParam( "policy", meta.getPolicy() );
        createBoolParam  ( "enabled", meta.getEnabled() );
   }
}