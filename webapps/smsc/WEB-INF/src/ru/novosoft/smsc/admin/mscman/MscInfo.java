/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 5, 2003
 * Time: 4:23:25 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.mscman;

public class MscInfo
{
    private final static String MANUAL_LOCKED = "Locked manually";
    private final static String AUT0_LOCKED = "Locked automatically";
    private final static String NOT_LOCKED = "Available";

    private String   mscNum = "";
    private boolean  mLock  = false;
    private boolean  aLock  = false;
    private int      fCount = 0;

    public MscInfo() {}
    public MscInfo(String str) {
        // todo: implement deserialization
    }
    public MscInfo(String mscNum, boolean mLock, boolean aLock, int fCount) {
        this.mscNum = mscNum; this.mLock = mLock;
        this.aLock = aLock; this.fCount = fCount;
    }

    public String getMscNum() {
        return mscNum;
    }
    public void setMscNum(String mscNum) {
        this.mscNum = mscNum;
    }
    public String getLockString() {
        if (mLock) return MANUAL_LOCKED;
        if (aLock) return AUT0_LOCKED;
        return NOT_LOCKED;
    }
    public boolean ismLock() {
        return aLock;
    }
    public void setmLock(boolean mLock) {
        this.mLock = mLock;
    }
    public boolean isaLock() {
        return aLock;
    }
    public void setaLock(boolean aLock) {
        this.aLock = aLock;
    }
    public int getfCount() {
        return fCount;
    }
    public void setfCount(int fCount) {
        this.fCount = fCount;
    }
}
