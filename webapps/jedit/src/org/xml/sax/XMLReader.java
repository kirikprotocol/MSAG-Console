// XMLReader.java - read an XML document.
// Written by David Megginson, sax@megginson.com
// NO WARRANTY!  This class is in the Public Domain.

// $Id$

package org.xml.sax;

import java.io.IOException;


/**
 * Interface for reading an XML document using callbacks.
 *
 * <blockquote>
 * <em>This module, both source code and documentation, is in the
 * Public Domain, and comes with <strong>NO WARRANTY</strong>.</em>
 * </blockquote>
 *
 * <p><strong>Note:</strong> despite its name, this interface does 
 * <em>not</em> extend the standard Java {@link java.io.Reader Reader} 
 * interface, because reading XML is a fundamentally different activity 
 * than reading character data.</p>
 *
 * <p>XMLReader is the interface that an XML parser's SAX2 driver must
 * implement.  This interface allows an application to set and
 * query features and properties in the parser, to register
 * event handlers for document processing, and to initiate
 * a document parse.</p>
 *
 * <p>All SAX interfaces are assumed to be synchronous: the
 * {@link #parse parse} methods must not return until parsing
 * is complete, and readers must wait for an event-handler callback
 * to return before reporting the next event.</p>
 *
 * <p>This interface replaces the (now deprecated) SAX 1.0 {@link
 * org.xml.sax.Parser Parser} interface.  The XMLReader interface
 * contains two important enhancements over the old Parser
 * interface:</p>
 *
 * <ol>
 * <li>it adds a standard way to query and set features and 
 *  properties; and</li>
 * <li>it adds Namespace support, which is required for many
 *  higher-level XML standards.</li>
 * </ol>
 *
 * <p>There are adapters available to convert a SAX1 Parser to
 * a SAX2 XMLReader and vice-versa.</p>
 *
 * @since SAX 2.0
 * @author David Megginson, 
 *         <a href="mailto:sax@megginson.com">sax@megginson.com</a>
 * @version 2.0
 * @see org.xml.sax.XMLFilter
 * @see org.xml.sax.helpers.ParserAdapter
 * @see org.xml.sax.helpers.XMLReaderAdapter 
 */
public interface XMLReader
{


    ////////////////////////////////////////////////////////////////////
    // Configuration.
    ////////////////////////////////////////////////////////////////////


    /**
     * Look up the value of a feature.
     *
     * <p>The feature name is any fully-qualified URI.  It is
     * possible for an XMLReader to recognize a feature name but
     * to be unable to return its value; this is especially true
     * in the case of an adapter for a SAX1 Parser, which has
     * no way of knowing whether the underlying parser is
     * performing validation or expanding external entities.</p>
     *
     * <p>All XMLReaders are required to recognize the
     * http://xml.org/sax/features/namespaces and the
     * http://xml.org/sax/features/namespace-prefixes feature names.</p>
     *
     * <p>Some feature values may be available only in specific
     * contexts, such as before, during, or after a parse.</p>
     *
     * <p>Typical usage is something like this:</p>
     *
     * <pre>
     * XMLReader r = new MySAXDriver();
     *
     *                         // try to activate validation
     * try {
     *   r.setFeature("http://xml.org/sax/features/validation", true);
     * } catch (SAXException e) {
     *   System.err.println("Cannot activate validation."); 
     * }
     *
     *                         // register event handlers
     * r.setContentHandler(new MyContentHandler());
     * r.setErrorHandler(new MyErrorHandler());
     *
     *                         // parse the first document
     * try {
     *   r.parse("http://www.foo.com/mydoc.xml");
     * } catch (IOException e) {
     *   System.err.println("I/O exception reading XML document");
     * } catch (SAXException e) {
     *   System.err.println("XML exception reading document.");
     * }
     * </pre>
     *
     * <p>Implementors are free (and encouraged) to invent their own features,
     * using names built on their own URIs.</p>
     *
     * @param name The feature name, which is a fully-qualified URI.
     * @return The current state of the feature (true or false).
     * @exception org.xml.sax.SAXNotRecognizedException When the
     *            XMLReader does not recognize the feature name.
     * @exception org.xml.sax.SAXNotSupportedException When the
     *            XMLReader recognizes the feature name but 
     *            cannot determine its value at this time.
     * @see #setFeature
     */
    public boolean getFeature (String name)
        throws SAXNotRecognizedException, SAXNotSupportedException;


