// Decompiled by Jad v1.5.8e2. Copyright 2001 Pavel Kouznetsov.
// Jad home page: http://kpdus.tripod.com/jad.html
// Decompiler options: packfields(3) packimports(7) deadcode fieldsfirst splitstr(64) nonlb lnc radix(10) lradix(10) 

package org.apache.xml.resolver.readers;

import java.io.IOException;
import java.io.InputStream;
import java.net.MalformedURLException;
import java.util.Vector;
import org.apache.xml.resolver.Catalog;
import org.apache.xml.resolver.CatalogEntry;
import org.apache.xml.resolver.CatalogException;
import org.apache.xml.resolver.helpers.Debug;

// Referenced classes of package org.apache.xml.resolver.readers:
//            TextCatalogReader

public class TR9401CatalogReader extends TextCatalogReader {

    public TR9401CatalogReader() {
    }

    public void readCatalog(Catalog catalog, InputStream inputstream) throws MalformedURLException, IOException {
        super.catfile = inputstream;
        if(super.catfile == null)
            return;
        Vector vector = null;
        do {
            String s = nextToken();
            if(s == null) {
                if(vector != null) {
                    catalog.unknownEntry(vector);
                    vector = null;
                }
                super.catfile.close();
                super.catfile = null;
                return;
            }
            String s1 = null;
            if(super.caseSensitive)
                s1 = s;
            else
                s1 = s.toUpperCase();
            if(s1.equals("DELEGATE"))
                s1 = "DELEGATE_PUBLIC";
            try {
                int i = CatalogEntry.getEntryType(s1);
                int j = CatalogEntry.getEntryArgCount(i);
                Vector vector1 = new Vector();
                if(vector != null) {
                    catalog.unknownEntry(vector);
                    vector = null;
                }
                for(int k = 0; k < j; k++)
                    vector1.addElement(nextToken());

                catalog.addEntry(new CatalogEntry(s1, vector1));
            }
            catch(CatalogException catalogexception) {
                if(catalogexception.getExceptionType() == 3) {
                    if(vector == null)
                        vector = new Vector();
                    vector.addElement(s);
                } else
                if(catalogexception.getExceptionType() == 2) {
                    Debug.message(1, "Invalid catalog entry", s);
                    vector = null;
                }
            }
        } while(true);
    }
}
