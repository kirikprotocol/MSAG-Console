/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Feb 5, 2003
 * Time: 4:23:25 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.mscman;

import java.util.StringTokenizer;

public class MscInfo
{
    private final static String MANUAL_LOCKED = "Locked manually";
    private final static String AUT0_LOCKED = "Locked automatically";
    private final static String NOT_LOCKED = "Available";

    private final static String FIELDS_SEPARATOR = ",";

    private String   mscNum = "";
    private boolean  mLock  = false;
    private boolean  aLock  = false;
    private int      fCount = 0;

    public MscInfo() {}
    public MscInfo(String str)
    {
        try {
            StringTokenizer st = new StringTokenizer(str, FIELDS_SEPARATOR);
            mscNum = st.nextToken();
            if (mscNum == null || mscNum.length() <=0) mscNum = "<<undefined>>";
            String mLockStr = st.nextToken();
            mLock = (mLockStr == null) ? false:(mLockStr.equalsIgnoreCase("true"));
            String aLockStr = st.nextToken();
            aLock = (aLockStr == null) ? false:(aLockStr.equalsIgnoreCase("true"));
            String fcStr = st.nextToken();
            fCount = Integer.parseInt(fcStr);
        } catch (Exception exc) {
            exc.printStackTrace();
            // todo: report error here
        }
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
        return mLock;
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
