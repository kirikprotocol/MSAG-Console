/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 10, 2002
 * Time: 4:38:40 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.smsstat;

public class SmeIdCountersSet {
    public String smeid;
    public int received;
    public int sent;

    SmeIdCountersSet(String id, int received, int sent) {
        smeid = id; this.received = received; this.sent = sent;
    }
}
