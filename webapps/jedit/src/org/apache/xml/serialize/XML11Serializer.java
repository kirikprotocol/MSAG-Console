/*
 * The Apache Software License, Version 1.1
 *
 *
 * Copyright (c) 1999-2002 The Apache Software Foundation.  All rights
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
 * 4. The names "Xerces" and "Apache Software Foundation" must
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
 * originally based on software copyright (c) 1999, International
 * Business Machines, Inc., http://www.apache.org.  For more
 * information on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */



// Sep 14, 2000:
//  Fixed problem with namespace handling. Contributed by
//  David Blondeau <blondeau@intalio.com>
// Sep 14, 2000:
//  Fixed serializer to report IO exception directly, instead at
//  the end of document processing.
//  Reported by Patrick Higgins <phiggins@transzap.com>
// Aug 21, 2000:
//  Fixed bug in startDocument not calling prepare.
//  Reported by Mikael Staldal <d96-mst-ingen-reklam@d.kth.se>
// Aug 21, 2000:
//  Added ability to omit DOCTYPE declaration.


package org.apache.xml.serialize;


import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.io.OutputStream;
import java.io.Writer;
import java.util.Enumeration;

import org.w3c.dom.Node;
import org.w3c.dom.Attr;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;

import org.apache.xerces.dom3.DOMError;

import org.xml.sax.DocumentHandler;
import org.xml.sax.ContentHandler;
import org.xml.sax.AttributeList;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.AttributesImpl;

import org.apache.xerces.util.SymbolTable;
import org.apache.xerces.util.XMLSymbols;
import org.apache.xerces.util.NamespaceSupport;
import org.apache.xerces.util.XMLChar;
import org.apache.xerces.util.XML11Char;
import org.apache.xerces.impl.Constants;
import org.apache.xerces.dom.DOMMessageFormatter;

/**
 * Implements an XML serializer supporting both DOM and SAX pretty
 * serializing. For usage instructions see {@link Serializer}.
 * <p>
 * If an output stream is used, the encoding is taken from the
 * output format (defaults to <tt>UTF-8</tt>). If a writer is
 * used, make sure the writer uses the same encoding (if applies)
 * as specified in the output format.
 * <p>
 * The serializer supports both DOM and SAX. SAX serializing is done by firing
 * SAX events and using the serializer as a document handler. DOM serializing is done
 * by calling {@link #serialize(Document)} or by using DOM Level 3  
 * {@link org.w3c.dom.ls.DOMWriter} and
 * serializing with {@link org.w3c.dom.ls.DOMWriter#writeNode}, 
 * {@link org.w3c.dom.ls.DOMWriter#writeToString}.
 * <p>
 * If an I/O exception occurs while serializing, the serializer
 * will not throw an exception directly, but only throw it
 * at the end of serializing (either DOM or SAX's {@link
 * org.xml.sax.DocumentHandler#endDocument}.
 * <p>
 * For elements that are not specified as whitespace preserving,
 * the serializer will potentially break long text lines at space
 * boundaries, indent lines, and serialize elements on separate
 * lines. Line terminators will be regarded as spaces, and
 * spaces at beginning of line will be stripped.
 * @author <a href="mailto:arkin@intalio.com">Assaf Arkin</a>
 * @author <a href="mailto:rahul.srivastava@sun.com">Rahul Srivastava</a>
 * @author Elena Litani IBM
 * @version $Revision$ $Date$
 * @see Serializer
 */
