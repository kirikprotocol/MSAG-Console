package ru.sibinco.scag.backend.protocol.commands.endpoints;

import ru.sibinco.scag.backend.endpoints.meta.MetaEndpoint;
import ru.sibinco.lib.SibincoException;

/**
 * Created by IntelliJ IDEA.
 * User: mixim
 * Date: 08.11.2007
 * Time: 16:13:27
 * To change this template use File | Settings | File Templates.
 */
public class DeleteMetaEntity extends MetaCommand {
    public DeleteMetaEntity(final MetaEndpoint meta) throws SibincoException {
        super("deleteMetaEntity", "command_gw.dtd");
        fillParams(meta);
    }
}
