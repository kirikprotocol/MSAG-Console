package mobi.eyeline.smsquiz.manager;

/**
 * Created by IntelliJ IDEA.
 * User: alkhal
 * Date: 30.09.2008
 * Time: 17:05:38
 * To change this template use File | Settings | File Templates.
 */
public interface SubscriptionManager {
    public boolean subscribe(String address);
    public boolean unsubscribe(String address);
    public boolean subscribed(String address);
}
