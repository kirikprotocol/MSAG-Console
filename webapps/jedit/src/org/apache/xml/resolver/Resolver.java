// Decompiled by DJ v3.7.7.81 Copyright 2004 Atanas Neshkov  Date: 05.08.2005 13:32:57
// Home Page : http://members.fortunecity.com/neshkov/dj.html  - Check often for new version!
// Decompiler options: packimports(3) 

package org.apache.xml.resolver;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.*;
import java.util.Enumeration;
import java.util.Vector;
import javax.xml.parsers.SAXParserFactory;
import org.apache.xml.resolver.helpers.Debug;
import org.apache.xml.resolver.readers.SAXCatalogReader;
import org.apache.xml.resolver.readers.TR9401CatalogReader;

// Referenced classes of package org.apache.xml.resolver:
//            Catalog, CatalogEntry, CatalogException

public class Resolver extends Catalog
{

    public Resolver()
    {
    }

    public void setupReaders()
    {
        SAXParserFactory saxparserfactory = SAXParserFactory.newInstance();
        saxparserfactory.setNamespaceAware(true);
        saxparserfactory.setValidating(false);
        SAXCatalogReader saxcatalogreader = new SAXCatalogReader(saxparserfactory);
        saxcatalogreader.setCatalogParser(null, "XMLCatalog", "org.apache.xml.resolver.readers.XCatalogReader");
        saxcatalogreader.setCatalogParser("urn:oasis:names:tc:entity:xmlns:xml:catalog", "catalog", "org.apache.xml.resolver.readers.ExtendedXMLCatalogReader");
        addReader("application/xml", saxcatalogreader);
        TR9401CatalogReader tr9401catalogreader = new TR9401CatalogReader();
        addReader("text/plain", tr9401catalogreader);
    }

    public void addEntry(CatalogEntry catalogentry)
    {
        int i = catalogentry.getEntryType();
        if(i == URISUFFIX)
        {
            String s = normalizeURI(catalogentry.getEntryArg(0));
            String s2 = makeAbsolute(normalizeURI(catalogentry.getEntryArg(1)));
            catalogentry.setEntryArg(1, s2);
            Debug.message(4, "URISUFFIX", s, s2);
        } else
        if(i == SYSTEMSUFFIX)
        {
            String s1 = normalizeURI(catalogentry.getEntryArg(0));
            String s3 = makeAbsolute(normalizeURI(catalogentry.getEntryArg(1)));
            catalogentry.setEntryArg(1, s3);
            Debug.message(4, "SYSTEMSUFFIX", s1, s3);
        }
        super.addEntry(catalogentry);
    }

    public String resolveURI(String s)
        throws MalformedURLException, IOException
    {
        System.out.println("Resolver.resolveURI 63 uri="+s);
        String s1 = super.resolveURI(s);
        if(s1 != null)
            return s1;
        for(Enumeration enumeration = super.catalogEntries.elements(); enumeration.hasMoreElements();)
        {
            CatalogEntry catalogentry = (CatalogEntry)enumeration.nextElement();
            if(catalogentry.getEntryType() == RESOLVER)
            {
                String s2 = resolveExternalSystem(s, catalogentry.getEntryArg(0));
                if(s2 != null)
                    return s2;
            } else
            if(catalogentry.getEntryType() == URISUFFIX)
            {
                String s3 = catalogentry.getEntryArg(0);
                String s4 = catalogentry.getEntryArg(1);
                if(s3.length() <= s.length() && s.substring(s.length() - s3.length()).equals(s3))
                    return s4;
            }
        }

        return resolveSubordinateCatalogs(Catalog.URI, null, null, s);
    }

    public String resolveSystem(String s)
        throws MalformedURLException, IOException
    {
        String s1 = super.resolveSystem(s);
        if(s1 != null)
            return s1;
        for(Enumeration enumeration = super.catalogEntries.elements(); enumeration.hasMoreElements();)
        {
            CatalogEntry catalogentry = (CatalogEntry)enumeration.nextElement();
            if(catalogentry.getEntryType() == RESOLVER)
            {
                String s2 = resolveExternalSystem(s, catalogentry.getEntryArg(0));
                if(s2 != null)
                    return s2;
            } else
            if(catalogentry.getEntryType() == SYSTEMSUFFIX)
            {
                String s3 = catalogentry.getEntryArg(0);
                String s4 = catalogentry.getEntryArg(1);
                if(s3.length() <= s.length() && s.substring(s.length() - s3.length()).equals(s3))
                    return s4;
            }
        }

        return resolveSubordinateCatalogs(Catalog.SYSTEM, null, null, s);
    }

