// Decompiled by Jad v1.5.8e2. Copyright 2001 Pavel Kouznetsov.
// Jad home page: http://kpdus.tripod.com/jad.html
// Decompiler options: packfields(3) packimports(7) deadcode fieldsfirst splitstr(64) nonlb lnc radix(10) lradix(10) 

package org.apache.xml.resolver;

import java.io.*;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Vector;
import javax.xml.parsers.SAXParserFactory;
import org.apache.xml.resolver.helpers.Debug;
import org.apache.xml.resolver.helpers.PublicId;
import org.apache.xml.resolver.readers.CatalogReader;
import org.apache.xml.resolver.readers.SAXCatalogReader;
import org.apache.xml.resolver.readers.TR9401CatalogReader;

// Referenced classes of package org.apache.xml.resolver:
//            CatalogException, CatalogEntry, CatalogManager

public class Catalog {

    public static final int BASE = CatalogEntry.addEntryType("BASE", 1);
    public static final int CATALOG = CatalogEntry.addEntryType("CATALOG", 1);
    public static final int DOCUMENT = CatalogEntry.addEntryType("DOCUMENT", 1);
    public static final int OVERRIDE = CatalogEntry.addEntryType("OVERRIDE", 1);
    public static final int SGMLDECL = CatalogEntry.addEntryType("SGMLDECL", 1);
    public static final int DELEGATE_PUBLIC = CatalogEntry.addEntryType("DELEGATE_PUBLIC", 2);
    public static final int DELEGATE_SYSTEM = CatalogEntry.addEntryType("DELEGATE_SYSTEM", 2);
    public static final int DELEGATE_URI = CatalogEntry.addEntryType("DELEGATE_URI", 2);
    public static final int DOCTYPE = CatalogEntry.addEntryType("DOCTYPE", 2);
    public static final int DTDDECL = CatalogEntry.addEntryType("DTDDECL", 2);
    public static final int ENTITY = CatalogEntry.addEntryType("ENTITY", 2);
    public static final int LINKTYPE = CatalogEntry.addEntryType("LINKTYPE", 2);
    public static final int NOTATION = CatalogEntry.addEntryType("NOTATION", 2);
    public static final int PUBLIC = CatalogEntry.addEntryType("PUBLIC", 2);
    public static final int SYSTEM = CatalogEntry.addEntryType("SYSTEM", 2);
    public static final int URI = CatalogEntry.addEntryType("URI", 2);
    public static final int REWRITE_SYSTEM = CatalogEntry.addEntryType("REWRITE_SYSTEM", 2);
    public static final int REWRITE_URI = CatalogEntry.addEntryType("REWRITE_URI", 2);
    protected URL base, catalogCwd;
    protected Vector catalogEntries, catalogFiles, localCatalogFiles, catalogs;
    protected boolean default_override;
    protected Vector localDelegate, readerArr;
    protected Hashtable readerMap;

    public void setupReaders() {
        SAXParserFactory saxparserfactory = SAXParserFactory.newInstance();
        saxparserfactory.setNamespaceAware(true);
        saxparserfactory.setValidating(false);
        SAXCatalogReader saxcatalogreader = new SAXCatalogReader(saxparserfactory);
        saxcatalogreader.setCatalogParser(null, "XMLCatalog", "org.apache.xml.resolver.readers.XCatalogReader");
        saxcatalogreader.setCatalogParser("urn:oasis:names:tc:entity:xmlns:xml:catalog", "catalog", "org.apache.xml.resolver.readers.OASISXMLCatalogReader");
        addReader("application/xml", saxcatalogreader);
        TR9401CatalogReader tr9401catalogreader = new TR9401CatalogReader();
        addReader("text/plain", tr9401catalogreader);
    }

    public void addReader(String s, CatalogReader catalogreader) {
        if(readerMap.contains(s)) {
            Integer integer = (Integer)readerMap.get(s);
            readerArr.set(integer.intValue(), catalogreader);
        } else {
            readerArr.add(catalogreader);
            Integer integer1 = new Integer(readerArr.size() - 1);
            readerMap.put(s, integer1);
        }
    }

    protected void copyReaders(Catalog catalog) {
        Vector vector = new Vector(readerMap.size());
        for(int i = 0; i < readerMap.size(); i++)
            vector.add(null);

        String s;
        Integer integer;
        for(Enumeration enumeration = readerMap.keys(); enumeration.hasMoreElements(); vector.set(integer.intValue(), s)) {
            s = (String)enumeration.nextElement();
            integer = (Integer)readerMap.get(s);
        }

        for(int j = 0; j < vector.size(); j++) {
            String s1 = (String)vector.get(j);
            Integer integer1 = (Integer)readerMap.get(s1);
            catalog.addReader(s1, (CatalogReader)readerArr.get(integer1.intValue()));
        }

    }

