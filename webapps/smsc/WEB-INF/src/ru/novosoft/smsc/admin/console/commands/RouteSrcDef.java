/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 24, 2002
 * Time: 2:30:44 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

public class RouteSrcDef
{
    public final static byte TYPE_MASK = 10;
    public final static byte TYPE_SUBJECT = 20;

    private byte type;
    private String src;

    public RouteSrcDef() {
        type = TYPE_MASK; src = null;
    }

    public RouteSrcDef(byte type, String src) {
        this.type = type; this.src = src;
    }

    public byte getType() {
        return type;
    }
    public String getSrc() {
        return src;
    }
    public void setType(byte type) {
        this.type = type;
    }
    public void setSrc(String src) {
        this.src = src;
    }
}
