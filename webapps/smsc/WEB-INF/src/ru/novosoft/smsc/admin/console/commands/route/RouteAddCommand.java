/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 7:06:42 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands.route;


import ru.novosoft.smsc.admin.category.Category;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.admin.route.*;


public class RouteAddCommand extends RouteGenCommand
{
    private byte bill    = Route.BILLING_TRUE;
    private boolean transit = false;
    private boolean arc     = true;
    private boolean allow   = true;
    private boolean receipt = true;
    private boolean active  = true;

    private int serviceid;
    private int priority;

    public void process(CommandContext ctx)
    {
      String out = "Route '" + route + "'";
      try {
          RouteList list = ctx.getRouteSubjectManager().getRoutes();
          Route smscRoute = list.get(route);
          if (smscRoute != null) {
            ctx.setMessage(out + " already exists");
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            return;
          }

          if (isSrcSmeId) {
            SME sme = ctx.getSmeManager().get(srcSmeId);
            if (sme == null) throw new Exception("SME '" + srcSmeId + "' not found (srcSmeId)");
          }

          if (isBackupSme) {
            SME sme = ctx.getSmeManager().get(backupSme);
            if (sme == null) throw new Exception("SME '" + backupSme + "' not found (backupSme)");
          }

          long providerId = -1;
          if (isProviderName) {
              Provider provider = ctx.getProviderManager().getProviderByName(providerName);
              if (provider == null) throw new Exception("Provider '" + providerName + "' not found");
              providerId = provider.getId();
          }

          long categoryId = -1;
          if (isCategoryName) {
              Category category = ctx.getCategoryManager().getCategoryByName(categoryName);
              if (category == null) throw new Exception("Category '" + categoryName + "' not found");
              categoryId = category.getId();
          }

          if (isForwardTo && (!isSrcSmeId || !srcSmeId.equalsIgnoreCase("MAP_PROXY")))
            throw new Exception("Option 'fwd' is valid only for srcSmeId='MAP_PROXY'");

          SourceList srcList = new SourceList();
          for (int i = 0; i < srcs.size(); i++) {
            Object obj = srcs.get(i);
            if (obj != null && obj instanceof RouteSrcDef) {
              RouteSrcDef def = (RouteSrcDef) obj;
              if (def.getType() == RouteSrcDef.TYPE_MASK) {
                srcList.add(new Source(new Mask(def.getSrc())));
              } else if (def.getType() == RouteSrcDef.TYPE_SUBJECT) {
                Subject subj = ctx.getRouteSubjectManager().getSubjects().get(def.getSrc());
                if (subj == null)
                    throw new Exception("Subject '" + def.getSrc() + "' in src definition not found");
                srcList.add(new Source(subj));
              }
              else throw new Exception("Unsupported src definition for " + out);
            }
          }

          DestinationList dstList = new DestinationList();
          for (int i = 0; i < dsts.size(); i++) {
            Object obj = dsts.get(i);
            if (obj != null && obj instanceof RouteDstDef) {
              RouteDstDef def = (RouteDstDef) obj;
              SME sme = ctx.getSmeManager().get(def.getSmeId());
              if (sme == null)
                  throw new Exception("SME '" + def.getSmeId() + "' in dst definition not found");
              if (def.getType() == RouteDstDef.TYPE_MASK) {
                dstList.add(new Destination(new Mask(def.getDst()), sme));
              } else if (def.getType() == RouteDstDef.TYPE_SUBJECT) {
                Subject subj = ctx.getRouteSubjectManager().getSubjects().get(def.getDst());
                if (subj == null)
                    throw new Exception("Subject '" + def.getDst() + "' in dst definition not found");
                dstList.add(new Destination(subj, sme));
              }
              else throw new Exception("Unsupported dst definition for " + out);
            }
          }

          smscRoute = new Route(route, priority, allow ? Route.TRAFFIC_MODE_ALL : Route.TRAFFIC_MODE_PROHIBITED, bill, transit, arc, !receipt,
                                active, serviceid, srcList, dstList, srcSmeId, deliveryMode, forwardTo,
                                hide, (isReplayPath) ? replayPath : REPLAY_PATH_PASS,
                                notes, forceDelivery, ((isAclId) ? aclId : -1),
                                (isAllowBlocked) ? allowBlocked : false, providerId, categoryId);

          if (isBackupSme)
            smscRoute.setBackupSmeId(backupSme);

          if (priority < 0 || priority > 32000)
            throw new Exception("Priority value should be between 0 and 32000");

          list.put(smscRoute);
      } catch (Exception e) {
          ctx.setMessage("Couldn't add " + out + ". Cause: " + e.getMessage());
          ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
          return;
      }

      ctx.setMessage(out + " added");
      ctx.setStatus(CommandContext.CMD_OK);
    }

    public void setBill(byte bill) {
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
    public void setReceipt(boolean receipt) {
      this.receipt = receipt;
    }
    public void setActive(boolean active) {
      this.active = active;
    }
    public void setTransit(boolean transit) {
      this.transit = transit;
    }

    public String getId() {
      return "ROUTE_ADD";
    }
	public void updateJournalAndStatuses(CommandContext ctx, String userName)
	{
		journalAppend(ctx, userName, SubjectTypes.TYPE_route, route, Actions.ACTION_ADD);
		ctx.getStatuses().setRoutesChanged(true);
	}
}

