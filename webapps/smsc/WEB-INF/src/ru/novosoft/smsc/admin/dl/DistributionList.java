/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 20, 2003
 * Time: 2:23:08 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.dl;

public class DistributionList
{
    private String  name;
    private String  owner;
    private int     maxElements;
    private boolean sys;

    public DistributionList() {
        name = ""; sys = false; maxElements = 0;
    }

    public DistributionList(String name, String owner, int maxElements, boolean sys) {
        this.name = name; this.owner = owner; this.sys = sys;
        this.maxElements = maxElements;
    }

    public String getName() { return name; }
    public void setName(String name) { this.name = name; }

    public String getOwner() { return owner; }
    public void setOwner(String owner) { this.owner = owner; }

    public boolean isSys() { return sys; }
    public void setSys(boolean sys) { this.sys = sys; }

    public int getMaxElements() { return maxElements; }
    public void setMaxElements(int maxElements) { this.maxElements = maxElements; }
}
