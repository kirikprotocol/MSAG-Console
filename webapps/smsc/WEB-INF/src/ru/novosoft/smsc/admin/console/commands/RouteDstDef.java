/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 24, 2002
 * Time: 2:30:44 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

public class RouteDstDef
{
    public final static byte TYPE_MASK = 10;
    public final static byte TYPE_SUBJECT = 20;

    private byte type;
    private String dst;
    private String smeId;

    public RouteDstDef() {
        type = TYPE_MASK; dst = null; smeId = null;
    }
    public RouteDstDef(byte type, String dst, String smeId) {
        this.type = type; this.dst = dst; this.smeId = smeId;
    }

    public byte getType() {
        return type;
    }
    public String getDst() {
        return dst;
    }
    public String getSmeId() {
        return smeId;
    }
    public void setType(byte type) {
        this.type = type;
    }
    public void setDst(String dst) {
        this.dst = dst;
    }
    public void setSmeId(String smeId) {
        this.smeId = smeId;
    }
}