    protected Catalog newCatalog() {
        String s = getClass().getName();
        try {
            Catalog catalog = (Catalog)Class.forName(s).newInstance();
            copyReaders(catalog);
            return catalog;
        }
        catch(ClassNotFoundException classnotfoundexception) {
          classnotfoundexception.printStackTrace();//  Debug.message(1, "Class Not Found Exception: " + s);
        }
        catch(IllegalAccessException illegalaccessexception) {
           illegalaccessexception.printStackTrace();// Debug.message(1, "Illegal Access Exception: " + s);
        }
        catch(InstantiationException instantiationexception) {
          instantiationexception.printStackTrace();//  Debug.message(1, "Instantiation Exception: " + s);
        }
        catch(ClassCastException classcastexception) {
           classcastexception.printStackTrace();// Debug.message(1, "Class Cast Exception: " + s);
        }
        catch(Exception exception) {
           exception.printStackTrace();// Debug.message(1, "Other Exception: " + s);
        }
        Catalog catalog1 = new Catalog();
        copyReaders(catalog1);
        return catalog1;
    }

    public Catalog() {
        catalogEntries = new Vector();
        default_override = true;
        catalogFiles = new Vector();
        localCatalogFiles = new Vector();
        catalogs = new Vector();
        localDelegate = new Vector();
        readerMap = new Hashtable();
        readerArr = new Vector();
        default_override = CatalogManager.preferPublic();
    }

    public String getCurrentBase() {
        return base.toString();
    }

    public String getDefaultOverride() {
        if(default_override)
            return "yes";
        else
            return "no";
    }

    public void loadSystemCatalogs() throws MalformedURLException, IOException {
        Vector vector = CatalogManager.catalogFiles();
        if(vector != null) {
            for(int i = 0; i < vector.size(); i++)
                catalogFiles.addElement(vector.elementAt(i));

        }
        if(catalogFiles.size() > 0) {
            String s = (String)catalogFiles.lastElement();
            catalogFiles.removeElement(s);
            parseCatalog(s);
        }
    }
    public synchronized void parseCatalog(String s) throws MalformedURLException, IOException {
        //Debug.message(4, "Parse catalog: " + s);
        catalogFiles.addElement(s);

        long startTime = System.currentTimeMillis();

        parsePendingCatalogs();

        int currentTime=(int)(System.currentTimeMillis()-startTime);
        System.out.println("Catalog 164 parse pending catalog time:"+currentTime+" ms");
    }

  public synchronized void parseCatalog(String s, InputStream inputstream) throws IOException, CatalogException {
       // Debug.message(4, "Parse " + s + " catalog on input stream");
        CatalogReader catalogreader = null;
        if(readerMap.containsKey(s)) {
            int i = ((Integer)readerMap.get(s)).intValue();
            catalogreader = (CatalogReader)readerArr.get(i);
        }
        if(catalogreader == null) {
            String s1 = "No CatalogReader for MIME type: " + s;
           // Debug.message(2, s1);
            throw new CatalogException(6, s1);
        } else {
            catalogreader.readCatalog(this, inputstream);

            long startTime = System.currentTimeMillis();
            parsePendingCatalogs();

            int currentTime=(int)(System.currentTimeMillis()-startTime);
            System.out.println("Catalog 185 parse pending catalog time:"+currentTime+" ms");
            return;
        }
    }

