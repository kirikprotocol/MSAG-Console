/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 7:06:42 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;

import java.util.ArrayList;

public abstract class RouteGenCommand implements Command
{
    protected String route;
    protected boolean bill = true;
    protected boolean arc = true;
    protected boolean allow = true;
    protected int serviceid;
    protected int priority;

    protected ArrayList srcs = new ArrayList();
    protected ArrayList dsts = new ArrayList();

    public void setRoute(String route) {
        this.route = route;
    }
    public void setBill(boolean bill) {
        this.bill = bill;
    }
    public void setArc(boolean arc) {
        this.arc = arc;
    }
    public void setAllow(boolean allow) {
        this.allow = allow;
    }
    public void setServiceid(int serviceid) {
        this.serviceid = serviceid;
    }
    public void setPriority(int priority) {
        this.priority = priority;
    }

}

