// Decompiled by Jad v1.5.8e2. Copyright 2001 Pavel Kouznetsov.
// Jad home page: http://kpdus.tripod.com/jad.html
// Decompiler options: packfields(3) packimports(7) deadcode fieldsfirst splitstr(64) nonlb lnc radix(10) lradix(10) 

package org.apache.xml.resolver;

import java.io.IOException;
import java.io.PrintStream;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.MissingResourceException;
import java.util.PropertyResourceBundle;
import java.util.ResourceBundle;
import java.util.StringTokenizer;
import java.util.Vector;

public class CatalogManager {

    private static String pFiles, pIgnoreMissing;
    private static String pVerbosity = "xml.catalog.verbosity";
    private static String pPrefer = "xml.catalog.prefer";
    private static String pStatic = "xml.catalog.staticCatalog";
    private static String pAllowPI = "xml.catalog.allowPI";
    private static String pClassname = "xml.catalog.className";
    private static boolean ignoreMissingProperties;
    private static ResourceBundle resources;
    private static String propertyFile = "CatalogManager.properties";
    private static URL propertyFileURI = null;
    private static String defaultCatalogFiles = "./xcatalog";
    private static int defaultVerbosity = 1;
    private static boolean defaultPreferPublic = true;
    private static boolean defaultStaticCatalog = true;
    private static boolean defaultOasisXMLCatalogPI = true;
    private static boolean defaultRelativeCatalogs = true;

    public CatalogManager() {
    }

    private static synchronized void readProperties() {
        try {
            propertyFileURI = (org.apache.xml.resolver.CatalogManager.class).getResource("/" + propertyFile);
            System.out.println("CatalogManager.readProperties() 42 propertyFileURI="+propertyFileURI);
            java.io.InputStream inputstream = (org.apache.xml.resolver.CatalogManager.class).getResourceAsStream("/" + propertyFile);
            if(inputstream == null) {
                if(!ignoreMissingProperties)
                    System.err.println("Catalog manager Cannot find " + propertyFile);
                return;
            }
            resources = new PropertyResourceBundle(inputstream);
        }
        catch(MissingResourceException missingresourceexception) {
            if(!ignoreMissingProperties)
                System.err.println("Cannot read " + propertyFile);
        }
        catch(IOException ioexception) {
            if(!ignoreMissingProperties)
                System.err.println("Failure trying to read " + propertyFile);
        }
    }

    public static void ignoreMissingProperties(boolean flag) {
        ignoreMissingProperties = flag;
    }

    public static int verbosity() {
        String s = System.getProperty(pVerbosity);
        if(s == null) {
            if(resources == null)
                readProperties();
            if(resources == null)
                return defaultVerbosity;
            try {
                s = resources.getString("verbosity");
            }
            catch(MissingResourceException missingresourceexception) {
                return defaultVerbosity;
            }
        }
        try {
            int i = Integer.parseInt(s.trim());
            return i;
        }
        catch(Exception exception) {
            System.err.println("Cannot parse verbosity: \"" + s + "\"");
        }
        return defaultVerbosity;
    }

    public static boolean relativeCatalogs() {
        if(resources == null)
            readProperties();
        if(resources == null)
            return defaultRelativeCatalogs;
        try {
            String s = resources.getString("relative-catalogs");
            return s.equalsIgnoreCase("true") || s.equalsIgnoreCase("yes") || s.equalsIgnoreCase("1");
        }
        catch(MissingResourceException missingresourceexception) {
            return defaultRelativeCatalogs;
        }
    }

    public static Vector catalogFiles() {
        String s = null;//System.getProperty(pFiles);
        boolean flag = false;
        if(s == null) {
            if(resources == null)
                readProperties();
            if(resources != null)
                try {
                    s = resources.getString("catalogs");
                    flag = true;
                }
                catch(MissingResourceException missingresourceexception) {
                    System.err.println(propertyFile + ": catalogs not found.");
                    s = null;
                }
        }
        if(s == null)
            s = defaultCatalogFiles;
        StringTokenizer stringtokenizer = new StringTokenizer(s, ";");
        Vector vector = new Vector();
        String s1;
        for(; stringtokenizer.hasMoreTokens(); vector.add(s1)) {
            s1 = stringtokenizer.nextToken();
            URL url = null;
            if(flag && !relativeCatalogs())
                try {
                    url = new URL(propertyFileURI, s1);
                    System.out.println("CatalogManager 129 url: "+url);
                    s1 = url.toString();
                }
                catch(MalformedURLException malformedurlexception) {
                    url = null;
                }
        }

        return vector;
    }

    public static boolean preferPublic() {
      String s =null;// System.getProperty(pPrefer);
        if(s == null) {
            if(resources == null)
                readProperties();
            if(resources == null)
                return defaultPreferPublic;
            try {
                s = resources.getString("prefer");
            }
            catch(MissingResourceException missingresourceexception) {
                return defaultPreferPublic;
            }
        }
        if(s == null)
            return defaultPreferPublic;
        else
            return s.equalsIgnoreCase("public");
    }

    public static boolean staticCatalog() {
        String s = System.getProperty(pStatic);
        if(s == null) {
            if(resources == null)
                readProperties();
            if(resources == null)
                return defaultStaticCatalog;
            try {
                s = resources.getString("static-catalog");
            }
            catch(MissingResourceException missingresourceexception) {
                return defaultStaticCatalog;
            }
        }
        if(s == null)
            return defaultStaticCatalog;
        else
            return s.equalsIgnoreCase("true") || s.equalsIgnoreCase("yes") || s.equalsIgnoreCase("1");
    }

    public static boolean allowOasisXMLCatalogPI() {
        String s = System.getProperty(pAllowPI);
        if(s == null) {
            if(resources == null)
                readProperties();
            if(resources == null)
                return defaultOasisXMLCatalogPI;
            try {
                s = resources.getString("allow-oasis-xml-catalog-pi");
            }
            catch(MissingResourceException missingresourceexception) {
                return defaultOasisXMLCatalogPI;
            }
        }
        if(s == null)
            return defaultOasisXMLCatalogPI;
        else
            return s.equalsIgnoreCase("true") || s.equalsIgnoreCase("yes") || s.equalsIgnoreCase("1");
    }

    public static String catalogClassName() {
        String s = System.getProperty(pClassname);
        if(s == null) {
            if(resources == null)
                readProperties();
            if(resources == null)
                return null;
            try {
                return resources.getString("catalog-class-name");
            }
            catch(MissingResourceException missingresourceexception) {
                return null;
            }
        } else {
            return s;
        }
    }

    static  {
        pFiles = "xml.catalog.files";
        pIgnoreMissing = "xml.catalog.ignoreMissing";
        ignoreMissingProperties =false;// System.getProperty(pIgnoreMissing) != null || System.getProperty(pFiles) != null;
    }
}
