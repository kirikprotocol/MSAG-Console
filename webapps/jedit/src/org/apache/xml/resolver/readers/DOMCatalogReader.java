// DOMCatalogReader.java - Read XML Catalog files

/*
 * The Apache Software License, Version 1.1
 *
 *
 * Copyright (c) 2001 The Apache Software Foundation.  All rights 
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:  
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Xalan" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written 
 *    permission, please contact apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation and was
 * originally based on software copyright (c) 2001, International
 * Business Machines Corporation., http://www.ibm.com.  For more
 * information on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

package org.apache.xml.resolver.readers;

import java.lang.Integer;
import java.util.Vector;
import java.util.Hashtable;
import java.util.Enumeration;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;
import java.net.MalformedURLException;

import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.ParserConfigurationException;

import org.apache.xml.resolver.Catalog;
import org.apache.xml.resolver.CatalogEntry;
import org.apache.xml.resolver.CatalogException;
import org.apache.xml.resolver.readers.CatalogReader;
import org.apache.xml.resolver.helpers.Debug;
import org.apache.xml.resolver.helpers.PublicId;
import org.apache.xml.resolver.helpers.Namespaces;

import org.xml.sax.SAXException;
import org.w3c.dom.*;

/**
 * <p>A DOM-based CatalogReader</p>
 *
 * <p>This class is used to read XML Catalogs using the DOM. This reader
 * has an advantage over the SAX-based reader that it can analyze the
 * DOM tree rather than simply a series of SAX events. It has the disadvantage
 * that it requires all of the code necessary to build and walk a DOM
 * tree.</p>
 *
 * <p>Since the choice of CatalogReaders (in the InputStream case) can only
 * be made on the basis of MIME type, the following problem occurs: only
 * one CatalogReader can exist for all XML mime types. In order to get
 * around this problem, the DOMCatalogReader relies on a set of external
 * CatalogParsers to actually build the catalog.</p>
 *
 * <p>The selection of CatalogParsers is made on the basis of the QName
 * of the root element of the document.</p>
 *
 * <p>This class requires the <a href="http://java.sun.com/aboutJava/communityprocess/final/jsr005/index.html">Java API for XML Parsing</a>.</p>
 *
 * @see Catalog
 * @see CatalogReader
 * @see SAXCatalogReader
 * @see TextCatalogReader
 * @see DOMCatalogParser
 *
 * @author Norman Walsh
 * <a href="mailto:Norman.Walsh@Sun.COM">Norman.Walsh@Sun.COM</a>
 *
 * @version 1.0
 */
public class DOMCatalogReader implements CatalogReader {
  /**
   * <p>Mapping table from QNames to CatalogParser classes.</p>
   *
   * <p>Each key in this hash table has the form "elementname"
   * or "{namespaceuri}elementname". The former is used if the
   * namespace URI is null.</p>
   */
  protected Hashtable namespaceMap = new Hashtable();

  /**
   * <p>Add a new parser to the reader.</p>
   *
   * <p>This method associates the specified parserClass with the
   * namespaceURI/rootElement names specified.</p>
   *
   * @param namespaceURI The namespace URI. <em>Not</em> the prefix.
   * @param rootElement The name of the root element.
   * @param parserClass The name of the parserClass to instantiate
   * for this kind of catalog.
   */
  public void setCatalogParser(String namespaceURI,
			       String rootElement,
			       String parserClass) {
    if (namespaceURI == null) {
      namespaceMap.put(rootElement, parserClass);
    } else {
      namespaceMap.put("{"+namespaceURI+"}"+rootElement, parserClass);
    }
  }

  /**
   * <p>Get the name of the parser class for a given catalog type.</p>
   *
   * <p>This method returns the parserClass associated with the
   * namespaceURI/rootElement names specified.</p>
   *
   * @param namespaceURI The namespace URI. <em>Not</em> the prefix.
   * @param rootElement The name of the root element.
   * @return The parser class.
   */
  public String getCatalogParser(String namespaceURI,
				 String rootElement) {
    if (namespaceURI == null) {
      return (String) namespaceMap.get(rootElement);
    } else {
      return (String) namespaceMap.get("{"+namespaceURI+"}"+rootElement);
    }
  }

  /**
   * <p>Null constructor; something for subclasses to call.</p>
   */
  public DOMCatalogReader() { }

