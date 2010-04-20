/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 7:10:13 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands.route;


import ru.novosoft.smsc.admin.category.Category;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.provider.Provider;
import ru.novosoft.smsc.admin.route.*;


public class RouteAlterCommand extends RouteGenCommand
{
  public final static byte ACTION_ADD = 10;
  public final static byte ACTION_DEL = 20;

  public final static byte TARGET_SRC = 30;
  public final static byte TARGET_DST = 40;

  private byte action = ACTION_ADD;
  private byte target = TARGET_SRC;

  private byte bill    = Route.BILLING_TRUE;
  private boolean transit = true;
  private boolean arc     = true;
  private boolean allow   = true;
  private boolean receipt = true;
  private boolean active  = true;
  private int serviceid;
  private int priority;

  private boolean setBill    = false;
  private boolean setTransit = false;
  private boolean setArc     = false;
  private boolean setAllow   = false;
  private boolean setId      = false;
  private boolean setPri     = false;
  private boolean setReceipt = false;
  private boolean setActive  = false;

  public void process(CommandContext ctx)
  {
    String out = "Route '" + route + "'";
    try
    {
      RouteList list = ctx.getRouteSubjectManager().getRoutes();

      Route oldRoute = list.get(route);
      if (oldRoute == null) {
        ctx.setMessage(out + " not found");
        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        return;
      }

      if (isSrcSmeId && srcSmeId.length() > 0) {
        SME sme = ctx.getSmeManager().get(srcSmeId);
        if (sme == null) throw new Exception("SME '" + srcSmeId + "' not found (srcSmeId)");
      }

      if (isBackupSme && backupSme.length() > 0) {
        SME sme = ctx.getSmeManager().get(backupSme);
        if (sme == null) throw new Exception("SME '" + backupSme + "' not found (backupSme)");
      }

      if (isForwardTo && (!isSrcSmeId || !srcSmeId.equalsIgnoreCase("MAP_PROXY")))
        throw new Exception("Option 'fwd' is valid only for srcSmeId='MAP_PROXY'");

      long providerId = oldRoute.getProviderId();
      if (isProviderName) {
          Provider provider = ctx.getProviderManager().getProviderByName(providerName);
          if (provider == null) throw new Exception("Provider '" + providerName + "' not found");
          providerId = provider.getId();
      }

      long categoryId = oldRoute.getCategoryId();
      if (isCategoryName) {
          Category category = ctx.getCategoryManager().getCategoryByName(categoryName);
          if (category == null) throw new Exception("Category '" + categoryName + "' not found");
          categoryId = category.getId();
      }

      Route newRoute = new Route(route,
                                 oldRoute.getPriority(), oldRoute.isEnabling(), oldRoute.getBilling(),
                                 oldRoute.isTransit(), oldRoute.isArchiving(),
                                 oldRoute.isSuppressDeliveryReports(),
                                 oldRoute.isActive(), oldRoute.getServiceId(),
                                 oldRoute.getSources(), oldRoute.getDestinations(),
                                 oldRoute.getSrcSmeId(), oldRoute.getDeliveryMode(), oldRoute.getForwardTo(),
                                 oldRoute.isHide(), oldRoute.getReplayPath(), oldRoute.getNotes(),
                                 oldRoute.isForceDelivery(), oldRoute.getAclId(),
                                 oldRoute.isAllowBlocked(), providerId, categoryId);

      if (target == TARGET_SRC)
      {
        for (int i = 0; i < srcs.size(); i++) {
          Object obj = srcs.get(i);
          if (obj != null && obj instanceof RouteSrcDef) {
            RouteSrcDef def = (RouteSrcDef) obj;
            Source src = null;
            if (def.getType() == RouteSrcDef.TYPE_MASK) {
              src = new Source(new Mask(def.getSrc()));
            } else if (def.getType() == RouteSrcDef.TYPE_SUBJECT) {
              Subject subj = ctx.getRouteSubjectManager().getSubjects().get(def.getSrc());
              if (subj == null)
                  throw new Exception("Subject '" + def.getSrc() + "' in src definition not found");
              src = new Source(subj);
            }
            else throw new Exception("Unsupported src definition for " + out);

            if (action == ACTION_ADD) {
              newRoute.addSource(src);
            } else if (action == ACTION_DEL) {
              Source oldSrc = newRoute.getSources().get(src.getName());
              if (oldSrc != null) {
                if (newRoute.getSources().size() > 1) newRoute.removeSource(src.getName());
                else throw new Exception("Couldn't delete source '" + src.getName() + "' for " + out + ". It rest only one");
              }
              else throw new Exception("Source '" + src.getName() + "' not found for " + out);
            }
            else throw new Exception("Unsupported action on " + out + ". Allowed ADD & DELETE");
          }
        }
      }
      else if (target == TARGET_DST)
      {
        for (int i = 0; i < dsts.size(); i++) {
          Object obj = dsts.get(i);
          if (obj != null && obj instanceof RouteDstDef) {
            RouteDstDef def = (RouteDstDef) obj;
            if (action == ACTION_ADD) {
              Destination dst = null;
              SME sme = ctx.getSmeManager().get(def.getSmeId());
              if (sme == null)
                  throw new Exception("SME '" + def.getSmeId() + "' in dst definition not found");
              if (def.getType() == RouteDstDef.TYPE_MASK) {
                dst = new Destination(new Mask(def.getDst()), sme);
              } else if (def.getType() == RouteDstDef.TYPE_SUBJECT) {
                Subject subj = ctx.getRouteSubjectManager().getSubjects().get(def.getDst());
                if (subj == null)
                    throw new Exception("Subject '" + def.getDst() + "' in dst definition not found");
                dst = new Destination(subj, sme);
              }
              else throw new Exception("Unsupported dst definition for " + out);
              newRoute.addDestination(dst);
            }
            else if (action == ACTION_DEL) {
              String dstName;
              if (def.getType() == RouteDstDef.TYPE_MASK) {
                dstName = (new Mask(def.getDst())).getMask();
              } else if (def.getType() == RouteDstDef.TYPE_SUBJECT) {
                Subject subj = ctx.getRouteSubjectManager().getSubjects().get(def.getDst());
                if (subj == null)
                    throw new Exception("Subject '" + def.getDst() + "' in dst definition not found");
                dstName = subj.getName();
              }
              else throw new Exception("Unsupported dst definition for " + out);

              Destination oldDst = newRoute.getDestinations().get(dstName);
              if (oldDst != null) {
                if (newRoute.getDestinations().size() > 1) newRoute.removeDestination(dstName);
                else throw new Exception("Couldn't delete destination '" + dstName + "' for " + out + ". It rest only one.");
              }
              else throw new Exception("Destination '" + dstName + "' not found for " + out);
            }
            else throw new Exception("Unsupported action on " + out + ". Allowed ADD & DELETE");
          }
        }
      }
      else throw new Exception("Unsupported target on " + out + ". Allowed SRC & DST");

      if (setBill) newRoute.setBilling(bill);
      if (setTransit) newRoute.setTransit(transit);
      if (setArc) newRoute.setArchiving(arc);
      if (setAllow) newRoute.setEnabling(allow);
      if (setReceipt) newRoute.setSuppressDeliveryReports(!receipt);
      if (setActive) newRoute.setActive(active);
      if (setId) newRoute.setServiceId(serviceid);
      if (setPri) {
        if (priority < 0 || priority > 32000)
          throw new Exception("Priority value should be between 0 and 32000");
        newRoute.setPriority(priority);
      }
      if (isDeliveryMode) newRoute.setDeliveryMode(deliveryMode);
      if (isForwardTo) newRoute.setForwardTo(forwardTo);
      if (isSrcSmeId) newRoute.setSrcSmeId(srcSmeId);
      if (isHide) newRoute.setHide(hide);
      if (isNotes) newRoute.setNotes(notes);
      if (isReplayPath) newRoute.setReplayPath(replayPath);
      if (isForceDelivery) newRoute.setForceDelivery(forceDelivery);
      if (isAclId) newRoute.setAclId(aclId);
      if (isAllowBlocked) newRoute.setAllowBlocked(allowBlocked);
      if (isBackupSme) newRoute.setBackupSmeId(backupSme);

      list.remove(oldRoute.getName());
      list.put(newRoute);
    }
    catch (Exception e) {
      ctx.setMessage("Couldn't add " + out + ". Cause: " + e.getMessage());
      ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
      return;
    }

    ctx.setMessage(out + " altered");
    ctx.setStatus(CommandContext.CMD_OK);
  }

  public void setAction(byte action) {
    this.action = action;
  }
  public void setTarget(byte target) {
    this.target = target;
  }
  public void setBill(byte bill) {
    this.bill = bill; setBill = true;
  }
  public void setTransit(boolean transit) {
    this.transit = transit; setTransit = true;
  }

  public void setArc(boolean arc) {
    this.arc = arc; setArc = true;
  }
  public void setAllow(boolean allow) {
    this.allow = allow; setAllow = true;
  }
  public void setReceipt(boolean receipt) {
    this.receipt = receipt; setReceipt = true;
  }
  public void setActive(boolean active) {
    this.active = active; setActive = true;
  }
  public void setServiceId(int serviceid) {
    this.serviceid = serviceid;setId = true;
  }
  public void setPriority(int priority) {
    this.priority = priority; setPri = true;
  }

  public String getId()
  {
    return "ROUTE_ALTER";
  }

	public void updateJournalAndStatuses(CommandContext ctx, String userName)
	{
		journalAppend(ctx, userName, SubjectTypes.TYPE_route, route, Actions.ACTION_MODIFY);
		ctx.getStatuses().setRoutesChanged(true);
	}
}

