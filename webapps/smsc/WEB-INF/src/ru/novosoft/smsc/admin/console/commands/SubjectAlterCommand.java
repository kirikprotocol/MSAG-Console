/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 7:05:13 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.admin.route.Subject;
import ru.novosoft.smsc.admin.route.MaskList;
import ru.novosoft.smsc.admin.route.Mask;

import java.util.Iterator;

public class SubjectAlterCommand extends SubjectGenCommand
{
    public final static int ACTION_ADD = 10;
    public final static int ACTION_DELETE = 20;

    private int action = ACTION_ADD;

    public void setActionAdd() {
        this.action = ACTION_ADD;
    }
    public void setActionDelete() {
        this.action = ACTION_DELETE;
    }

    private boolean containsInMasks(String mask) {
        return masks.contains(mask);
    }
    public void process(CommandContext ctx)
    {
        String out = "Subject '"+subject+"'";
        try {
            Subject smscSubject = ctx.getRouteSubjectManager().getSubjects().get(subject);
            if (smscSubject == null) {
                ctx.setMessage(out+" not found");
                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                return;
            }
            if (defaultSmeId != null) {
                SME sme = ctx.getSmeManager().getSmes().get(defaultSmeId);
                if (sme != null) {
                    smscSubject.setDefaultSme(sme);
                    ctx.setMessage(out+" altered. Default sme changed");
                    ctx.setStatus(CommandContext.CMD_OK);
                } else {
                    ctx.setMessage("Couldn't alter "+out+
                                   ". No default SME for id '"+defaultSmeId+"'");
                    ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                }
            } else if (action == ACTION_ADD) {
                MaskList list = smscSubject.getMasks();
                for (int i=0; i<masks.size(); i++) {
                    Object obj = masks.get(i);
                    if (obj != null && obj instanceof String) {
                        list.add(new Mask((String)obj));
                    }
                }
                ctx.setMessage(out+" altered. Mask(s) added");
                ctx.setStatus(CommandContext.CMD_OK);
            } else if (action == ACTION_DELETE) {
                MaskList oldList = smscSubject.getMasks();
                MaskList newList = new MaskList();
                Iterator i = oldList.iterator();
                while (i.hasNext()) {
                    Mask mask = (Mask)i.next();
                    if (mask != null && !containsInMasks(mask.getMask())) {
                        newList.add(mask);
                    }
                }
                smscSubject.setMasks(newList);
                ctx.setMessage(out+" altered. Mask(s) deleted");
                ctx.setStatus(CommandContext.CMD_OK);
            } else {
                ctx.setMessage("Couldn't alter "+out+". Cause: Unknown action");
                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            }
        } catch (Exception e) {
            ctx.setMessage("Couldn't alter "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "SUBJECT_ALTER";
    }

}