  /**
   * <p>Read a catalog from an input stream</p>
   *
   * <p>This class reads a catalog from an input stream:</p>
   *
   * <ul>
   * <li>Based on the QName of the root element, it determines which
   * parser to instantiate for this catalog.</li>
   * <li>It constructs a DOM Document from the catalog and</li>
   * <li>For each child of the root node, it calls the parser's
   * parseCatalogEntry method. This method is expected to make
   * appropriate calls back into the catalog to add entries for the
   * entries in the catalog. It is free to do this in whatever manner
   * is appropriate (perhaps using just the node passed in, perhaps
   * wandering arbitrarily throughout the tree).</li>
   * </ul>
   *
   * @param catalog The catalog for which this reader is called.
   * @param is The input stream that is to be read.
   * @throws IOException if the URL cannot be read.
   * @throws UnknownCatalogFormatException if the catalog format is
   * not recognized.
   * @throws UnparseableCatalogException if the catalog cannot be parsed.
   * (For example, if it is supposed to be XML and isn't well-formed or
   * if the parser class cannot be instantiated.)
   */
  public void readCatalog(Catalog catalog, InputStream is)
    throws IOException, CatalogException {

    DocumentBuilderFactory factory = null;
    DocumentBuilder builder = null;

    factory = DocumentBuilderFactory.newInstance();
    factory.setNamespaceAware(false);
    factory.setValidating(false);
    try {
      builder = factory.newDocumentBuilder();
    } catch (ParserConfigurationException pce) {
      throw new CatalogException(CatalogException.UNPARSEABLE);
    }

    Document doc = null;

    try {
      doc = builder.parse(is);
    } catch (SAXException se) {
      throw new CatalogException(CatalogException.UNKNOWN_FORMAT);
    }

    Element root = doc.getDocumentElement();

    String namespaceURI = Namespaces.getNamespaceURI(root);
    String localName    = Namespaces.getLocalName(root);

    String domParserClass = getCatalogParser(namespaceURI,
					     localName);

    if (domParserClass == null) {
      if (namespaceURI == null) {
	Debug.message(1, "No Catalog parser for " + localName);
      } else {
	Debug.message(1, "No Catalog parser for "
		      + "{" + namespaceURI + "}"
		      + localName);
      }
      return;
    }

    DOMCatalogParser domParser = null;

    try {
      domParser = (DOMCatalogParser) Class.forName(domParserClass).newInstance();
    } catch (ClassNotFoundException cnfe) {
      Debug.message(1, "Cannot load XML Catalog Parser class", domParserClass);
      throw new CatalogException(CatalogException.UNPARSEABLE);
    } catch (InstantiationException ie) {
      Debug.message(1, "Cannot instantiate XML Catalog Parser class", domParserClass);
      throw new CatalogException(CatalogException.UNPARSEABLE);
    } catch (IllegalAccessException iae) {
      Debug.message(1, "Cannot access XML Catalog Parser class", domParserClass);
      throw new CatalogException(CatalogException.UNPARSEABLE);
    } catch (ClassCastException cce ) {
      Debug.message(1, "Cannot cast XML Catalog Parser class", domParserClass);
      throw new CatalogException(CatalogException.UNPARSEABLE);
    }

    Node node = root.getFirstChild();
    while (node != null) {
      domParser.parseCatalogEntry(catalog, node);
      node = node.getNextSibling();
    }
  }

  /**
   * <p>Read the catalog behind the specified URL.</p>
   *
   * @see #readCatalog(Catalog, InputStream)
   *
   * @param catalog The catalog for which we are reading.
   * @param fileUrl The URL of the document that should be read.
   *
   * @throws MalformedURLException if the specified URL cannot be
   * turned into a URL object.
   * @throws IOException if the URL cannot be read.
   * @throws UnknownCatalogFormatException if the catalog format is
   * not recognized.
   * @throws UnparseableCatalogException if the catalog cannot be parsed.
   * (For example, if it is supposed to be XML and isn't well-formed.)
   */
  public void readCatalog(Catalog catalog, String fileUrl)
    throws MalformedURLException, IOException, CatalogException {
    System.out.println("DOMCatalogReader.readCatalog() ... "+fileUrl);
    URL url = new URL(fileUrl);
    URLConnection urlCon = url.openConnection();
    readCatalog(catalog, urlCon.getInputStream());
  }
}
