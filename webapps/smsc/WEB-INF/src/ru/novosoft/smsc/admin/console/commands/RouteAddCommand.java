/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 7:06:42 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.route.*;

public class RouteAddCommand extends RouteGenCommand
{
    private boolean bill = true;
    private boolean arc = true;
    private boolean allow = true;
    private int serviceid;
    private int priority;

    public void setBill(boolean bill) {
        this.bill = bill;
    }
    public void setArc(boolean arc) {
        this.arc = arc;
    }
    public void setAllow(boolean allow) {
        this.allow = allow;
    }
    public void setServiceId(int serviceid) {
        this.serviceid = serviceid;
    }
    public void setPriority(int priority) {
        this.priority = priority;
    }

    public void process(CommandContext ctx)
    {
        String out = "Route '"+route+"'";
        try
        {
            RouteList list =  ctx.getSmsc().getRoutes();
            Route smscRoute = list.get(route);
            if (smscRoute != null) {
                ctx.setMessage(out+" already exists");
                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                return;
            }

            SourceList srcList = new SourceList();
            for (int i=0; i<srcs.size(); i++) {
                Object obj = srcs.get(i);
                if (obj != null && obj instanceof RouteSrcDef) {
                    RouteSrcDef def = (RouteSrcDef)obj;
                    if (def.getType() == RouteSrcDef.TYPE_MASK) {
                        srcList.add(new Source(new Mask(def.getSrc())));
                    } else if (def.getType() == RouteSrcDef.TYPE_SUBJECT) {
                        Subject subj = ctx.getSmsc().getSubjects().get(def.getSrc());
                        if (subj == null) {
                            ctx.setMessage("Subject '"+def.getSrc()+"' in src definition not found. Couldn't add "+out);
                            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                            return;
                        }
                        srcList.add(new Source(subj));
                    } else {
                        ctx.setMessage("Unsupported src definition for "+out);
                        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                        return;
                    }
                }
            }

            DestinationList dstList = new DestinationList();
            for (int i=0; i<dsts.size(); i++) {
                Object obj = dsts.get(i);
                if (obj != null && obj instanceof RouteDstDef) {
                    RouteDstDef def = (RouteDstDef)obj;
                    SME sme = ctx.getSmsc().getSmes().get(def.getSmeId());
                    if (sme == null) {
                        ctx.setMessage("SME '"+def.getSmeId()+"' in dst definition not found. Couldn't add "+out);
                        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                        return;
                    }
                    if (def.getType() == RouteDstDef.TYPE_MASK) {
                        dstList.add(new Destination(new Mask(def.getDst()), sme));
                    } else if (def.getType() == RouteDstDef.TYPE_SUBJECT) {
                        Subject subj = ctx.getSmsc().getSubjects().get(def.getDst());
                        if (subj == null) {
                            ctx.setMessage("Subject '"+def.getDst()+"' in dst definition not found. Couldn't add "+out);
                            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                            return;
                        }
                        dstList.add(new Destination(subj, sme));
                    } else {
                        ctx.setMessage("Unsupported dst definition for "+out);
                        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                        return;
                    }
                }
            }

            smscRoute = new Route(route, priority, allow, bill, arc,
                                  serviceid, srcList, dstList);
            list.put(smscRoute);
        }
        catch (Exception e) {
            ctx.setMessage("Couldn't add "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            return;
        }

        ctx.setMessage(out+" added");
        ctx.setStatus(CommandContext.CMD_OK);
    }

    public String getId() {
        return "ROUTE_ADD";
    }

}

