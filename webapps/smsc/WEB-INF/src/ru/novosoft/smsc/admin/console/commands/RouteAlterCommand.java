/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 7:10:13 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.route.*;

public class RouteAlterCommand extends RouteGenCommand
{
    public final static byte ACTION_ADD = 10;
    public final static byte ACTION_DEL = 20;

    public final static byte TARGET_SRC = 30;
    public final static byte TARGET_DST = 40;

    private byte action = ACTION_ADD;
    private byte target = TARGET_SRC;

    private boolean bill = true;
    private boolean arc = true;
    private boolean allow = true;
    private int serviceid;
    private int priority;

    private boolean setBill = false;
    private boolean setArc = false;
    private boolean setAllow = false;
    private boolean setId = false;
    private boolean setPri = false;

    public void setAction(byte action) {
        this.action = action;
    }
    public void setTarget(byte target) {
        this.target = target;
    }

    public void setBill(boolean bill) {
        this.bill = bill; setBill = true;
    }
    public void setArc(boolean arc) {
        this.arc = arc; setArc = true;
    }
    public void setAllow(boolean allow) {
        this.allow = allow; setAllow = true;
    }
    public void setServiceId(int serviceid) {
        this.serviceid = serviceid; setId = true;
    }
    public void setPriority(int priority) {
        this.priority = priority; setPri = true;
    }

    public void process(CommandContext ctx)
    {
        String out = "Route '"+route+"'";
        try
        {
            RouteList list =  ctx.getRouteSubjectManager().getRoutes();

            Route oldRoute = list.get(route);
            if (oldRoute == null) {
                ctx.setMessage(out+" not found");
                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                return;
            }

            Route newRoute = new Route(route,
                    oldRoute.getPriority(), oldRoute.isEnabling(), oldRoute.isBilling(),
                    oldRoute.isArchiving(), oldRoute.isSuppressDeliveryReports(),
						  oldRoute.getServiceId(),
                    oldRoute.getSources(), oldRoute.getDestinations());

            if (target == TARGET_SRC)
            {
                for (int i=0; i<srcs.size(); i++) {
                    Object obj = srcs.get(i);
                    if (obj != null && obj instanceof RouteSrcDef) {
                        RouteSrcDef def = (RouteSrcDef)obj;
                        Source src = null;
                        if (def.getType() == RouteSrcDef.TYPE_MASK) {
                            src = new Source(new Mask(def.getSrc()));
                        } else if (def.getType() == RouteSrcDef.TYPE_SUBJECT) {
                            Subject subj = ctx.getRouteSubjectManager().getSubjects().get(def.getSrc());
                            if (subj == null) {
                                ctx.setMessage("Subject '"+def.getSrc()+"' in src definition not found. Couldn't alter "+out);
                                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                                return;
                            }
                            src = new Source(subj);
                        } else {
                            ctx.setMessage("Unsupported src definition for "+out);
                            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                            return;
                        }

                        if (action == ACTION_ADD) {
                            newRoute.addSource(src);
                        }
                        else if (action == ACTION_DEL)
                        {
                            Source oldSrc = newRoute.getSources().get(src.getName());
                            if (oldSrc != null) {
                                if (newRoute.getSources().size() > 1) {
                                    newRoute.removeDestination(src.getName());
                                } else {
                                    ctx.setMessage("Couldn't delete source '"+src.getName()+"' for "+out+". It rest only one.");
                                    ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                                    return;
                                }
                            } else {
                                ctx.setMessage("Source '"+src.getName()+"' not found for "+out);
                                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                                return;
                            }
                        }
                        else {
                            ctx.setMessage("Unsupported action on "+out+". Allowed ADD & DELETE");
                            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                            return;
                        }
                    }
                }
            }
            else if (target == TARGET_DST)
            {
                for (int i=0; i<dsts.size(); i++) {
                    Object obj = dsts.get(i);
                    if (obj != null && obj instanceof RouteDstDef) {
                        RouteDstDef def = (RouteDstDef)obj;
                        if (action == ACTION_ADD)
                        {
                            Destination dst = null;
                            SME sme = ctx.getSmeManager().getSmes().get(def.getSmeId());
                            if (sme == null) {
                                ctx.setMessage("SME '"+def.getSmeId()+"' in dst definition not found. Couldn't alter "+out);
                                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                                return;
                            }
                            if (def.getType() == RouteDstDef.TYPE_MASK) {
                                dst = new Destination(new Mask(def.getDst()), sme);
                            } else if (def.getType() == RouteDstDef.TYPE_SUBJECT) {
                                Subject subj = ctx.getRouteSubjectManager().getSubjects().get(def.getDst());
                                if (subj == null) {
                                    ctx.setMessage("Subject '"+def.getDst()+"' in dst definition not found. Couldn't alter "+out);
                                    ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                                    return;
                                }
                                dst = new Destination(subj, sme);
                            } else {
                                ctx.setMessage("Unsupported dst definition for "+out);
                                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                                return;
                            }
                            newRoute.addDestination(dst);
                        }
                        else if (action == ACTION_DEL)
                        {
                            String dstName;
                            if (def.getType() == RouteDstDef.TYPE_MASK) {
                                dstName = (new Mask(def.getDst())).getMask();
                            } else if (def.getType() == RouteDstDef.TYPE_SUBJECT) {
                                Subject subj = ctx.getRouteSubjectManager().getSubjects().get(def.getDst());
                                if (subj == null) {
                                    ctx.setMessage("Subject '"+def.getDst()+"' in dst definition not found. Couldn't alter "+out);
                                    ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                                    return;
                                }
                                dstName = subj.getName();
                            } else {
                                ctx.setMessage("Unsupported dst definition for "+out);
                                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                                return;
                            }

                            Destination oldDst = newRoute.getDestinations().get(dstName);
                            if (oldDst != null) {
                                if (newRoute.getDestinations().size() > 1) {
                                    newRoute.removeDestination(dstName);
                                } else {
                                    ctx.setMessage("Couldn't delete destination '"+dstName+"' for "+out+". It rest only one.");
                                    ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                                    return;
                                }
                            } else {
                                ctx.setMessage("Destination '"+dstName+"' not found for "+out);
                                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                                return;
                            }
                        }
                        else {
                            ctx.setMessage("Unsupported action on "+out+". Allowed ADD & DELETE");
                            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                            return;
                        }
                    }
                }
            }
            else
            {
                ctx.setMessage("Unsupported target on "+out+". Allowed SRC & DST");
                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                return;
            }

            if (setBill) newRoute.setBilling(bill);
            if (setArc) newRoute.setArchiving(arc);
            if (setAllow) newRoute.setEnabling(allow);
            if (setId) newRoute.setServiceId(serviceid);
            if (setPri) {
                if (priority < 0 || priority > 32000)
                    throw new Exception("Priority value should be between 0 and 32000");
                newRoute.setPriority(priority);
            }

            list.remove(oldRoute.getName());
            list.put(newRoute);
        }
        catch (Exception e) {
            ctx.setMessage("Couldn't add "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            return;
        }

        ctx.setMessage(out+" altered");
        ctx.setStatus(CommandContext.CMD_OK);
    }

    public String getId() {
        return "ROUTE_ALTER";
    }

}