    public String resolvePublic(String s, String s1)
        throws MalformedURLException, IOException
    {
        String s2 = super.resolvePublic(s, s1);
        if(s2 != null)
            return s2;
        for(Enumeration enumeration = super.catalogEntries.elements(); enumeration.hasMoreElements();)
        {
            CatalogEntry catalogentry = (CatalogEntry)enumeration.nextElement();
            if(catalogentry.getEntryType() == RESOLVER)
            {
                if(s1 != null)
                {
                    String s3 = resolveExternalSystem(s1, catalogentry.getEntryArg(0));
                    if(s3 != null)
                        return s3;
                }
                String s4 = resolveExternalPublic(s, catalogentry.getEntryArg(0));
                if(s4 != null)
                    return s4;
            }
        }

        return resolveSubordinateCatalogs(Catalog.PUBLIC, null, s, s1);
    }

    protected String resolveExternalSystem(String s, String s1)
        throws MalformedURLException, IOException
    {
        System.out.println("Resolver.resolveExternalSystem 144 s="+s);
        Resolver resolver = queryResolver(s1, "i2l", s, null);
        if(resolver != null)
            return resolver.resolveSystem(s);
        else
            return null;
    }

    protected String resolveExternalPublic(String s, String s1)
        throws MalformedURLException, IOException
    {
        Resolver resolver = queryResolver(s1, "fpi2l", s, null);
        if(resolver != null)
            return resolver.resolvePublic(s, null);
        else
            return null;
    }

    protected Resolver queryResolver(String s, String s1, String s2, String s3)
    {
        Object obj = null;
        String s4 = s + "?command=" + s1 + "&format=tr9401&uri=" + s2 + "&uri2=" + s3;
        Object obj1 = null;
        try
        {
            URL url = new URL(s4);
            System.out.println("Resolver 168 url: "+url);
            URLConnection urlconnection = url.openConnection();
            urlconnection.setUseCaches(false);
            Resolver resolver = (Resolver)newCatalog();
            String s5 = urlconnection.getContentType();
            if(s5.indexOf(";") > 0)
                s5 = s5.substring(0, s5.indexOf(";"));
            resolver.parseCatalog(s5, urlconnection.getInputStream());
            return resolver;
        }
        catch(CatalogException catalogexception)
        {
            if(catalogexception.getExceptionType() == 6)
                Debug.message(1, "Unparseable catalog: " + s4);
            else
            if(catalogexception.getExceptionType() == 5)
                Debug.message(1, "Unknown catalog format: " + s4);
            return null;
        }
        catch(MalformedURLException malformedurlexception)
        {
            Debug.message(1, "Malformed resolver URL: " + s4);
            return null;
        }
        catch(IOException ioexception)
        {
            Debug.message(1, "I/O Exception opening resolver: " + s4);
        }
        return null;
    }

    private Vector appendVector(Vector vector, Vector vector1)
    {
        if(vector1 != null)
        {
            for(int i = 0; i < vector1.size(); i++)
                vector.addElement(vector1.elementAt(i));

        }
        return vector;
    }

    public Vector resolveAllSystemReverse(String s)
        throws MalformedURLException, IOException
    {
        Vector vector = new Vector();
        if(s != null)
        {
            Vector vector1 = resolveLocalSystemReverse(s);
            vector = appendVector(vector, vector1);
        }
        Vector vector2 = resolveAllSubordinateCatalogs(SYSTEMREVERSE, null, null, s);
        return appendVector(vector, vector2);
    }

    public String resolveSystemReverse(String s)
        throws MalformedURLException, IOException
    {
        Vector vector = resolveAllSystemReverse(s);
        if(vector != null && vector.size() > 0)
            return (String)vector.elementAt(0);
        else
            return null;
    }

    public Vector resolveAllSystem(String s)
        throws MalformedURLException, IOException
    {
        Vector vector = new Vector();
        if(s != null)
        {
            Vector vector1 = resolveAllLocalSystem(s);
            vector = appendVector(vector, vector1);
        }
        Vector vector2 = resolveAllSubordinateCatalogs(Catalog.SYSTEM, null, null, s);
        vector = appendVector(vector, vector2);
        if(vector.size() > 0)
            return vector;
        else
            return null;
    }

