package ru.sibinco.scag.backend.protocol.commands.endpoints;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.backend.endpoints.meta.MetaEndpoint;

/**
 * Created by IntelliJ IDEA.
 * User: mixim
 * Date: 08.11.2007
 * Time: 15:49:31
 * To change this template use File | Settings | File Templates.
 */
public class AddMetaEntity  extends MetaCommand{

    public AddMetaEntity(final MetaEndpoint meta) throws SibincoException {
        super("addMetaEntity", "command_gw.dtd");
        fillParams(meta);
        logger.info( "AddMetaEntity");
    }
}