    protected synchronized void parsePendingCatalogs() throws MalformedURLException, IOException {

        if(!localCatalogFiles.isEmpty()) {
            Vector vector = new Vector();
            for(Enumeration enumeration1 = localCatalogFiles.elements(); enumeration1.hasMoreElements(); vector.addElement(enumeration1.nextElement()));
            for(int i = 0; i < catalogFiles.size(); i++) {
                String s2 = (String)catalogFiles.elementAt(i);
                vector.addElement(s2);
            }

            catalogFiles = vector;
            localCatalogFiles.clear();
        }
        if(catalogFiles.isEmpty() && !localDelegate.isEmpty()) {
            for(Enumeration enumeration = localDelegate.elements(); enumeration.hasMoreElements(); catalogEntries.addElement(enumeration.nextElement()));
            localDelegate.clear();
        }
        while(!catalogFiles.isEmpty())  {
            String s = (String)catalogFiles.elementAt(0);
            try {
                catalogFiles.remove(0);
            }
            catch(ArrayIndexOutOfBoundsException arrayindexoutofboundsexception) { }
            if(catalogEntries.size() == 0 && catalogs.size() == 0)
                try {
                    System.out.println("Catalog Start parse catalog file ... "+s);
                    long startTime = System.currentTimeMillis();

                    parseCatalogFile(s);

                    int currentTime=(int)(System.currentTimeMillis()-startTime);                    
                    System.out.println("Catalog Catalog file parsed. Time:"+currentTime+" ms");
                }
                catch(CatalogException catalogexception) {
                    System.out.println("FIXME: " + catalogexception.toString());
                }
            else
                catalogs.addElement(s);
            if(!localCatalogFiles.isEmpty()) {
                Vector vector1 = new Vector();
                for(Enumeration enumeration3 = localCatalogFiles.elements(); enumeration3.hasMoreElements(); vector1.addElement(enumeration3.nextElement()));
                for(int j = 0; j < catalogFiles.size(); j++) {
                    String s1 = (String)catalogFiles.elementAt(j);
                    vector1.addElement(s1);
                }

                catalogFiles = vector1;
                localCatalogFiles.clear();
            }
            if(!localDelegate.isEmpty()) {
                for(Enumeration enumeration2 = localDelegate.elements(); enumeration2.hasMoreElements(); catalogEntries.addElement(enumeration2.nextElement()));
                localDelegate.clear();
            }
        }
        catalogFiles.clear();
    }
    protected synchronized void parseCatalogFile(String s) throws MalformedURLException, IOException, CatalogException {

      /*  try {
            String s1 = fixSlashes(System.getProperty("user.dir"));
            catalogCwd = new URL("file:" + s1 + "/basename");
        }
        catch(MalformedURLException malformedurlexception) {
           // String s2 = fixSlashes(System.getProperty("user.dir"));
            //Debug.message(1, "Malformed URL on cwd", s2);
            catalogCwd = null;
        } */

        try {
            // todo debug -commnet
            //base= new URL("jar:http://localhost:20801/msag/rules/rules/jedit.jar!/xml/dtds/catalog");

            base = new URL(catalogCwd, fixSlashes(s));
            System.out.println("Catalog 251 url: "+base);
        }
        catch(MalformedURLException malformedurlexception1) {
            try {
                base = new URL("file:" + fixSlashes(s));
                System.out.println("Catalog 256 url: "+base);
            }
            catch(MalformedURLException malformedurlexception2) {
               // Debug.message(1, "Malformed URL on catalog filename", fixSlashes(s));
                base = null;
            }
        }
       // Debug.message(2, "Loading catalog", s);
       // Debug.message(4, "Default BASE", base.toString());
        s = base.toString();
        Object obj = null;
        boolean flag = false;
        boolean flag1 = false;
        for(int i = 0; !flag && i < readerArr.size(); i++) {
            CatalogReader catalogreader = (CatalogReader)readerArr.get(i);
            DataInputStream datainputstream;
            try {
                flag1 = false;
                System.out.println("Catalog open input stream: base="+base);
                datainputstream = new DataInputStream(base.openStream());
            }
            catch(FileNotFoundException filenotfoundexception) {
                flag1 = true;
                break;
            }
            try {
                System.out.println("Catalog Read catalog .. base="+base);
                long startTime = System.currentTimeMillis();
                catalogreader.readCatalog(this, datainputstream);
                int currentTime=(int)(System.currentTimeMillis()-startTime);              
                System.out.println("Catalog Catalog was read time:"+currentTime+" ms");
                flag = true;
            }
            catch(CatalogException catalogexception) {
                if(catalogexception.getExceptionType() == 7)
                    break;
            }
            try {
                datainputstream.close();
            }
            catch(IOException ioexception) { }
        }

        if(!flag)
            if(flag1) System.out.println("Catalog does not exist"+s);//Debug.message(3, "Catalog does not exist", s);
            else      System.out.println("Failed to parse catalog"+s);//Debug.message(1, "Failed to parse catalog", s);
    }
    public void addEntry(CatalogEntry catalogentry) {
        int i = catalogentry.getEntryType();
        if(i == BASE) {
            String s = catalogentry.getEntryArg(0);
            URL url = null;
          //  Debug.message(5, "BASE CUR", base.toString());
          //  Debug.message(4, "BASE STR", s);
            try {
                s = fixSlashes(s);
                url = new URL(base, s);
                System.out.println("Catalog url: "+url);
            }
            catch(MalformedURLException malformedurlexception) {
                try {
                    url = new URL("file:" + s);
                    System.out.println("Catalog 311 url: "+url);
                }
                catch(MalformedURLException malformedurlexception1) {
                  malformedurlexception1.printStackTrace();//  Debug.message(1, "Malformed URL on base", s);
                    url = null;
                }
            }
            if(url != null)
                base = url;
           // Debug.message(5, "BASE NEW", base.toString());
        } else
        if(i == CATALOG) {
            String s1 = makeAbsolute(catalogentry.getEntryArg(0));
           // Debug.message(4, "CATALOG", s1);
            localCatalogFiles.addElement(s1);
        } else
        if(i == PUBLIC) {
            String s2 = PublicId.normalize(catalogentry.getEntryArg(0));
            String s17 = makeAbsolute(normalizeURI(catalogentry.getEntryArg(1)));
            catalogentry.setEntryArg(0, s2);
            catalogentry.setEntryArg(1, s17);
            //Debug.message(4, "PUBLIC", s2, s17);
            catalogEntries.addElement(catalogentry);
        } else
        if(i == SYSTEM) {
            String s3 = normalizeURI(catalogentry.getEntryArg(0));
            String s18 = makeAbsolute(normalizeURI(catalogentry.getEntryArg(1)));
            catalogentry.setEntryArg(1, s18);
            //Debug.message(4, "SYSTEM", s3, s18);
            catalogEntries.addElement(catalogentry);
        } else
        if(i == URI) {
            String s4 = normalizeURI(catalogentry.getEntryArg(0));
            String s19 = makeAbsolute(normalizeURI(catalogentry.getEntryArg(1)));
            catalogentry.setEntryArg(1, s19);
            //Debug.message(4, "URI", s4, s19);
            catalogEntries.addElement(catalogentry);
        } else
        if(i == DOCUMENT) {
            String s5 = makeAbsolute(normalizeURI(catalogentry.getEntryArg(0)));
            catalogentry.setEntryArg(0, s5);
            //Debug.message(4, "DOCUMENT", s5);
            catalogEntries.addElement(catalogentry);
        } else
        if(i == OVERRIDE) {
            //Debug.message(4, "OVERRIDE", catalogentry.getEntryArg(0));
            catalogEntries.addElement(catalogentry);
        } else
        if(i == SGMLDECL) {
            String s6 = makeAbsolute(normalizeURI(catalogentry.getEntryArg(0)));
            catalogentry.setEntryArg(0, s6);
            //Debug.message(4, "SGMLDECL", s6);
            catalogEntries.addElement(catalogentry);
        } else
        if(i == DELEGATE_PUBLIC) {
            String s7 = PublicId.normalize(catalogentry.getEntryArg(0));
            String s20 = makeAbsolute(normalizeURI(catalogentry.getEntryArg(1)));
            catalogentry.setEntryArg(0, s7);
            catalogentry.setEntryArg(1, s20);
            //Debug.message(4, "DELEGATE_PUBLIC", s7, s20);
            addDelegate(catalogentry);
        } else
        if(i == DELEGATE_SYSTEM) {
            String s8 = normalizeURI(catalogentry.getEntryArg(0));
            String s21 = makeAbsolute(normalizeURI(catalogentry.getEntryArg(1)));
            catalogentry.setEntryArg(0, s8);
            catalogentry.setEntryArg(1, s21);
            //Debug.message(4, "DELEGATE_SYSTEM", s8, s21);
            addDelegate(catalogentry);
        } else
        if(i == DELEGATE_URI) {
            String s9 = normalizeURI(catalogentry.getEntryArg(0));
            String s22 = makeAbsolute(normalizeURI(catalogentry.getEntryArg(1)));
            catalogentry.setEntryArg(0, s9);
            catalogentry.setEntryArg(1, s22);
            //Debug.message(4, "DELEGATE_URI", s9, s22);
            addDelegate(catalogentry);
        } else
        if(i == REWRITE_SYSTEM) {
            String s10 = normalizeURI(catalogentry.getEntryArg(0));
            String s23 = makeAbsolute(normalizeURI(catalogentry.getEntryArg(1)));
            catalogentry.setEntryArg(0, s10);
            catalogentry.setEntryArg(1, s23);
            //Debug.message(4, "REWRITE_SYSTEM", s10, s23);
            catalogEntries.addElement(catalogentry);
        } else
        if(i == REWRITE_URI) {
            String s11 = normalizeURI(catalogentry.getEntryArg(0));
            String s24 = makeAbsolute(normalizeURI(catalogentry.getEntryArg(1)));
            catalogentry.setEntryArg(0, s11);
            catalogentry.setEntryArg(1, s24);
            //Debug.message(4, "REWRITE_URI", s11, s24);
            catalogEntries.addElement(catalogentry);
        } else
        if(i == DOCTYPE) {
            String s12 = makeAbsolute(normalizeURI(catalogentry.getEntryArg(1)));
            catalogentry.setEntryArg(1, s12);
            //Debug.message(4, "DOCTYPE", catalogentry.getEntryArg(0), s12);
            catalogEntries.addElement(catalogentry);
        } else
        if(i == DTDDECL) {
            String s13 = PublicId.normalize(catalogentry.getEntryArg(0));
            catalogentry.setEntryArg(0, s13);
            String s25 = makeAbsolute(normalizeURI(catalogentry.getEntryArg(1)));
            catalogentry.setEntryArg(1, s25);
            //Debug.message(4, "DTDDECL", s13, s25);
            catalogEntries.addElement(catalogentry);
        } else
        if(i == ENTITY) {
            String s14 = makeAbsolute(normalizeURI(catalogentry.getEntryArg(1)));
            catalogentry.setEntryArg(1, s14);
            //Debug.message(4, "ENTITY", catalogentry.getEntryArg(0), s14);
            catalogEntries.addElement(catalogentry);
        } else
        if(i == LINKTYPE) {
            String s15 = makeAbsolute(normalizeURI(catalogentry.getEntryArg(1)));
            catalogentry.setEntryArg(1, s15);
            //Debug.message(4, "LINKTYPE", catalogentry.getEntryArg(0), s15);
            catalogEntries.addElement(catalogentry);
        } else
        if(i == NOTATION) {
            String s16 = makeAbsolute(normalizeURI(catalogentry.getEntryArg(1)));
            catalogentry.setEntryArg(1, s16);
            //Debug.message(4, "NOTATION", catalogentry.getEntryArg(0), s16);
            catalogEntries.addElement(catalogentry);
        } else {
            catalogEntries.addElement(catalogentry);
        }
    }