    private Vector resolveAllLocalSystem(String s)
    {
        Vector vector = new Vector();
        String s1 = System.getProperty("os.name");
        boolean flag = s1.indexOf("Windows") >= 0;
        for(Enumeration enumeration = super.catalogEntries.elements(); enumeration.hasMoreElements();)
        {
            CatalogEntry catalogentry = (CatalogEntry)enumeration.nextElement();
            if(catalogentry.getEntryType() == Catalog.SYSTEM && (catalogentry.getEntryArg(0).equals(s) || flag && catalogentry.getEntryArg(0).equalsIgnoreCase(s)))
                vector.addElement(catalogentry.getEntryArg(1));
        }

        if(vector.size() == 0)
            return null;
        else
            return vector;
    }

    private Vector resolveLocalSystemReverse(String s)
    {
        Vector vector = new Vector();
        String s1 = System.getProperty("os.name");
        boolean flag = s1.indexOf("Windows") >= 0;
        for(Enumeration enumeration = super.catalogEntries.elements(); enumeration.hasMoreElements();)
        {
            CatalogEntry catalogentry = (CatalogEntry)enumeration.nextElement();
            if(catalogentry.getEntryType() == Catalog.SYSTEM && (catalogentry.getEntryArg(1).equals(s) || flag && catalogentry.getEntryArg(1).equalsIgnoreCase(s)))
                vector.addElement(catalogentry.getEntryArg(0));
        }

        if(vector.size() == 0)
            return null;
        else
            return vector;
    }

    private synchronized Vector resolveAllSubordinateCatalogs(int i, String s, String s1, String s2)
        throws MalformedURLException, IOException
    {
        Vector vector = new Vector();
        for(int j = 0; j < super.catalogs.size(); j++)
        {
            Resolver resolver = null;
            try
            {
                resolver = (Resolver)super.catalogs.elementAt(j);
            }
            catch(ClassCastException classcastexception)
            {
                String s8 = (String)super.catalogs.elementAt(j);
                resolver = (Resolver)newCatalog();
                try
                {
                    resolver.parseCatalog(s8);
                }
                catch(MalformedURLException malformedurlexception)
                {
                    Debug.message(1, "Malformed Catalog URL", s8);
                }
                catch(FileNotFoundException filenotfoundexception)
                {
                    Debug.message(1, "Failed to load catalog, file not found", s8);
                }
                catch(IOException ioexception)
                {
                    Debug.message(1, "Failed to load catalog, I/O error", s8);
                }
                super.catalogs.setElementAt(resolver, j);
            }
            Object obj = null;
            if(i == Catalog.DOCTYPE)
            {
                String s3 = resolver.resolveDoctype(s, s1, s2);
                if(s3 != null)
                {
                    vector.addElement(s3);
                    return vector;
                }
                continue;
            }
            if(i == Catalog.DOCUMENT)
            {
                String s4 = resolver.resolveDocument();
                if(s4 != null)
                {
                    vector.addElement(s4);
                    return vector;
                }
                continue;
            }
            if(i == Catalog.ENTITY)
            {
                String s5 = resolver.resolveEntity(s, s1, s2);
                if(s5 != null)
                {
                    vector.addElement(s5);
                    return vector;
                }
                continue;
            }
            if(i == Catalog.NOTATION)
            {
                String s6 = resolver.resolveNotation(s, s1, s2);
                if(s6 != null)
                {
                    vector.addElement(s6);
                    return vector;
                }
                continue;
            }
            if(i == Catalog.PUBLIC)
            {
                String s7 = resolver.resolvePublic(s1, s2);
                if(s7 != null)
                {
                    vector.addElement(s7);
                    return vector;
                }
                continue;
            }
            if(i == Catalog.SYSTEM)
            {
                Vector vector1 = resolver.resolveAllSystem(s2);
                vector = appendVector(vector, vector1);
                break;
            }
            if(i == SYSTEMREVERSE)
            {
                Vector vector2 = resolver.resolveAllSystemReverse(s2);
                vector = appendVector(vector, vector2);
            }
        }

        if(vector != null)
            return vector;
        else
            return null;
    }

    public static final int URISUFFIX = CatalogEntry.addEntryType("URISUFFIX", 2);
    public static final int SYSTEMSUFFIX = CatalogEntry.addEntryType("SYSTEMSUFFIX", 2);
    public static final int RESOLVER = CatalogEntry.addEntryType("RESOLVER", 1);
    public static final int SYSTEMREVERSE = CatalogEntry.addEntryType("SYSTEMREVERSE", 1);

}