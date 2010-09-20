// Decompiled by Jad v1.5.8e2. Copyright 2001 Pavel Kouznetsov.
// Jad home page: http://kpdus.tripod.com/jad.html
// Decompiler options: packfields(3) packimports(7) deadcode fieldsfirst splitstr(64) nonlb lnc radix(10) lradix(10) 

package org.apache.xml.resolver.readers;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;
import java.net.UnknownHostException;
import java.util.Hashtable;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;
import org.apache.xml.resolver.Catalog;
import org.apache.xml.resolver.CatalogException;
import org.apache.xml.resolver.helpers.Debug;
import org.xml.sax.*;

// Referenced classes of package org.apache.xml.resolver.readers:
//            SAXParserHandler, SAXCatalogParser, CatalogReader

public class SAXCatalogReader
    implements CatalogReader, ContentHandler, DocumentHandler {

    protected SAXParserFactory parserFactory;
    protected String parserClass;
    protected Hashtable namespaceMap;
    private SAXCatalogParser saxParser;
    private boolean abandonHope;
    private Catalog catalog;

    public void setParserFactory(SAXParserFactory saxparserfactory) {
        parserFactory = saxparserfactory;
    }

    public void setParserClass(String s) {
        parserClass = s;
    }

    public SAXParserFactory getParserFactory() {
        return parserFactory;
    }

    public String getParserClass() {
        return parserClass;
    }

    public void setCatalogParser(String s, String s1, String s2) {
        if(s == null)
            namespaceMap.put(s1, s2);
        else
            namespaceMap.put("{" + s + "}" + s1, s2);
    }

    public String getCatalogParser(String s, String s1) {
        if(s == null)
            return (String)namespaceMap.get(s1);
        else
            return (String)namespaceMap.get("{" + s + "}" + s1);
    }

    public SAXCatalogReader() {
        parserFactory = null;
        parserClass = null;
        namespaceMap = new Hashtable();
        saxParser = null;
        abandonHope = false;
        parserFactory = null;
        parserClass = null;
    }

    public SAXCatalogReader(SAXParserFactory saxparserfactory) {
        parserFactory = null;
        parserClass = null;
        namespaceMap = new Hashtable();
        saxParser = null;
        abandonHope = false;
        parserFactory = saxparserfactory;
    }

    public SAXCatalogReader(String s) {
        parserFactory = null;
        parserClass = null;
        namespaceMap = new Hashtable();
        saxParser = null;
        abandonHope = false;
        parserClass = s;
    }

    public void readCatalog(Catalog catalog1, String s) throws MalformedURLException, IOException, CatalogException {
        System.out.println("SAXCatalogReader.readCatalog() ... "+s);
        URL url = null;
        try {
            url = new URL(s);
            System.out.println("SAXCatalogReader 99 url: "+url);
        }
        catch(MalformedURLException malformedurlexception) {
            url = new URL("file:///" + s);
             System.out.println("SAXCatalogReader 103 url: "+url);
        }
        try {

            URLConnection urlconnection = url.openConnection();
            readCatalog(catalog1, urlconnection.getInputStream());
        }
        catch(FileNotFoundException filenotfoundexception) {
            Debug.message(1, "Failed to load catalog, file not found", url.toString());
        }
    }

    public void readCatalog(Catalog catalog1, InputStream inputstream) throws IOException, CatalogException {
        if(parserFactory == null && parserClass == null) {
            Debug.message(1, "Cannot read SAX catalog without a parser");
            throw new CatalogException(6);
        }
        catalog = catalog1;
        try {
            if(parserFactory != null) {
                SAXParser saxparser = parserFactory.newSAXParser();
                SAXParserHandler saxparserhandler = new SAXParserHandler();
                saxparserhandler.setContentHandler(this);
                saxparser.parse(new InputSource(inputstream), saxparserhandler);
            } else {
                Parser parser = (Parser)Class.forName(parserClass).newInstance();
                parser.setDocumentHandler(this);
                parser.parse(new InputSource(inputstream));
            }
        }
        catch(ClassNotFoundException classnotfoundexception) {
            throw new CatalogException(6);
        }
        catch(IllegalAccessException illegalaccessexception) {
            throw new CatalogException(6);
        }
        catch(InstantiationException instantiationexception) {
            throw new CatalogException(6);
        }
        catch(ParserConfigurationException parserconfigurationexception) {
            throw new CatalogException(5);
        }
        catch(SAXException saxexception) {
            Exception exception = saxexception.getException();
            UnknownHostException unknownhostexception = new UnknownHostException();
            FileNotFoundException filenotfoundexception = new FileNotFoundException();
            if(exception != null) {
                if(exception.getClass() == unknownhostexception.getClass())
                    throw new CatalogException(7, exception.toString());
                if(exception.getClass() == filenotfoundexception.getClass())
                    throw new CatalogException(7, exception.toString());
            }
            throw new CatalogException(saxexception);
        }
    }

    public void setDocumentLocator(Locator locator) {
        if(saxParser != null)
            saxParser.setDocumentLocator(locator);
    }

    public void startDocument() throws SAXException {
        saxParser = null;
        abandonHope = false;
    }

    public void endDocument() throws SAXException {
        if(saxParser != null)
            saxParser.endDocument();
    }

    public void startElement(String s, AttributeList attributelist) throws SAXException {
        if(abandonHope)
            return;
        if(saxParser == null) {
            String s1 = "";
            if(s.indexOf(':') > 0)
                s1 = s.substring(0, s.indexOf(':'));
            String s2 = s;
            if(s2.indexOf(':') > 0)
                s2 = s2.substring(s2.indexOf(':') + 1);
            String s3 = null;
            if(s1.equals(""))
                s3 = attributelist.getValue("xmlns");
            else
                s3 = attributelist.getValue("xmlns:" + s1);
            String s4 = getCatalogParser(s3, s2);
            if(s4 == null) {
                abandonHope = true;
                if(s3 == null)
                    Debug.message(2, "No Catalog parser for " + s);
                else
                    Debug.message(2, "No Catalog parser for {" + s3 + "}" + s);
                return;
            }
            try {
                saxParser = (SAXCatalogParser)Class.forName(s4).newInstance();
                saxParser.setCatalog(catalog);
                saxParser.startDocument();
                saxParser.startElement(s, attributelist);
            }
            catch(ClassNotFoundException classnotfoundexception) {
                saxParser = null;
                abandonHope = true;
                Debug.message(2, classnotfoundexception.toString());
            }
            catch(InstantiationException instantiationexception) {
                saxParser = null;
                abandonHope = true;
                Debug.message(2, instantiationexception.toString());
            }
            catch(IllegalAccessException illegalaccessexception) {
                saxParser = null;
                abandonHope = true;
                Debug.message(2, illegalaccessexception.toString());
            }
            catch(ClassCastException classcastexception) {
                saxParser = null;
                abandonHope = true;
                Debug.message(2, classcastexception.toString());
            }
        } else {
            saxParser.startElement(s, attributelist);
        }
    }

    public void startElement(String s, String s1, String s2, Attributes attributes) throws SAXException {
        if(abandonHope)
            return;
        if(saxParser == null) {
            String s3 = getCatalogParser(s, s1);
            if(s3 == null) {
                abandonHope = true;
                if(s == null)
                    Debug.message(2, "No Catalog parser for " + s1);
                else
                    Debug.message(2, "No Catalog parser for {" + s + "}" + s1);
                return;
            }
            try {
                saxParser = (SAXCatalogParser)Class.forName(s3).newInstance();
                saxParser.setCatalog(catalog);
                saxParser.startDocument();
                saxParser.startElement(s, s1, s2, attributes);
            }
            catch(ClassNotFoundException classnotfoundexception) {
                saxParser = null;
                abandonHope = true;
                Debug.message(2, classnotfoundexception.toString());
            }
            catch(InstantiationException instantiationexception) {
                saxParser = null;
                abandonHope = true;
                Debug.message(2, instantiationexception.toString());
            }
            catch(IllegalAccessException illegalaccessexception) {
                saxParser = null;
                abandonHope = true;
                Debug.message(2, illegalaccessexception.toString());
            }
            catch(ClassCastException classcastexception) {
                saxParser = null;
                abandonHope = true;
                Debug.message(2, classcastexception.toString());
            }
        } else {
            saxParser.startElement(s, s1, s2, attributes);
        }
    }

    public void endElement(String s) throws SAXException {
        if(saxParser != null)
            saxParser.endElement(s);
    }

    public void endElement(String s, String s1, String s2) throws SAXException {
        if(saxParser != null)
            saxParser.endElement(s, s1, s2);
    }

    public void characters(char ac[], int i, int j) throws SAXException {
        if(saxParser != null)
            saxParser.characters(ac, i, j);
    }

    public void ignorableWhitespace(char ac[], int i, int j) throws SAXException {
        if(saxParser != null)
            saxParser.ignorableWhitespace(ac, i, j);
    }

    public void processingInstruction(String s, String s1) throws SAXException {
        if(saxParser != null)
            saxParser.processingInstruction(s, s1);
    }

    public void startPrefixMapping(String s, String s1) throws SAXException {
        if(saxParser != null)
            saxParser.startPrefixMapping(s, s1);
    }

    public void endPrefixMapping(String s) throws SAXException {
        if(saxParser != null)
            saxParser.endPrefixMapping(s);
    }

    public void skippedEntity(String s) throws SAXException {
        if(saxParser != null)
            saxParser.skippedEntity(s);
    }
}