    public void unknownEntry(Vector vector) {
        if(vector != null && vector.size() > 0) {
            String s = (String)vector.elementAt(0);
            //Debug.message(2, "Unrecognized token parsing catalog", s);
        }
    }

    public void parseAllCatalogs() throws MalformedURLException, IOException {
        for(int i = 0; i < catalogs.size(); i++) {
            Catalog catalog = null;
            try {
                catalog = (Catalog)catalogs.elementAt(i);
            }
            catch(ClassCastException classcastexception) {
                String s = (String)catalogs.elementAt(i);
                Catalog catalog1 = newCatalog();
                catalog1.parseCatalog(s);
                catalogs.setElementAt(catalog1, i);
                catalog1.parseAllCatalogs();
            }
        }

        for(Enumeration enumeration = catalogEntries.elements(); enumeration.hasMoreElements();) {
            CatalogEntry catalogentry = (CatalogEntry)enumeration.nextElement();
            if(catalogentry.getEntryType() == DELEGATE_PUBLIC || catalogentry.getEntryType() == DELEGATE_SYSTEM || catalogentry.getEntryType() == DELEGATE_URI) {
                Catalog catalog2 = newCatalog();
                catalog2.parseCatalog(catalogentry.getEntryArg(1));
            }
        }

    }

    public String resolveDoctype(String s, String s1, String s2) throws MalformedURLException, IOException {
        Object obj = null;
        //Debug.message(3, "resolveDoctype(" + s + "," + s1 + "," + s2 + ")");
        s2 = normalizeURI(s2);
        if(s1 != null && s1.startsWith("urn:publicid:"))
            s1 = PublicId.decodeURN(s1);
        if(s2 != null && s2.startsWith("urn:publicid:")) {
            s2 = PublicId.decodeURN(s2);
            if(s1 != null && !s1.equals(s2)) {
                //Debug.message(1, "urn:publicid: system identifier differs from public identifier; "+"using public identifier");
                s2 = null;
            } else {
                s1 = s2;
                s2 = null;
            }
        }
        if(s2 != null) {
            String s3 = resolveLocalSystem(s2);
            if(s3 != null)
                return s3;
        }
        if(s1 != null) {
            String s4 = resolveLocalPublic(DOCTYPE, s, s1, s2);
            if(s4 != null)
                return s4;
        }
        boolean flag = default_override;
        for(Enumeration enumeration = catalogEntries.elements(); enumeration.hasMoreElements();) {
            CatalogEntry catalogentry = (CatalogEntry)enumeration.nextElement();
            if(catalogentry.getEntryType() == OVERRIDE)
                flag = catalogentry.getEntryArg(0).equalsIgnoreCase("YES");
            else
            if(catalogentry.getEntryType() == DOCTYPE && catalogentry.getEntryArg(0).equals(s) && (flag || s2 == null))
                return catalogentry.getEntryArg(1);
        }

        return resolveSubordinateCatalogs(DOCTYPE, s, s1, s2);
    }

