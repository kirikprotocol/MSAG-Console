/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 25, 2003
 * Time: 12:02:59 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.dl;

public class Principal
{
    private String  address;
    private int     maxLists;
    private int     maxElements;

    public Principal(String address, int maxLists, int maxElements) {
        this.address = address;
        this.maxLists = maxLists;
        this.maxElements = maxElements;
    }

    public String getAddress() {
        return address;
    }
    public void setAddress(String address) {
        this.address = address;
    }

    public int getMaxLists() {
        return maxLists;
    }
    public void setMaxLists(int maxLists) {
        this.maxLists = maxLists;
    }

    public int getMaxElements() {
        return maxElements;
    }
    public void setMaxElements(int maxElements) {
        this.maxElements = maxElements;
    }
}
