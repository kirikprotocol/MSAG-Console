package ru.novosoft.smsc.admin.console.commands.closedgroup;

import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.closedgroups.ClosedGroup;

import java.util.Iterator;

/**
 * Created by IntelliJ IDEA.
 * User: starkom
 * Date: 26.02.2006
 * Time: 15:04:17
 * To change this template use File | Settings | File Templates.
 */
public abstract class ClosedGroupGenCommand extends CommandClass {
    protected int groupId = -1;
    protected String groupName = "";
    protected String groupIdOrName = "";
    protected String name = "";
    protected String descr = "";

    protected boolean isGroupNameOrId = false;
    protected boolean isDescr = false;

    public void process(CommandContext ctx) {
        if (isGroupNameOrId)
            for (Iterator i = ctx.getClosedGroupManager().getClosedGroups().iterator(); i.hasNext();) {
                ClosedGroup cg = (ClosedGroup) i.next();
                if (Integer.toString(cg.getId()).equals(groupIdOrName) || (cg.getName().equals(groupIdOrName))) {
                    groupId = cg.getId();
                    groupName = cg.getName();
                    break;
                }
            }
    }

    public void setClosedGroupId(String idOrName) {
        groupIdOrName = idOrName;
        isGroupNameOrId = true;
    }

    public void setDescr(String descr) {
        this.descr = descr;
        isDescr = true;
    }

    public void setClosedGroupName(String name) {
        this.groupName = name;
    }
}