    public String resolveDocument() throws MalformedURLException, IOException {
        //Debug.message(3, "resolveDocument");
        for(Enumeration enumeration = catalogEntries.elements(); enumeration.hasMoreElements();) {
            CatalogEntry catalogentry = (CatalogEntry)enumeration.nextElement();
            if(catalogentry.getEntryType() == DOCUMENT)
                return catalogentry.getEntryArg(1);
        }

        return resolveSubordinateCatalogs(DOCUMENT, null, null, null);
    }

    public String resolveEntity(String s, String s1, String s2) throws MalformedURLException, IOException {
        Object obj = null;
        //Debug.message(3, "resolveEntity(" + s + "," + s1 + "," + s2 + ")");
        s2 = normalizeURI(s2);
        if(s1 != null && s1.startsWith("urn:publicid:"))
            s1 = PublicId.decodeURN(s1);
        if(s2 != null && s2.startsWith("urn:publicid:")) {
            s2 = PublicId.decodeURN(s2);
            if(s1 != null && !s1.equals(s2)) {
                //Debug.message(1, "urn:publicid: system identifier differs from public identifier; "+"using public identifier");
                s2 = null;
            } else {
                s1 = s2;
                s2 = null;
            }
        }
        if(s2 != null) {
            String s3 = resolveLocalSystem(s2);
            if(s3 != null)
                return s3;
        }
        if(s1 != null) {
            String s4 = resolveLocalPublic(ENTITY, s, s1, s2);
            if(s4 != null)
                return s4;
        }
        boolean flag = default_override;
        for(Enumeration enumeration = catalogEntries.elements(); enumeration.hasMoreElements();) {
            CatalogEntry catalogentry = (CatalogEntry)enumeration.nextElement();
            if(catalogentry.getEntryType() == OVERRIDE)
                flag = catalogentry.getEntryArg(0).equalsIgnoreCase("YES");
            else
            if(catalogentry.getEntryType() == ENTITY && catalogentry.getEntryArg(0).equals(s) && (flag || s2 == null))
                return catalogentry.getEntryArg(1);
        }

        return resolveSubordinateCatalogs(ENTITY, s, s1, s2);
    }

