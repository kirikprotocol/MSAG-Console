package ru.sibinco.scag.backend.protocol.commands.endpoints;

import ru.sibinco.scag.backend.endpoints.svc.Svc;
import ru.sibinco.lib.SibincoException;

/**
 * Created by IntelliJ IDEA.
 * User: mixim
 * Date: 09.11.2007
 * Time: 13:19:10
 * To change this template use File | Settings | File Templates.
 */
public class UpdateSvc extends SvcCommand {

    public UpdateSvc(final Svc svc) throws SibincoException {
        super("updateSme", "command_gw.dtd");
        fillParams(svc);
    }
}