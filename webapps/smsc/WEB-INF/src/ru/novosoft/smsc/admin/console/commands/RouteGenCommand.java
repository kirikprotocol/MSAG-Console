/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 7:06:42 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.route.Route;

import java.util.ArrayList;

public abstract class RouteGenCommand implements Command
{
  public final static byte REPLAY_PATH_PASS     = Route.REPLAY_PATH_PASS;
  public final static byte REPLAY_PATH_FORCE    = Route.REPLAY_PATH_FORCE;
  public final static byte REPLAY_PATH_SUPPRESS = Route.REPLAY_PATH_SUPPRESS;

  protected String route = null;

  protected ArrayList srcs = new ArrayList();
  protected ArrayList dsts = new ArrayList();

  protected String srcSmeId    = "";
  protected boolean isSrcSmeId = false;

  protected String deliveryMode    = "default";
  protected boolean isDeliveryMode = false;

  protected String forwardTo    = "";
  protected boolean isForwardTo = false;

  protected String notes    = "";
  protected boolean isNotes = false;

  protected boolean hide   = false;
  protected boolean isHide = false;

  protected byte replayPath      = REPLAY_PATH_PASS;
  protected boolean isReplayPath = false;

  protected boolean forceDelivery   = false;
  protected boolean isForceDelivery = false;

  protected long aclId = -1;
  protected boolean isAclId = false;

  public void setRoute(String route) {
    this.route = route;
  }
  public void addSrcDef(RouteSrcDef def) {
    srcs.add(def);
  }
  public void addDstDef(RouteDstDef def) {
    dsts.add(def);
  }
  public void setSrcSmeId(String srcSmeId) {
    this.srcSmeId = srcSmeId; isSrcSmeId = true;
  }
  public void setDeliveryMode(String deliveryMode) {
    this.deliveryMode = deliveryMode; isDeliveryMode = true;
  }
  public void setForwardTo(String forwardTo) {
    this.forwardTo = forwardTo; isForwardTo = true;
  }
  public void setNotes(String notes) {
    this.notes = notes; isNotes = true;
  }
  public void setHide(boolean hide) {
    this.hide = hide; isHide = true;
  }
  public void setReplayPath(byte replayPath) {
    this.replayPath = replayPath; isReplayPath = true;
  }
  public void setForceDelivery(boolean forceDelivery) {
    this.forceDelivery = forceDelivery;
    isForceDelivery = true;
  }
  public void setAclId(long aclId) {
    this.aclId = aclId; isAclId = true;
  }
}