    public String resolveNotation(String s, String s1, String s2) throws MalformedURLException, IOException {
        Object obj = null;
        //Debug.message(3, "resolveNotation(" + s + "," + s1 + "," + s2 + ")");
        s2 = normalizeURI(s2);
        if(s1 != null && s1.startsWith("urn:publicid:"))
            s1 = PublicId.decodeURN(s1);
        if(s2 != null && s2.startsWith("urn:publicid:")) {
            s2 = PublicId.decodeURN(s2);
            if(s1 != null && !s1.equals(s2)) {
                //Debug.message(1, "urn:publicid: system identifier differs from public identifier; "+"using public identifier");
                s2 = null;
            } else {
                s1 = s2;
                s2 = null;
            }
        }
        if(s2 != null) {
            String s3 = resolveLocalSystem(s2);
            if(s3 != null)
                return s3;
        }
        if(s1 != null) {
            String s4 = resolveLocalPublic(NOTATION, s, s1, s2);
            if(s4 != null)
                return s4;
        }
        boolean flag = default_override;
        for(Enumeration enumeration = catalogEntries.elements(); enumeration.hasMoreElements();) {
            CatalogEntry catalogentry = (CatalogEntry)enumeration.nextElement();
            if(catalogentry.getEntryType() == OVERRIDE)
                flag = catalogentry.getEntryArg(0).equalsIgnoreCase("YES");
            else
            if(catalogentry.getEntryType() == NOTATION && catalogentry.getEntryArg(0).equals(s) && (flag || s2 == null))
                return catalogentry.getEntryArg(1);
        }

        return resolveSubordinateCatalogs(NOTATION, s, s1, s2);
    }

    public String resolvePublic(String s, String s1) throws MalformedURLException, IOException {
        //Debug.message(3, "resolvePublic(" + s + "," + s1 + ")");
        s1 = normalizeURI(s1);
        if(s != null && s.startsWith("urn:publicid:"))
            s = PublicId.decodeURN(s);
        if(s1 != null && s1.startsWith("urn:publicid:")) {
            s1 = PublicId.decodeURN(s1);
            if(s != null && !s.equals(s1)) {
               // Debug.message(1, "urn:publicid: system identifier differs from public identifier; "+"using public identifier");
                s1 = null;
            } else {
                s = s1;
                s1 = null;
            }
        }
        if(s1 != null) {
            String s2 = resolveLocalSystem(s1);
            if(s2 != null)
                return s2;
        }
        String s3 = resolveLocalPublic(PUBLIC, null, s, s1);
        System.out.println("Catalog resolvePublic avter resolveLocalPublic s3= "+s3);
        if(s3 != null)
            return s3;
        else
            return resolveSubordinateCatalogs(PUBLIC, null, s, s1);
    }

    protected synchronized String resolveLocalPublic(int i, String s, String s1, String s2) throws MalformedURLException, IOException {
        s1 = PublicId.normalize(s1);
        if(s2 != null) {
            String s3 = resolveLocalSystem(s2);
            if(s3 != null)
                return s3;
        }
        boolean flag = default_override;
        for(Enumeration enumeration = catalogEntries.elements(); enumeration.hasMoreElements();) {
            CatalogEntry catalogentry = (CatalogEntry)enumeration.nextElement();
            if(catalogentry.getEntryType() == OVERRIDE)
                flag = catalogentry.getEntryArg(0).equalsIgnoreCase("YES");
            else
            if(catalogentry.getEntryType() == PUBLIC && catalogentry.getEntryArg(0).equals(s1) && (flag || s2 == null))
                return catalogentry.getEntryArg(1);
        }

        flag = default_override;
        Enumeration enumeration1 = catalogEntries.elements();
        Vector vector = new Vector();
        while(enumeration1.hasMoreElements())  {
            CatalogEntry catalogentry1 = (CatalogEntry)enumeration1.nextElement();
            if(catalogentry1.getEntryType() == OVERRIDE)
                flag = catalogentry1.getEntryArg(0).equalsIgnoreCase("YES");
            else
            if(catalogentry1.getEntryType() == DELEGATE_PUBLIC && (flag || s2 == null)) {
                String s4 = catalogentry1.getEntryArg(0);
                if(s4.length() <= s1.length() && s4.equals(s1.substring(0, s4.length())))
                    vector.addElement(catalogentry1.getEntryArg(1));
            }
        }
        if(vector.size() > 0) {
            Enumeration enumeration2 = vector.elements();
          /*  if(Debug.getDebug() > 1) {
                Debug.message(2, "Switching to delegated catalog(s):");
                String s5;
                for(; enumeration2.hasMoreElements(); Debug.message(2, "\t" + s5))
                    s5 = (String)enumeration2.nextElement();

            }  */
            Catalog catalog = newCatalog();
            String s6;
            for(Enumeration enumeration3 = vector.elements(); enumeration3.hasMoreElements(); catalog.parseCatalog(s6))
                s6 = (String)enumeration3.nextElement();

            return catalog.resolvePublic(s1, null);
        } else {
            return null;
        }
    }

