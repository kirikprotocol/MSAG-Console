/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 7:06:42 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


import ru.novosoft.smsc.admin.console.Command;

import java.util.ArrayList;

public abstract class RouteGenCommand implements Command
{
    protected String route = null;

    protected ArrayList srcs = new ArrayList();
    protected ArrayList dsts = new ArrayList();

    public void setRoute(String route) {
        this.route = route;
    }
    public void addSrcDef(RouteSrcDef def) {
        srcs.add(def);
    }
    public void addDstDef(RouteDstDef def) {
        dsts.add(def);
    }

}

