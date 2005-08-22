// Decompiled by Jad v1.5.8e2. Copyright 2001 Pavel Kouznetsov.
// Jad home page: http://kpdus.tripod.com/jad.html
// Decompiler options: packfields(3) packimports(7) deadcode fieldsfirst splitstr(64) nonlb lnc radix(10) lradix(10) 

package org.apache.xml.resolver.readers;

import java.io.IOException;
import java.io.InputStream;
import java.net.MalformedURLException;
import org.apache.xml.resolver.Catalog;
import org.apache.xml.resolver.CatalogException;

public interface CatalogReader {

    public abstract void readCatalog(Catalog catalog, String s) throws MalformedURLException, IOException, CatalogException;

    public abstract void readCatalog(Catalog catalog, InputStream inputstream) throws IOException, CatalogException;
}