    public String resolveSystem(String s) throws MalformedURLException, IOException {
       // Debug.message(3, "resolveSystem(" + s + ")");
        s = normalizeURI(s);
        if(s != null && s.startsWith("urn:publicid:")) {
            s = PublicId.decodeURN(s);
            return resolvePublic(s, null);
        }
        if(s != null) {
            String s1 = resolveLocalSystem(s);
            if(s1 != null)
                return s1;
        }
        return resolveSubordinateCatalogs(SYSTEM, null, null, s);
    }

    protected String resolveLocalSystem(String s) throws MalformedURLException, IOException {
        String s1 = System.getProperty("os.name");
        boolean flag = s1.indexOf("Windows") >= 0;
        for(Enumeration enumeration = catalogEntries.elements(); enumeration.hasMoreElements();) {
            CatalogEntry catalogentry = (CatalogEntry)enumeration.nextElement();
            if(catalogentry.getEntryType() == SYSTEM && (catalogentry.getEntryArg(0).equals(s) || flag && catalogentry.getEntryArg(0).equalsIgnoreCase(s)))
                return catalogentry.getEntryArg(1);
        }

        Enumeration enumeration1 = catalogEntries.elements();
        String s2 = null;
        String s3 = null;
        while(enumeration1.hasMoreElements())  {
            CatalogEntry catalogentry1 = (CatalogEntry)enumeration1.nextElement();
            if(catalogentry1.getEntryType() == REWRITE_SYSTEM) {
                String s4 = catalogentry1.getEntryArg(0);
                if(s4.length() <= s.length() && s4.equals(s.substring(0, s4.length())) && (s2 == null || s4.length() > s2.length())) {
                    s2 = s4;
                    s3 = catalogentry1.getEntryArg(1);
                }
            }
            if(s3 != null)
                return s3 + s.substring(s2.length());
        }
        enumeration1 = catalogEntries.elements();
        Vector vector = new Vector();
        while(enumeration1.hasMoreElements())  {
            CatalogEntry catalogentry2 = (CatalogEntry)enumeration1.nextElement();
            if(catalogentry2.getEntryType() == DELEGATE_SYSTEM) {
                String s5 = catalogentry2.getEntryArg(0);
                if(s5.length() <= s.length() && s5.equals(s.substring(0, s5.length())))
                    vector.addElement(catalogentry2.getEntryArg(1));
            }
        }
        if(vector.size() > 0) {
            Enumeration enumeration2 = vector.elements();
           /* if(Debug.getDebug() > 1) {
                Debug.message(2, "Switching to delegated catalog(s):");
                String s6;
                for(; enumeration2.hasMoreElements(); Debug.message(2, "\t" + s6))
                    s6 = (String)enumeration2.nextElement();

            } */
            Catalog catalog = newCatalog();
            String s7;
            for(Enumeration enumeration3 = vector.elements(); enumeration3.hasMoreElements(); catalog.parseCatalog(s7))
                s7 = (String)enumeration3.nextElement();

            return catalog.resolveSystem(s);
        } else {
            return null;
        }
    }

    public String resolveURI(String s) throws MalformedURLException, IOException {
       // Debug.message(3, "resolveURI(" + s + ")");
        s = normalizeURI(s);
        if(s != null && s.startsWith("urn:publicid:")) {
            s = PublicId.decodeURN(s);
            return resolvePublic(s, null);
        }
        if(s != null) {
            String s1 = resolveLocalURI(s);
            if(s1 != null)
                return s1;
        }
        return resolveSubordinateCatalogs(URI, null, null, s);
    }

    protected String resolveLocalURI(String s) throws MalformedURLException, IOException {
        for(Enumeration enumeration = catalogEntries.elements(); enumeration.hasMoreElements();) {
            CatalogEntry catalogentry = (CatalogEntry)enumeration.nextElement();
            if(catalogentry.getEntryType() == URI && catalogentry.getEntryArg(0).equals(s))
                return catalogentry.getEntryArg(1);
        }

        Enumeration enumeration1 = catalogEntries.elements();
        String s1 = null;
        String s2 = null;
        while(enumeration1.hasMoreElements())  {
            CatalogEntry catalogentry1 = (CatalogEntry)enumeration1.nextElement();
            if(catalogentry1.getEntryType() == REWRITE_URI) {
                String s3 = catalogentry1.getEntryArg(0);
                if(s3.length() <= s.length() && s3.equals(s.substring(0, s3.length())) && (s1 == null || s3.length() > s1.length())) {
                    s1 = s3;
                    s2 = catalogentry1.getEntryArg(1);
                }
            }
            if(s2 != null)
                return s2 + s.substring(s1.length());
        }
        enumeration1 = catalogEntries.elements();
        Vector vector = new Vector();
        while(enumeration1.hasMoreElements())  {
            CatalogEntry catalogentry2 = (CatalogEntry)enumeration1.nextElement();
            if(catalogentry2.getEntryType() == DELEGATE_URI) {
                String s4 = catalogentry2.getEntryArg(0);
                if(s4.length() <= s.length() && s4.equals(s.substring(0, s4.length())))
                    vector.addElement(catalogentry2.getEntryArg(1));
            }
        }
        if(vector.size() > 0) {
            Enumeration enumeration2 = vector.elements();
          /*  if(Debug.getDebug() > 1) {
                Debug.message(2, "Switching to delegated catalog(s):");
                String s5;
                for(; enumeration2.hasMoreElements(); Debug.message(2, "\t" + s5))
                    s5 = (String)enumeration2.nextElement();

            }  */
            Catalog catalog = newCatalog();
            String s6;
            for(Enumeration enumeration3 = vector.elements(); enumeration3.hasMoreElements(); catalog.parseCatalog(s6))
                s6 = (String)enumeration3.nextElement();

            return catalog.resolveURI(s);
        } else {
            return null;
        }
    }

