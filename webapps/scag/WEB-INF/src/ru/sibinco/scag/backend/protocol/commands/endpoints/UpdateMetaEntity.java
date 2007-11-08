package ru.sibinco.scag.backend.protocol.commands.endpoints;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.backend.endpoints.meta.MetaEndpoint;

/**
 * Created by IntelliJ IDEA.
 * User: mixim
 * Date: 08.11.2007
 * Time: 16:13:51
 * To change this template use File | Settings | File Templates.
 */
public class UpdateMetaEntity extends MetaCommand{

    public UpdateMetaEntity(final MetaEndpoint meta) throws SibincoException {
        super("updateMetaEntity", "command_gw.dtd");
        fillParams(meta);
    }
}
