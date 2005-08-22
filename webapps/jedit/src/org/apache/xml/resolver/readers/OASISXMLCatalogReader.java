// Decompiled by Jad v1.5.8e2. Copyright 2001 Pavel Kouznetsov.
// Jad home page: http://kpdus.tripod.com/jad.html
// Decompiler options: packfields(3) packimports(7) deadcode fieldsfirst splitstr(64) nonlb lnc radix(10) lradix(10) 

package org.apache.xml.resolver.readers;

import java.util.Enumeration;
import java.util.Stack;
import java.util.Vector;
import org.apache.xml.resolver.Catalog;
import org.apache.xml.resolver.CatalogEntry;
import org.apache.xml.resolver.CatalogException;
import org.apache.xml.resolver.helpers.Debug;
import org.apache.xml.resolver.helpers.PublicId;
import org.xml.sax.Attributes;
import org.xml.sax.Locator;
import org.xml.sax.SAXException;

// Referenced classes of package org.apache.xml.resolver.readers:
//            SAXCatalogReader, SAXCatalogParser

public class OASISXMLCatalogReader extends SAXCatalogReader
    implements SAXCatalogParser {

    protected Catalog catalog;
    public static final boolean DEBUG=false;
    public static final String namespaceName = "urn:oasis:names:tc:entity:xmlns:xml:catalog";
    public static final String tr9401NamespaceName = "urn:oasis:names:tc:entity:xmlns:tr9401:catalog";
    protected Stack baseURIStack, overrideStack, namespaceStack;

    public OASISXMLCatalogReader() {
        catalog = null;
        baseURIStack = new Stack();
        overrideStack = new Stack();
        namespaceStack = new Stack();
    }

    public void setCatalog(Catalog catalog1) {
        catalog = catalog1;
    }

    public Catalog getCatalog() {
        return catalog;
    }

    protected boolean inExtensionNamespace() {
        boolean flag = false;
        for(Enumeration enumeration = namespaceStack.elements(); !flag && enumeration.hasMoreElements();) {
            String s = (String)enumeration.nextElement();
            if(s == null)
                flag = true;
            else
                flag = !s.equals("urn:oasis:names:tc:entity:xmlns:tr9401:catalog") && !s.equals("urn:oasis:names:tc:entity:xmlns:xml:catalog");
        }

        return flag;
    }

    public void setDocumentLocator(Locator locator) {
    }

    public void startDocument() throws SAXException {
        baseURIStack.push(catalog.getCurrentBase());
        overrideStack.push(catalog.getDefaultOverride());
    }

    public void endDocument() throws SAXException {
    }

    public void startElement(String s, String s1, String s2, Attributes attributes) throws SAXException {
        int i = -1;
        Vector vector = new Vector();
        namespaceStack.push(s);
        boolean flag = inExtensionNamespace();
        if(s != null && "urn:oasis:names:tc:entity:xmlns:xml:catalog".equals(s) && !flag) {
            if(attributes.getValue("xml:base") != null) {
                String s3 = attributes.getValue("xml:base");
                i = Catalog.BASE;
                vector.add(s3);
                baseURIStack.push(s3);
               // Debug.message(4, "xml:base", s3);
                try {
                    CatalogEntry catalogentry2 = new CatalogEntry(i, vector);
                    catalog.addEntry(catalogentry2);
                }
                catch(CatalogException catalogexception2) {
                    if(catalogexception2.getExceptionType() == 3)
                     if (DEBUG) System.out.println("Invalid catalog entry type "+s1);
                      // Debug.message(1, "Invalid catalog entry type", s1);
                    else
                    if(catalogexception2.getExceptionType() == 2)
                     if (DEBUG) System.out.println("Invalid catalog entry (base) "+s1);
                       // Debug.message(1, "Invalid catalog entry (base)", s1);
                }
                i = -1;
                vector = new Vector();
            } else {
                baseURIStack.push(baseURIStack.peek());
            }
            if((s1.equals("catalog") || s1.equals("group")) && attributes.getValue("prefer") != null) {
                String s4 = attributes.getValue("prefer");
                if(s4.equals("public"))
                    s4 = "yes";
                else
                if(s4.equals("system")) {
                    s4 = "no";
                } else {
                    if (DEBUG) System.out.println("Invalid prefer: must be 'system' or 'public'"+ s1);
                    s4 = catalog.getDefaultOverride();
                }
                i = Catalog.OVERRIDE;
                vector.add(s4);
                overrideStack.push(s4);
                if (DEBUG) System.out.println("override"+ s4);
                try {
                    CatalogEntry catalogentry3 = new CatalogEntry(i, vector);
                    catalog.addEntry(catalogentry3);
                }
                catch(CatalogException catalogexception3) {
                    if(catalogexception3.getExceptionType() == 3)
                       if (DEBUG)  System.out.println( "Invalid catalog entry type"+ s1);
                    else
                    if(catalogexception3.getExceptionType() == 2)
                      if (DEBUG)  System.out.println( "Invalid catalog entry (override)"+ s1);
                }
                i = -1;
                vector = new Vector();
            } else {
                overrideStack.push(overrideStack.peek());
            }
            if(s1.equals("delegatePublic")) {
                if(checkAttributes(attributes, "publicIdStartString", "catalog")) {
                    i = Catalog.DELEGATE_PUBLIC;
                    vector.add(attributes.getValue("publicIdStartString"));
                    vector.add(attributes.getValue("catalog"));
                 if (DEBUG)   System.out.println( "delegatePublic "+ PublicId.normalize(attributes.getValue("publicIdStartString"))+attributes.getValue("catalog"));
                }
            } else
            if(s1.equals("delegateSystem")) {
                if(checkAttributes(attributes, "systemIdStartString", "catalog")) {
                    i = Catalog.DELEGATE_SYSTEM;
                    vector.add(attributes.getValue("systemIdStartString"));
                    vector.add(attributes.getValue("catalog"));
                  if (DEBUG)  System.out.println( "delegateSystem "+ attributes.getValue("systemIdStartString") + attributes.getValue("catalog"));
                }
            } else
            if(s1.equals("delegateURI")) {
                if(checkAttributes(attributes, "uriStartString", "catalog")) {
                    i = Catalog.DELEGATE_URI;
                    vector.add(attributes.getValue("uriStartString"));
                    vector.add(attributes.getValue("catalog"));
                  if (DEBUG)  System.out.println( "delegateURI "+attributes.getValue("uriStartString")+ attributes.getValue("catalog"));
                }
            } else
            if(s1.equals("rewriteSystem")) {
                if(checkAttributes(attributes, "systemIdStartString", "rewritePrefix")) {
                    i = Catalog.REWRITE_SYSTEM;
                    vector.add(attributes.getValue("systemIdStartString"));
                    vector.add(attributes.getValue("rewritePrefix"));
                 if (DEBUG)   System.out.println( "rewriteSystem "+ attributes.getValue("systemIdStartString")+ attributes.getValue("rewritePrefix"));
                }
            } else
            if(s1.equals("rewriteURI")) {
                if(checkAttributes(attributes, "uriStartString", "rewritePrefix")) {
                    i = Catalog.REWRITE_URI;
                    vector.add(attributes.getValue("uriStartString"));
                    vector.add(attributes.getValue("rewritePrefix"));
                  if (DEBUG)   System.out.println( "rewriteURI "+ attributes.getValue("uriStartString")+ attributes.getValue("rewritePrefix"));
                }
            } else
            if(s1.equals("nextCatalog")) {
                if(checkAttributes(attributes, "catalog")) {
                    i = Catalog.CATALOG;
                    vector.add(attributes.getValue("catalog"));
                  if (DEBUG)   System.out.println( "nextCatalog "+ attributes.getValue("catalog"));
                }
            } else
            if(s1.equals("public")) {
                if(checkAttributes(attributes, "publicId", "uri")) {
                    i = Catalog.PUBLIC;
                    vector.add(attributes.getValue("publicId"));
                    vector.add(attributes.getValue("uri"));
                  if (DEBUG)   System.out.println( "public "+ PublicId.normalize(attributes.getValue("publicId"))+ attributes.getValue("uri"));
                }
            } else
            if(s1.equals("system")) {
                if(checkAttributes(attributes, "systemId", "uri")) {
                    i = Catalog.SYSTEM;
                    vector.add(attributes.getValue("systemId"));
                    vector.add(attributes.getValue("uri"));
                 if (DEBUG)   System.out.println( "system "+ attributes.getValue("systemId")+ attributes.getValue("uri"));
                }
            } else
            if(s1.equals("uri")) {
                if(checkAttributes(attributes, "name", "uri")) {
                    i = Catalog.URI;
                    vector.add(attributes.getValue("name"));
                    vector.add(attributes.getValue("uri"));
                   if (DEBUG)  System.out.println( "uri "+ attributes.getValue("name")+ attributes.getValue("uri"));
                }
            } else
            if(!s1.equals("catalog") && !s1.equals("group"))
              if (DEBUG)   System.out.println( "Invalid catalog entry type "+ s1);
            if(i >= 0)
                try {
                    CatalogEntry catalogentry = new CatalogEntry(i, vector);
                    catalog.addEntry(catalogentry);
                }
                catch(CatalogException catalogexception) {
                    if(catalogexception.getExceptionType() == 3)
                     if (DEBUG)   System.out.println( "Invalid catalog entry type "+ s1);
                    else
                    if(catalogexception.getExceptionType() == 2)
                      if (DEBUG)   System.out.println( "Invalid catalog entry "+ s1);
                }
        }
        if(s != null && "urn:oasis:names:tc:entity:xmlns:tr9401:catalog".equals(s) && !flag) {
            if(attributes.getValue("xml:base") != null) {
                String s5 = attributes.getValue("xml:base");
                i = Catalog.BASE;
                vector.add(s5);
                baseURIStack.push(s5);
               System.out.println( "xml:base "+ s5);
                try {
                    CatalogEntry catalogentry4 = new CatalogEntry(i, vector);
                    catalog.addEntry(catalogentry4);
                }
                catch(CatalogException catalogexception4) {
                    if(catalogexception4.getExceptionType() == 3)
                     if (DEBUG)   System.out.println("Invalid catalog entry type "+ s1);
                    else
                    if(catalogexception4.getExceptionType() == 2)
                      if (DEBUG)   System.out.println( "Invalid catalog entry (base) " + s1);
                }
                i = -1;
                vector = new Vector();
            } else {
                baseURIStack.push(baseURIStack.peek());
            }
            if(s1.equals("doctype")) {
                OASISXMLCatalogReader _tmp = this;
                i = Catalog.DOCTYPE;
                vector.add(attributes.getValue("name"));
                vector.add(attributes.getValue("uri"));
            } else
            if(s1.equals("document")) {
                OASISXMLCatalogReader _tmp1 = this;
                i = Catalog.DOCUMENT;
                vector.add(attributes.getValue("uri"));
            } else
            if(s1.equals("dtddecl")) {
                OASISXMLCatalogReader _tmp2 = this;
                i = Catalog.DTDDECL;
                vector.add(attributes.getValue("publicId"));
                vector.add(attributes.getValue("uri"));
            } else
            if(s1.equals("entity")) {
                i = Catalog.ENTITY;
                vector.add(attributes.getValue("name"));
                vector.add(attributes.getValue("uri"));
            } else
            if(s1.equals("linktype")) {
                i = Catalog.LINKTYPE;
                vector.add(attributes.getValue("name"));
                vector.add(attributes.getValue("uri"));
            } else
            if(s1.equals("notation")) {
                i = Catalog.NOTATION;
                vector.add(attributes.getValue("name"));
                vector.add(attributes.getValue("uri"));
            } else
            if(s1.equals("sgmldecl")) {
                i = Catalog.SGMLDECL;
                vector.add(attributes.getValue("uri"));
            } else {
              if (DEBUG)  System.out.println( "Invalid catalog entry type "+ s1);
            }
            if(i >= 0)
                try {
                    CatalogEntry catalogentry1 = new CatalogEntry(i, vector);
                    catalog.addEntry(catalogentry1);
                }
                catch(CatalogException catalogexception1) {
                    if(catalogexception1.getExceptionType() == 3)
                      if (DEBUG)  System.out.println( "Invalid catalog entry type "+ s1);
                    else
                    if(catalogexception1.getExceptionType() == 2)
                      if (DEBUG)  System.out.println( "Invalid catalog entry "+ s1);
                }
        }
    }

    public boolean checkAttributes(Attributes attributes, String s) {
        if(attributes.getValue(s) == null) {
          if (DEBUG)  System.out.println( "Error: required attribute " + s + " missing.");
            return false;
        } else {
            return true;
        }
    }

    public boolean checkAttributes(Attributes attributes, String s, String s1) {
        return checkAttributes(attributes, s) && checkAttributes(attributes, s1);
    }

    public void endElement(String s, String s1, String s2) throws SAXException {
        byte byte0 = -1;
        Vector vector = new Vector();
        boolean flag = inExtensionNamespace();
        if(s != null && !flag && ("urn:oasis:names:tc:entity:xmlns:xml:catalog".equals(s) || "urn:oasis:names:tc:entity:xmlns:tr9401:catalog".equals(s))) {
            String s3 = (String)baseURIStack.pop();
            String s5 = (String)baseURIStack.peek();
            if(!s5.equals(s3)) {
                OASISXMLCatalogReader _tmp = this;
                int i = Catalog.BASE;
                vector.add(s5);
              if (DEBUG)  System.out.println( "(reset) xml:base "+ s5);
                try {
                    CatalogEntry catalogentry = new CatalogEntry(i, vector);
                    catalog.addEntry(catalogentry);
                }
                catch(CatalogException catalogexception) {
                    if(catalogexception.getExceptionType() == 3)
                      if (DEBUG)   System.out.println( "Invalid catalog entry type "+ s1);
                    else
                    if(catalogexception.getExceptionType() == 2)
                      if (DEBUG)   System.out.println( "Invalid catalog entry (rbase) "+ s1);
                }
            }
        }
        if(s != null && "urn:oasis:names:tc:entity:xmlns:xml:catalog".equals(s) && !flag && (s1.equals("catalog") || s1.equals("group"))) {
            String s4 = (String)overrideStack.pop();
            String s6 = (String)overrideStack.peek();
            if(!s6.equals(s4)) {
                OASISXMLCatalogReader _tmp1 = this;
                int j = Catalog.OVERRIDE;
                vector.add(s6);
                overrideStack.push(s6);
               if (DEBUG)  System.out.println( "(reset) override "+ s6);
                try {
                    CatalogEntry catalogentry1 = new CatalogEntry(j, vector);
                    catalog.addEntry(catalogentry1);
                }
                catch(CatalogException catalogexception1) {
                    if(catalogexception1.getExceptionType() == 3)
                     if (DEBUG)   System.out.println( "Invalid catalog entry type "+ s1);
                    else
                    if(catalogexception1.getExceptionType() == 2)
                      if (DEBUG)   System.out.println( "Invalid catalog entry (roverride) "+ s1);
                }
            }
        }
        namespaceStack.pop();
    }

    public void characters(char ac[], int i, int j) throws SAXException {
    }

    public void ignorableWhitespace(char ac[], int i, int j) throws SAXException {
    }

    public void processingInstruction(String s, String s1) throws SAXException {
    }

    public void skippedEntity(String s) throws SAXException {
    }

    public void startPrefixMapping(String s, String s1) throws SAXException {
    }

    public void endPrefixMapping(String s) throws SAXException {
    }
}