    /**
     * Set the state of a feature.
     *
     * <p>The feature name is any fully-qualified URI.  It is
     * possible for an XMLReader to recognize a feature name but
     * to be unable to set its value; this is especially true
     * in the case of an adapter for a SAX1 {@link org.xml.sax.Parser Parser},
     * which has no way of affecting whether the underlying parser is
     * validating, for example.</p>
     *
     * <p>All XMLReaders are required to support setting
     * http://xml.org/sax/features/namespaces to true and
     * http://xml.org/sax/features/namespace-prefixes to false.</p>
     *
     * <p>Some feature values may be immutable or mutable only 
     * in specific contexts, such as before, during, or after 
     * a parse.</p>
     *
     * @param name The feature name, which is a fully-qualified URI.
     * @param state The requested state of the feature (true or false).
     * @exception org.xml.sax.SAXNotRecognizedException When the
     *            XMLReader does not recognize the feature name.
     * @exception org.xml.sax.SAXNotSupportedException When the
     *            XMLReader recognizes the feature name but 
     *            cannot set the requested value.
     * @see #getFeature
     */
    public void setFeature (String name, boolean value)
 throws SAXNotRecognizedException, SAXNotSupportedException;


    /**
     * Look up the value of a property.
     *
     * <p>The property name is any fully-qualified URI.  It is
     * possible for an XMLReader to recognize a property name but
     * to be unable to return its state; this is especially true
     * in the case of an adapter for a SAX1 {@link org.xml.sax.Parser
     * Parser}.</p>
     *
     * <p>XMLReaders are not required to recognize any specific
     * property names, though an initial core set is documented for
     * SAX2.</p>
     *
     * <p>Some property values may be available only in specific
     * contexts, such as before, during, or after a parse.</p>
     *
     * <p>Implementors are free (and encouraged) to invent their own properties,
     * using names built on their own URIs.</p>
     *
     * @param name The property name, which is a fully-qualified URI.
     * @return The current value of the property.
     * @exception org.xml.sax.SAXNotRecognizedException When the
     *            XMLReader does not recognize the property name.
     * @exception org.xml.sax.SAXNotSupportedException When the
     *            XMLReader recognizes the property name but 
     *            cannot determine its value at this time.
     * @see #setProperty
     */
    public Object getProperty (String name)
 throws SAXNotRecognizedException, SAXNotSupportedException;


    /**
     * Set the value of a property.
     *
     * <p>The property name is any fully-qualified URI.  It is
     * possible for an XMLReader to recognize a property name but
     * to be unable to set its value; this is especially true
     * in the case of an adapter for a SAX1 {@link org.xml.sax.Parser
     * Parser}.</p>
     *
     * <p>XMLReaders are not required to recognize setting
     * any specific property names, though a core set is provided with 
     * SAX2.</p>
     *
     * <p>Some property values may be immutable or mutable only 
     * in specific contexts, such as before, during, or after 
     * a parse.</p>
     *
     * <p>This method is also the standard mechanism for setting
     * extended handlers.</p>
     *
     * @param name The property name, which is a fully-qualified URI.
     * @param state The requested value for the property.
     * @exception org.xml.sax.SAXNotRecognizedException When the
     *            XMLReader does not recognize the property name.
     * @exception org.xml.sax.SAXNotSupportedException When the
     *            XMLReader recognizes the property name but 
     *            cannot set the requested value.
     */
    public void setProperty (String name, Object value)
 throws SAXNotRecognizedException, SAXNotSupportedException;



    ////////////////////////////////////////////////////////////////////
    // Event handlers.
    ////////////////////////////////////////////////////////////////////


    /**
     * Allow an application to register an entity resolver.
     *
     * <p>If the application does not register an entity resolver,
     * the XMLReader will perform its own default resolution.</p>
     *
     * <p>Applications may register a new or different resolver in the
     * middle of a parse, and the SAX parser must begin using the new
     * resolver immediately.</p>
     *
     * @param resolver The entity resolver.
     * @exception java.lang.NullPointerException If the resolver 
     *            argument is null.
     * @see #getEntityResolver
     */
    public void setEntityResolver (EntityResolver resolver);


    /**
     * Return the current entity resolver.
     *
     * @return The current entity resolver, or null if none
     *         has been registered.
     * @see #setEntityResolver
     */
    public EntityResolver getEntityResolver ();


