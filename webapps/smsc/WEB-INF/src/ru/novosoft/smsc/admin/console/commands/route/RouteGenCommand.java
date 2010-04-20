/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 7:06:42 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands.route;


import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.route.Route;

import java.util.ArrayList;

public abstract class RouteGenCommand extends CommandClass
 {
    public final static byte REPLAY_PATH_PASS = Route.REPLAY_PATH_PASS;
    public final static byte REPLAY_PATH_FORCE = Route.REPLAY_PATH_FORCE;
    public final static byte REPLAY_PATH_SUPPRESS = Route.REPLAY_PATH_SUPPRESS;

    public final static byte BILLING_TRUE = Route.BILLING_TRUE;
    public final static byte BILLING_FALSE = Route.BILLING_FALSE;
    public final static byte BILLING_MT = Route.BILLING_MT;
    public final static byte BILLING_FR = Route.BILLING_FR;

    protected String route = null;

    protected ArrayList srcs = new ArrayList();
    protected ArrayList dsts = new ArrayList();

    protected String srcSmeId = "";
    protected boolean isSrcSmeId = false;

    protected String deliveryMode = "default";
    protected boolean isDeliveryMode = false;

    protected String forwardTo = "";
    protected boolean isForwardTo = false;

    protected String notes = "";
    protected boolean isNotes = false;

    protected boolean hide = false;
    protected boolean isHide = false;

    protected byte replayPath = REPLAY_PATH_PASS;
    protected boolean isReplayPath = false;

    protected boolean forceDelivery = false;
    protected boolean isForceDelivery = false;

    protected long aclId = -1;
    protected boolean isAclId = false;

    protected String providerName = null;
    protected boolean isProviderName = false;

    protected String categoryName = null;
    protected boolean isCategoryName = false;

    protected boolean allowBlocked = false;
    protected boolean isAllowBlocked = false;

    protected String backupSme = null;
    protected boolean isBackupSme = false;

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
        this.forceDelivery = forceDelivery; isForceDelivery = true;
    }
    public void setAllowBlocked(boolean allowBlocked) {
        this.allowBlocked = allowBlocked; isAllowBlocked = true;
    }
    public void setAclId(long aclId) {
        this.aclId = aclId; isAclId = true;
    }
    public void setProviderName(String providerName) {
        this.providerName = providerName; isProviderName = true;
    }
    public void setCategoryName(String categoryName) {
        this.categoryName = categoryName; isCategoryName = true;
    }
    public void setBackupSme(String backupSme) {
        this.backupSme = backupSme; isBackupSme = true;
    }
}

