// Decompiled by Jad v1.5.8e2. Copyright 2001 Pavel Kouznetsov.
// Jad home page: http://kpdus.tripod.com/jad.html
// Decompiler options: packfields(3) packimports(7) deadcode fieldsfirst splitstr(64) nonlb lnc radix(10) lradix(10) 

package org.apache.xml.resolver;

import java.util.Hashtable;
import java.util.Vector;

// Referenced classes of package org.apache.xml.resolver:
//            CatalogException

public class CatalogEntry {

    protected static int nextEntry = 0;
    protected static Hashtable entryTypes = new Hashtable();
    protected static Vector entryArgs = new Vector();
    protected int entryType;
    protected Vector args;

    public static int addEntryType(String s, int i) {
        entryTypes.put(s, new Integer(nextEntry));
        entryArgs.add(nextEntry, new Integer(i));
        nextEntry++;
        return nextEntry - 1;
    }

    public static int getEntryType(String s) throws CatalogException {
        if(!entryTypes.containsKey(s))
            throw new CatalogException(3);
        Integer integer = (Integer)entryTypes.get(s);
        if(integer == null)
            throw new CatalogException(3);
        else
            return integer.intValue();
    }

    public static int getEntryArgCount(String s) throws CatalogException {
        return getEntryArgCount(getEntryType(s));
    }

    public static int getEntryArgCount(int i) throws CatalogException {
        try {
            Integer integer = (Integer)entryArgs.get(i);
            return integer.intValue();
        }
        catch(ArrayIndexOutOfBoundsException arrayindexoutofboundsexception) {
            throw new CatalogException(3);
        }
    }

    public CatalogEntry() {
        entryType = 0;
        args = null;
    }

    public CatalogEntry(String s, Vector vector) throws CatalogException {
        entryType = 0;
        args = null;
        Integer integer = (Integer)entryTypes.get(s);
        if(integer == null)
            throw new CatalogException(3);
        int i = integer.intValue();
        try {
            Integer integer1 = (Integer)entryArgs.get(i);
            if(integer1.intValue() != vector.size())
                throw new CatalogException(2);
        }
        catch(ArrayIndexOutOfBoundsException arrayindexoutofboundsexception) {
            throw new CatalogException(3);
        }
        entryType = i;
        args = vector;
    }

    public CatalogEntry(int i, Vector vector) throws CatalogException {
        entryType = 0;
        args = null;
        try {
            Integer integer = (Integer)entryArgs.get(i);
            if(integer.intValue() != vector.size())
                throw new CatalogException(2);
        }
        catch(ArrayIndexOutOfBoundsException arrayindexoutofboundsexception) {
            throw new CatalogException(3);
        }
        entryType = i;
        args = vector;
    }

    public int getEntryType() {
        return entryType;
    }

    public String getEntryArg(int i) {
        try {
            String s = (String)args.get(i);
            return s;
        }
        catch(ArrayIndexOutOfBoundsException arrayindexoutofboundsexception) {
            return null;
        }
    }

    public void setEntryArg(int i, String s) throws ArrayIndexOutOfBoundsException {
        args.set(i, s);
    }

}