public class XML11Serializer
extends XMLSerializer {

    //
    // constants
    //

    protected static final boolean DEBUG = false;

    // 
    // data
    //

    // 
    // DOM Level 3 implementation: variables intialized in DOMWriterImpl
    // 

    /** stores namespaces in scope */
    protected NamespaceSupport fNSBinder;

    /** stores all namespace bindings on the current element */
    protected NamespaceSupport fLocalNSBinder;

    /** symbol table for serialization */
    protected SymbolTable fSymbolTable;    

    // is node dom level 1 node?
    protected boolean fDOML1 = false;
    // counter for new prefix names
    protected int fNamespaceCounter = 1;
    protected final static String PREFIX = "NS";

    /**
     * Controls whether namespace fixup should be performed during
     * the serialization. 
     * NOTE: if this field is set to true the following 
     * fields need to be initialized: fNSBinder, fLocalNSBinder, fSymbolTable, 
     * XMLSymbols.EMPTY_STRING, fXmlSymbol, fXmlnsSymbol, fNamespaceCounter.
     */
    protected boolean fNamespaces = false;


    private boolean fPreserveSpace;


    /**
     * Constructs a new serializer. The serializer cannot be used without
     * calling {@link #setOutputCharStream} or {@link #setOutputByteStream}
     * first.
     */
    public XML11Serializer() {
        super( );
        _format.setVersion("1.1");
    }


    /**
     * Constructs a new serializer. The serializer cannot be used without
     * calling {@link #setOutputCharStream} or {@link #setOutputByteStream}
     * first.
     */
    public XML11Serializer( OutputFormat format ) {
        super( format );
        _format.setVersion("1.1");
    }


    /**
     * Constructs a new serializer that writes to the specified writer
     * using the specified output format. If <tt>format</tt> is null,
     * will use a default output format.
     *
     * @param writer The writer to use
     * @param format The output format to use, null for the default
     */
    public XML11Serializer( Writer writer, OutputFormat format ) {
        super( writer, format );
        _format.setVersion("1.1");
    }


    /**
     * Constructs a new serializer that writes to the specified output
     * stream using the specified output format. If <tt>format</tt>
     * is null, will use a default output format.
     *
     * @param output The output stream to use
     * @param format The output format to use, null for the default
     */
    public XML11Serializer( OutputStream output, OutputFormat format ) {
        super( output, format != null ? format : new OutputFormat( Method.XML, null, false ) );
        _format.setVersion("1.1");
    }

    //-----------------------------------------//
    // SAX content handler serializing methods //
    //-----------------------------------------//


    public void characters( char[] chars, int start, int length )
        throws SAXException
    {
        ElementState state;

        try {
            state = content();

            // Check if text should be print as CDATA section or unescaped
            // based on elements listed in the output format (the element
            // state) or whether we are inside a CDATA section or entity.

            if ( state.inCData || state.doCData ) {
                int          saveIndent;

                // Print a CDATA section. The text is not escaped, but ']]>'
                // appearing in the code must be identified and dealt with.
                // The contents of a text node is considered space preserving.
                if ( ! state.inCData ) {
                    _printer.printText( "<![CDATA[" );
                    state.inCData = true;
                }
                saveIndent = _printer.getNextIndent();
                _printer.setNextIndent( 0 );
                char ch;
                for ( int index = start ; index < length ; ++index ) {
                    ch = chars[index];
                    if ( ch == ']' && index + 2 < length &&
                        chars[ index + 1 ] == ']' && chars[ index + 2 ] == '>' ) {
                        _printer.printText("]]]]><![CDATA[>");
                        index +=2; 
                        continue;
                    }
                    if (!XML11Char.isXML11Valid(ch)) {
                        // check if it is surrogate
                        if (++index <length) {
                            surrogates(ch, chars[index]);
                        } 
                        else {
                            fatalError("The character '"+(char)ch+"' is an invalid XML character"); 
                        }
                        continue;
                    } else {
                        if ( _encodingInfo.isPrintable((char)ch) && XML11Char.isXML11ValidLiteral(ch)) {
                            _printer.printText((char)ch);
                        } else {
                            // The character is not printable -- split CDATA section
                            _printer.printText("]]>&#x");                        
                            _printer.printText(Integer.toHexString(ch));                        
                            _printer.printText(";<![CDATA[");
                        }
                    }
                }
                _printer.setNextIndent( saveIndent );

            } else {

                int saveIndent;

                if ( state.preserveSpace ) {
                    // If preserving space then hold of indentation so no
                    // excessive spaces are printed at line breaks, escape
                    // the text content without replacing spaces and print
                    // the text breaking only at line breaks.
                    saveIndent = _printer.getNextIndent();
                    _printer.setNextIndent( 0 );
                    printText( chars, start, length, true, state.unescaped );
                    _printer.setNextIndent( saveIndent );
                } else {
                    printText( chars, start, length, false, state.unescaped );
                }
            }
        } catch ( IOException except ) {
            throw new SAXException( except );
        }
    }


    //
    // overwrite printing functions to make sure serializer prints out valid XML
    //
    protected void printEscaped( String source ) throws IOException {
        int length = source.length();
        for ( int i = 0 ; i < length ; ++i ) {
            int ch = source.charAt(i);
            if (!XML11Char.isXML11Valid(ch)) {
                if (++i <length) {
                    surrogates(ch, source.charAt(i));
                } else {
                    fatalError("The character '"+(char)ch+"' is an invalid XML character"); 
                }
                continue;
            }
            printXMLChar(ch);
        }
    }

    protected final void printCDATAText( String text ) throws IOException {
        int length = text.length();
        char ch;

        for ( int index = 0 ; index <  length; ++index ) {
            ch = text.charAt( index );
            
            if ( ch ==']' && index + 2 < length && 
                 text.charAt (index + 1) == ']' && text.charAt (index + 2) == '>' ) { // check for ']]>'

                // DOM Level 3 Load and Save
                //
                if (fFeatures != null && fDOMErrorHandler != null) {
                    if (!getFeature(Constants.DOM_SPLIT_CDATA)) {
                        // issue fatal error
                        String msg = DOMMessageFormatter.formatMessage(DOMMessageFormatter.SERIALIZER_DOMAIN, "EndingCDATA", null);
                        modifyDOMError(msg, DOMError.SEVERITY_FATAL_ERROR, fCurrentNode);
                        boolean continueProcess = fDOMErrorHandler.handleError(fDOMError);
                        if (!continueProcess) {
                            throw new IOException();
                        }
                    } else {
                        // issue warning
                        String msg = DOMMessageFormatter.formatMessage(DOMMessageFormatter.SERIALIZER_DOMAIN, "SplittingCDATA", null);
                        modifyDOMError(msg, DOMError.SEVERITY_WARNING, fCurrentNode);
                        fDOMErrorHandler.handleError(fDOMError);
                    }
                }

                // split CDATA section
                _printer.printText("]]]]><![CDATA[>");
                index +=2; 
                continue;
            }
            
            if (!XML11Char.isXML11Valid(ch)) {
                // check if it is surrogate
                if (++index <length) {
                    surrogates(ch, text.charAt(index));
                } 
                else {
                    fatalError("The character '"+(char)ch+"' is an invalid XML character"); 
                }
                continue;
            } else {
                if ( _encodingInfo.isPrintable((char)ch) && XML11Char.isXML11ValidLiteral(ch)) {
                    _printer.printText((char)ch);
                } else {

                    // The character is not printable -- split CDATA section
                    _printer.printText("]]>&#x");                        
                    _printer.printText(Integer.toHexString(ch));                        
                    _printer.printText(";<![CDATA[");
                }
            }
        }
    }


    // note that this "int" should, in all cases, be a char.
    // REVISIT:  make it a char...
    protected final void printXMLChar( int ch ) throws IOException {

        if ( ch == '<') {
            _printer.printText("&lt;");
        } else if (ch == '&') {
            _printer.printText("&amp;");
        } else if ( ch == '"') {
            // REVISIT: for character data we should not convert this into 
            //          char reference
            _printer.printText("&quot;");
        } else if ( _encodingInfo.isPrintable((char)ch) && XML11Char.isXML11ValidLiteral(ch)) { 
            _printer.printText((char)ch);
        } else {
            // The character is not printable, print as character reference.
            _printer.printText( "&#x" );
            _printer.printText(Integer.toHexString(ch));
            _printer.printText( ';' );
        }
    }



    protected final void surrogates(int high, int low) throws IOException{
        if (XMLChar.isHighSurrogate(high)) {
            if (!XMLChar.isLowSurrogate(low)) {
                //Invalid XML
                fatalError("The character '"+(char)low+"' is an invalid XML character"); 
            }
            else {
                int supplemental = XMLChar.supplemental((char)high, (char)low);
                if (!XML11Char.isXML11Valid(supplemental)) {
                    //Invalid XML
                    fatalError("The character '"+(char)supplemental+"' is an invalid XML character"); 
                }
                else {
                    if (content().inCData ) {
                        _printer.printText("]]>&#x");                        
                        _printer.printText(Integer.toHexString(supplemental));                        
                        _printer.printText(";<![CDATA[");
                    }  
                    else {
                        _printer.printText("&#x");                        
                        _printer.printText(Integer.toHexString(supplemental));                        
                        _printer.printText(";");
                    }
                }
            }
        } else {
            fatalError("The character '"+(char)high+"' is an invalid XML character"); 
        }

    }


    protected void printText( String text, boolean preserveSpace, boolean unescaped )
    throws IOException {
        int index;
        char ch;
        int length = text.length();
        if ( preserveSpace ) {
            // Preserving spaces: the text must print exactly as it is,
            // without breaking when spaces appear in the text and without
            // consolidating spaces. If a line terminator is used, a line
            // break will occur.
            for ( index = 0 ; index < length ; ++index ) {
                ch = text.charAt( index );
                if (!XML11Char.isXML11Valid(ch)) {
                    // check if it is surrogate
                    if (++index <length) {
                        surrogates(ch, text.charAt(index));
                    } else {
                        fatalError("The character '"+(char)ch+"' is an invalid XML character"); 
                    }
                    continue;
                }
                if ( unescaped  && XML11Char.isXML11ValidLiteral(ch)) {
                    _printer.printText( ch );
                } else
                    printXMLChar( ch );
            }
        } else {
            // Not preserving spaces: print one part at a time, and
            // use spaces between parts to break them into different
            // lines. Spaces at beginning of line will be stripped
            // by printing mechanism. Line terminator is treated
            // no different than other text part.
            for ( index = 0 ; index < length ; ++index ) {
                ch = text.charAt( index );
                if (!XML11Char.isXML11Valid(ch)) {
                    // check if it is surrogate
                    if (++index <length) {
                        surrogates(ch, text.charAt(index));
                    } else {
                        fatalError("The character '"+(char)ch+"' is an invalid XML character"); 
                    }
                    continue;
                }
                if ( XML11Char.isXML11Space(ch))
                    _printer.printSpace();
                else if ( unescaped && XML11Char.isXML11ValidLiteral(ch) )
                    _printer.printText( ch );
                else
                    printXMLChar( ch );
            }
        }
    }



    protected void printText( char[] chars, int start, int length,
                              boolean preserveSpace, boolean unescaped ) throws IOException {
        int index;
        char ch;

        if ( preserveSpace ) {
            // Preserving spaces: the text must print exactly as it is,
            // without breaking when spaces appear in the text and without
            // consolidating spaces. If a line terminator is used, a line
            // break will occur.
            while ( length-- > 0 ) {
                ch = chars[ start ];
                ++start;
                if (!XML11Char.isXML11Valid(ch)) {
                    // check if it is surrogate
                    if (++start <length) {
                        surrogates(ch, chars[start]);
                    } else {
                        fatalError("The character '"+(char)ch+"' is an invalid XML character"); 
                    }
                    continue;
                }
                if ( unescaped && XML11Char.isXML11ValidLiteral(ch))
                    _printer.printText( ch );
                else
                    printXMLChar( ch );
            }
        } else {
            // Not preserving spaces: print one part at a time, and
            // use spaces between parts to break them into different
            // lines. Spaces at beginning of line will be stripped
            // by printing mechanism. Line terminator is treated
            // no different than other text part.
            while ( length-- > 0 ) {
                ch = chars[ start ];
                ++start;

                if (!XML11Char.isXML11Valid(ch)) {
                    // check if it is surrogate
                    if (++start <length) {
                        surrogates(ch, chars[start]);
                    } else {
                        fatalError("The character '"+(char)ch+"' is an invalid XML character"); 
                    }
                    continue;
                }
                if ( XML11Char.isXML11Space(ch))
                    _printer.printSpace();
                else if ( unescaped && XML11Char.isXML11ValidLiteral(ch))
                    _printer.printText( ch );
                else
                    printXMLChar( ch );
            }
        }
    }


    public boolean reset() {
        super.reset();
        return true;

    }

}