    /**
     * Allow an application to register a DTD event handler.
     *
     * <p>If the application does not register a DTD handler, all DTD
     * events reported by the SAX parser will be silently ignored.</p>
     *
     * <p>Applications may register a new or different handler in the
     * middle of a parse, and the SAX parser must begin using the new
     * handler immediately.</p>
     *
     * @param handler The DTD handler.
     * @exception java.lang.NullPointerException If the handler 
     *            argument is null.
     * @see #getDTDHandler
     */
    public void setDTDHandler (DTDHandler handler);


    /**
     * Return the current DTD handler.
     *
     * @return The current DTD handler, or null if none
     *         has been registered.
     * @see #setDTDHandler
     */
    public DTDHandler getDTDHandler ();


    /**
     * Allow an application to register a content event handler.
     *
     * <p>If the application does not register a content handler, all
     * content events reported by the SAX parser will be silently
     * ignored.</p>
     *
     * <p>Applications may register a new or different handler in the
     * middle of a parse, and the SAX parser must begin using the new
     * handler immediately.</p>
     *
     * @param handler The content handler.
     * @exception java.lang.NullPointerException If the handler 
     *            argument is null.
     * @see #getContentHandler
     */
    public void setContentHandler (ContentHandler handler);


    /**
     * Return the current content handler.
     *
     * @return The current content handler, or null if none
     *         has been registered.
     * @see #setContentHandler
     */
    public ContentHandler getContentHandler ();


    /**
     * Allow an application to register an error event handler.
     *
     * <p>If the application does not register an error handler, all
     * error events reported by the SAX parser will be silently
     * ignored; however, normal processing may not continue.  It is
     * highly recommended that all SAX applications implement an
     * error handler to avoid unexpected bugs.</p>
     *
     * <p>Applications may register a new or different handler in the
     * middle of a parse, and the SAX parser must begin using the new
     * handler immediately.</p>
     *
     * @param handler The error handler.
     * @exception java.lang.NullPointerException If the handler 
     *            argument is null.
     * @see #getErrorHandler
     */
    public void setErrorHandler (ErrorHandler handler);


    /**
     * Return the current error handler.
     *
     * @return The current error handler, or null if none
     *         has been registered.
     * @see #setErrorHandler
     */
    public ErrorHandler getErrorHandler ();



    ////////////////////////////////////////////////////////////////////
    // Parsing.
    ////////////////////////////////////////////////////////////////////

    /**
     * Parse an XML document.
     *
     * <p>The application can use this method to instruct the XML
     * reader to begin parsing an XML document from any valid input
     * source (a character stream, a byte stream, or a URI).</p>
     *
     * <p>Applications may not invoke this method while a parse is in
     * progress (they should create a new XMLReader instead for each
     * nested XML document).  Once a parse is complete, an
     * application may reuse the same XMLReader object, possibly with a
     * different input source.</p>
     *
     * <p>During the parse, the XMLReader will provide information
     * about the XML document through the registered event
     * handlers.</p>
     *
     * <p>This method is synchronous: it will not return until parsing
     * has ended.  If a client application wants to terminate 
     * parsing early, it should throw an exception.</p>
     *
     * @param source The input source for the top-level of the
     *        XML document.
     * @exception org.xml.sax.SAXException Any SAX exception, possibly
     *            wrapping another exception.
     * @exception java.io.IOException An IO exception from the parser,
     *            possibly from a byte stream or character stream
     *            supplied by the application.
     * @see org.xml.sax.InputSource
     * @see #parse(java.lang.String)
     * @see #setEntityResolver
     * @see #setDTDHandler
     * @see #setContentHandler
     * @see #setErrorHandler 
     */
    public void parse (InputSource input)
 throws IOException, SAXException;


    /**
     * Parse an XML document from a system identifier (URI).
     *
     * <p>This method is a shortcut for the common case of reading a
     * document from a system identifier.  It is the exact
     * equivalent of the following:</p>
     *
     * <pre>
     * parse(new InputSource(systemId));
     * </pre>
     *
     * <p>If the system identifier is a URL, it must be fully resolved
     * by the application before it is passed to the parser.</p>
     *
     * @param systemId The system identifier (URI).
     * @exception org.xml.sax.SAXException Any SAX exception, possibly
     *            wrapping another exception.
     * @exception java.io.IOException An IO exception from the parser,
     *            possibly from a byte stream or character stream
     *            supplied by the application.
     * @see #parse(org.xml.sax.InputSource)
     */
    public void parse (String systemId)
 throws IOException, SAXException;

}

// end of XMLReader.java