    protected synchronized String resolveSubordinateCatalogs(int i, String s, String s1, String s2) throws MalformedURLException, IOException {
        for(int j = 0; j < catalogs.size(); j++) {
            Catalog catalog = null;
            try {
                catalog = (Catalog)catalogs.elementAt(j);
            }
            catch(ClassCastException classcastexception) {
                String s4 = (String)catalogs.elementAt(j);
                catalog = newCatalog();
                try {
                    catalog.parseCatalog(s4);
                }
                catch(MalformedURLException malformedurlexception) {
                  //  Debug.message(1, "Malformed Catalog URL", s4);
                }
                catch(FileNotFoundException filenotfoundexception) {
                  //  Debug.message(1, "Failed to load catalog, file not found", s4);
                }
                catch(IOException ioexception) {
                   // Debug.message(1, "Failed to load catalog, I/O error", s4);
                }
                catalogs.setElementAt(catalog, j);
            }
            String s3 = null;
            if(i == DOCTYPE)
                s3 = catalog.resolveDoctype(s, s1, s2);
            else
            if(i == DOCUMENT)
                s3 = catalog.resolveDocument();
            else
            if(i == ENTITY)
                s3 = catalog.resolveEntity(s, s1, s2);
            else
            if(i == NOTATION)
                s3 = catalog.resolveNotation(s, s1, s2);
            else
            if(i == PUBLIC)
                s3 = catalog.resolvePublic(s1, s2);
            else
            if(i == SYSTEM)
                s3 = catalog.resolveSystem(s2);
            else
            if(i == URI)
                s3 = catalog.resolveURI(s2);
            if(s3 != null)
                return s3;
        }

        return null;
    }

    protected String fixSlashes(String s) {
        return s.replace('\\', '/');
    }

    protected String makeAbsolute(String s) {
        URL url = null;
        s = fixSlashes(s);
        try {
            url = new URL(base, s);
            System.out.println("Catalog 876 url: "+url);
        }
        catch(MalformedURLException malformedurlexception) {
           malformedurlexception.printStackTrace();// Debug.message(1, "Malformed URL on system identifier", s);
        }
        if(url != null)
            return url.toString();
        else
            return s;
    }

    protected String normalizeURI(String s) {
        String s1 = "";
        if(s == null)
            return null;
        byte abyte0[];
        try {
            abyte0 = s.getBytes("UTF-8");
        }
        catch(UnsupportedEncodingException unsupportedencodingexception) {
           unsupportedencodingexception.printStackTrace();// Debug.message(1, "UTF-8 is an unsupported encoding!?");
            return s;
        }
        for(int i = 0; i < abyte0.length; i++) {
            int j = abyte0[i] & 255;
            if(j <= 32 || j > 127 || j == 34 || j == 60 || j == 62 || j == 92 || j == 94 || j == 96 || j == 123 || j == 124 || j == 125 || j == 127)
                s1 = s1 + encodedByte(j);
            else
                s1 = s1 + (char)abyte0[i];
        }

        return s1;
    }

    protected String encodedByte(int i) {
        String s = Integer.toHexString(i).toUpperCase();
        if(s.length() < 2)
            return "%0" + s;
        else
            return "%" + s;
    }

    protected void addDelegate(CatalogEntry catalogentry) {
        int i = 0;
        String s = catalogentry.getEntryArg(0);
        for(Enumeration enumeration = localDelegate.elements(); enumeration.hasMoreElements();) {
            CatalogEntry catalogentry1 = (CatalogEntry)enumeration.nextElement();
            String s1 = catalogentry1.getEntryArg(0);
            if(s1.equals(s))
                return;
            if(s1.length() > s.length())
                i++;
            if(s1.length() < s.length())
                break;
        }

        if(localDelegate.size() == 0)
            localDelegate.addElement(catalogentry);
        else
            localDelegate.insertElementAt(catalogentry, i);
    }

}
