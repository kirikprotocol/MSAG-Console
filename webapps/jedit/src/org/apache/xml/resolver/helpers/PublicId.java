// Decompiled by Jad v1.5.8e2. Copyright 2001 Pavel Kouznetsov.
// Jad home page: http://kpdus.tripod.com/jad.html
// Decompiler options: packfields(3) packimports(7) deadcode fieldsfirst splitstr(64) nonlb lnc radix(10) lradix(10) 

package org.apache.xml.resolver.helpers;


public abstract class PublicId {

    protected PublicId() {
    }

    public static String normalize(String s) {
        String s1 = s.replace('\t', ' ');
        s1 = s1.replace('\r', ' ');
        s1 = s1.replace('\n', ' ');
        int i;
        for(s1 = s1.trim(); (i = s1.indexOf("  ")) >= 0; s1 = s1.substring(0, i) + s1.substring(i + 1));
        return s1;
    }

    public static String encodeURN(String s) {
        String s1 = normalize(s);
        s1 = stringReplace(s1, "%", "%25");
        s1 = stringReplace(s1, ";", "%3B");
        s1 = stringReplace(s1, "'", "%27");
        s1 = stringReplace(s1, "?", "%3F");
        s1 = stringReplace(s1, "#", "%23");
        s1 = stringReplace(s1, "+", "%2B");
        s1 = stringReplace(s1, " ", "+");
        s1 = stringReplace(s1, "::", ";");
        s1 = stringReplace(s1, ":", "%3A");
        s1 = stringReplace(s1, "//", ":");
        s1 = stringReplace(s1, "/", "%2F");
        return "urn:publicid:" + s1;
    }

    public static String decodeURN(String s) {
        String s1 = "";
        if(s.startsWith("urn:publicid:"))
            s1 = s.substring(13);
        else
            return s;
        s1 = stringReplace(s1, "%2F", "/");
        s1 = stringReplace(s1, ":", "//");
        s1 = stringReplace(s1, "%3A", ":");
        s1 = stringReplace(s1, ";", "::");
        s1 = stringReplace(s1, "+", " ");
        s1 = stringReplace(s1, "%2B", "+");
        s1 = stringReplace(s1, "%23", "#");
        s1 = stringReplace(s1, "%3F", "?");
        s1 = stringReplace(s1, "%27", "'");
        s1 = stringReplace(s1, "%3B", ";");
        s1 = stringReplace(s1, "%25", "%");
        return s1;
    }

    private static String stringReplace(String s, String s1, String s2) {
        String s3 = "";
        for(int i = s.indexOf(s1); i >= 0; i = s.indexOf(s1)) {
            s3 = s3 + s.substring(0, i);
            s3 = s3 + s2;
            s = s.substring(i + 1);
        }

        return s3 + s;
    }
}
