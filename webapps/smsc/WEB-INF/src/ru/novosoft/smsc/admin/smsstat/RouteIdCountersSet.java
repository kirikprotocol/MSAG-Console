/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 10, 2002
 * Time: 4:38:25 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.smsstat;

public class RouteIdCountersSet {
    public String routeid;
    public int counter;

    RouteIdCountersSet(String id, int count) {
        routeid = id; this.counter = count;
    }
}
