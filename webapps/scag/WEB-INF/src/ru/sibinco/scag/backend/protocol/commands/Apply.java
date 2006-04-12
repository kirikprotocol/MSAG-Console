package ru.sibinco.scag.backend.protocol.commands;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.backend.daemon.Command;


/**
 * Created by igork Date: 26.05.2004 Time: 16:35:20
 */
public class Apply extends Command {

    private final String applyTarget;

    public Apply(final String applyTarget) throws SibincoException {
        super("apply", "command_gw.dtd");
        this.applyTarget = applyTarget;
        createStringParam("subj", this.applyTarget);
    }

    public String getApplyTarget() {
        return applyTarget;
    }
}
