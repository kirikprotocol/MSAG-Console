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

package org.apache.xerces.dom;

import java.io.Serializable;
import java.util.Enumeration;
import java.util.Hashtable;

import org.apache.xerces.dom3.DOMConfiguration;
import org.apache.xerces.dom3.UserDataHandler;
import org.apache.xerces.util.XMLChar;
import org.apache.xerces.xni.NamespaceContext;
import org.w3c.dom.Attr;
import org.w3c.dom.CDATASection;
import org.w3c.dom.Comment;
import org.w3c.dom.DOMException;
import org.w3c.dom.DOMImplementation;
import org.w3c.dom.Document;
import org.w3c.dom.DocumentFragment;
import org.w3c.dom.DocumentType;
import org.w3c.dom.Element;
import org.w3c.dom.Entity;
import org.w3c.dom.EntityReference;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.w3c.dom.Notation;
import org.w3c.dom.ProcessingInstruction;
import org.w3c.dom.Text;
import org.w3c.dom.events.Event;
import org.w3c.dom.events.EventListener;
import org.w3c.dom.ls.DOMImplementationLS;
import org.w3c.dom.ls.DOMWriter;
import org.w3c.dom.ls.DocumentLS;

/**
 * The Document interface represents the entire HTML or XML document.
 * Conceptually, it is the root of the document tree, and provides the
 * primary access to the document's data.
 * <P>
 * Since elements, text nodes, comments, processing instructions,
 * etc. cannot exist outside the context of a Document, the Document
 * interface also contains the factory methods needed to create these
 * objects. The Node objects created have a ownerDocument attribute
 * which associates them with the Document within whose context they
 * were created.
 * <p>
 * The CoreDocumentImpl class only implements the DOM Core. Additional modules
 * are supported by the more complete DocumentImpl subclass.
 * <p>
 * <b>Note:</b> When any node in the document is serialized, the
 * entire document is serialized along with it.
 *
 * @author Arnaud  Le Hors, IBM
 * @author Joe Kesselman, IBM
 * @author Andy Clark, IBM
 * @author Ralf Pfeiffer, IBM
 * @version $Id$
 * @since  PR-DOM-Level-1-19980818.
 */
public class CoreDocumentImpl
    extends ParentNode implements Document, DocumentLS {

    //
    // Constants
    //
    
    /** Serialization version. */
    static final long serialVersionUID = 0;
    
    //
    // Data
    //

    // document information

    /** Document type. */
    protected DocumentTypeImpl docType;

    /** Document element. */
    protected ElementImpl docElement;

    /** NodeListCache free list */
    transient NodeListCache fFreeNLCache;

    /**Experimental DOM Level 3 feature: Document encoding */
    protected String encoding;

    /**Experimental DOM Level 3 feature: Document actualEncoding */
    protected String actualEncoding;

    /**Experimental DOM Level 3 feature: Document version */
    protected String version;
                                                                           
    /**Experimental DOM Level 3 feature: Document standalone */
    protected boolean standalone;

    /**Experimental DOM Level 3 feature: documentURI */
    protected String fDocumentURI;

    /** Table for user data attached to this document nodes. */
    protected Hashtable userData;

    /** Identifiers. */
    protected Hashtable identifiers;
    
    // DOM Level 3: normalizeDocument
    transient DOMNormalizer domNormalizer = null;
    transient DOMConfigurationImpl fConfiguration= null;    

    /** Table for quick check of child insertion. */
    private final static int[] kidOK;

    /**
     * Number of alterations made to this document since its creation.
     * Serves as a "dirty bit" so that live objects such as NodeList can
     * recognize when an alteration has been made and discard its cached
     * state information.
     * <p>
     * Any method that alters the tree structure MUST cause or be
     * accompanied by a call to changed(), to inform it that any outstanding
     * NodeLists may have to be updated.
     * <p>
     * (Required because NodeList is simultaneously "live" and integer-
     * indexed -- a bad decision in the DOM's design.)
     * <p>
     * Note that changes which do not affect the tree's structure -- changing
     * the node's name, for example -- do _not_ have to call changed().
     * <p>
     * Alternative implementation would be to use a cryptographic
     * Digest value rather than a count. This would have the advantage that
     * "harmless" changes (those producing equal() trees) would not force
     * NodeList to resynchronize. Disadvantage is that it's slightly more prone
     * to "false negatives", though that's the difference between "wildly
     * unlikely" and "absurdly unlikely". IF we start maintaining digests,
     * we should consider taking advantage of them.
     *
     * Note: This used to be done a node basis, so that we knew what
     * subtree changed. But since only DeepNodeList really use this today,
     * the gain appears to be really small compared to the cost of having
     * an int on every (parent) node plus having to walk up the tree all the
     * way to the root to mark the branch as changed everytime a node is
     * changed.
     * So we now have a single counter global to the document. It means that
     * some objects may flush their cache more often than necessary, but this
     * makes nodes smaller and only the document needs to be marked as changed.
     */
    protected int changes = 0;

    // experimental

    /** Allow grammar access. */
    protected boolean allowGrammarAccess;

    /** Bypass error checking. */
    protected boolean errorChecking = true;

    //
    // Static initialization
    //

    static {

        kidOK = new int[13];

        kidOK[DOCUMENT_NODE] =
            1 << ELEMENT_NODE | 1 << PROCESSING_INSTRUCTION_NODE |
            1 << COMMENT_NODE | 1 << DOCUMENT_TYPE_NODE;

        kidOK[DOCUMENT_FRAGMENT_NODE] =
        kidOK[ENTITY_NODE] =
        kidOK[ENTITY_REFERENCE_NODE] =
        kidOK[ELEMENT_NODE] =
            1 << ELEMENT_NODE | 1 << PROCESSING_INSTRUCTION_NODE |
            1 << COMMENT_NODE | 1 << TEXT_NODE |
            1 << CDATA_SECTION_NODE | 1 << ENTITY_REFERENCE_NODE ;


        kidOK[ATTRIBUTE_NODE] =
            1 << TEXT_NODE | 1 << ENTITY_REFERENCE_NODE;

        kidOK[DOCUMENT_TYPE_NODE] =
        kidOK[PROCESSING_INSTRUCTION_NODE] =
        kidOK[COMMENT_NODE] =
        kidOK[TEXT_NODE] =
        kidOK[CDATA_SECTION_NODE] =
        kidOK[NOTATION_NODE] =
            0;

    } // static

    //
    // Constructors
    //

    /**
     * NON-DOM: Actually creating a Document is outside the DOM's spec,
     * since it has to operate in terms of a particular implementation.
     */
    public CoreDocumentImpl() {
        this(false);
    }

    /** Constructor. */
    public CoreDocumentImpl(boolean grammarAccess) {
        super(null);
        ownerDocument = this;
        allowGrammarAccess = grammarAccess;       
    }

    /**
     * For DOM2 support.
     * The createDocument factory method is in DOMImplementation.
     */
    public CoreDocumentImpl(DocumentType doctype)
    {
        this(doctype, false);
    }

    /** For DOM2 support. */
    public CoreDocumentImpl(DocumentType doctype, boolean grammarAccess) {
        this(grammarAccess);
        if (doctype != null) {
            DocumentTypeImpl doctypeImpl;
            try {
                doctypeImpl = (DocumentTypeImpl) doctype;
            } catch (ClassCastException e) {
                String msg = DOMMessageFormatter.formatMessage(DOMMessageFormatter.DOM_DOMAIN, "WRONG_DOCUMENT_ERR", null);
                throw new DOMException(DOMException.WRONG_DOCUMENT_ERR, msg);
            }
            doctypeImpl.ownerDocument = this;
            appendChild(doctype);
        }
    }

    //
    // Node methods
    //

    // even though ownerDocument refers to this in this implementation
    // the DOM Level 2 spec says it must be null, so make it appear so
    final public Document getOwnerDocument() {
        return null;
    }

    /** Returns the node type. */
    public short getNodeType() {
        return Node.DOCUMENT_NODE;
    }

    /** Returns the node name. */
    public String getNodeName() {
        return "#document";
    }

    /**
     * Deep-clone a document, including fixing ownerDoc for the cloned
     * children. Note that this requires bypassing the WRONG_DOCUMENT_ERR
     * protection. I've chosen to implement it by calling importNode
     * which is DOM Level 2.
     *
     * @return org.w3c.dom.Node
     * @param deep boolean, iff true replicate children
     */
    public Node cloneNode(boolean deep) {

        CoreDocumentImpl newdoc = new CoreDocumentImpl();
        callUserDataHandlers(this, newdoc, UserDataHandler.NODE_CLONED);
        cloneNode(newdoc, deep);

        return newdoc;

    } // cloneNode(boolean):Node


    /**
     * internal method to share code with subclass
     **/
    protected void cloneNode(CoreDocumentImpl newdoc, boolean deep) {

        // clone the children by importing them
        if (needsSyncChildren()) {
            synchronizeChildren();
        }

        if (deep) {
            Hashtable reversedIdentifiers = null;

            if (identifiers != null) {
                // Build a reverse mapping from element to identifier.
                reversedIdentifiers = new Hashtable();
                Enumeration elementIds = identifiers.keys();
                while (elementIds.hasMoreElements()) {
                    Object elementId = elementIds.nextElement();
                    reversedIdentifiers.put(identifiers.get(elementId),
                                            elementId);
                }
            }

            // Copy children into new document.
            for (ChildNode kid = firstChild; kid != null;
                 kid = kid.nextSibling) {
                newdoc.appendChild(newdoc.importNode(kid, true, true,
                                                     reversedIdentifiers));
            }
        }

        // experimental
        newdoc.allowGrammarAccess = allowGrammarAccess;
        newdoc.errorChecking = errorChecking;

    } // cloneNode(CoreDocumentImpl,boolean):void

    /**
     * Since a Document may contain at most one top-level Element child,
     * and at most one DocumentType declaraction, we need to subclass our
     * add-children methods to implement this constraint.
     * Since appendChild() is implemented as insertBefore(,null),
     * altering the latter fixes both.
     * <p>
     * While I'm doing so, I've taken advantage of the opportunity to
     * cache documentElement and docType so we don't have to
     * search for them.
     *
     * REVISIT: According to the spec it is not allowed to alter neither the
     * document element nor the document type in any way
     */
    public Node insertBefore(Node newChild, Node refChild)
        throws DOMException {

        // Only one such child permitted
        int type = newChild.getNodeType();
        if (errorChecking) {
            if((type == Node.ELEMENT_NODE && docElement != null) ||
               (type == Node.DOCUMENT_TYPE_NODE && docType != null)) {
                String msg = DOMMessageFormatter.formatMessage(DOMMessageFormatter.DOM_DOMAIN, "HIERARCHY_REQUEST_ERR", null);
                throw new DOMException(DOMException.HIERARCHY_REQUEST_ERR, msg);
            }
        }
        // Adopt orphan doctypes
        if (newChild.getOwnerDocument() == null &&
            newChild instanceof DocumentTypeImpl) {
            ((DocumentTypeImpl) newChild).ownerDocument = this;
        }
        super.insertBefore(newChild,refChild);

        // If insert succeeded, cache the kid appropriately
        if (type == Node.ELEMENT_NODE) {
            docElement = (ElementImpl)newChild;
        }
        else if (type == Node.DOCUMENT_TYPE_NODE) {
            docType = (DocumentTypeImpl)newChild;
        }

        return newChild;

    } // insertBefore(Node,Node):Node

    /**
     * Since insertBefore caches the docElement (and, currently, docType),
     * removeChild has to know how to undo the cache
     *
     * REVISIT: According to the spec it is not allowed to alter neither the
     * document element nor the document type in any way
     */
    public Node removeChild(Node oldChild) throws DOMException {

        super.removeChild(oldChild);

        // If remove succeeded, un-cache the kid appropriately
        int type = oldChild.getNodeType();
        if(type == Node.ELEMENT_NODE) {
            docElement = null;
        }
        else if (type == Node.DOCUMENT_TYPE_NODE) {
            docType = null;
        }

        return oldChild;

    }   // removeChild(Node):Node

    /**
     * Since we cache the docElement (and, currently, docType),
     * replaceChild has to update the cache
     *
     * REVISIT: According to the spec it is not allowed to alter neither the
     * document element nor the document type in any way
     */
    public Node replaceChild(Node newChild, Node oldChild)
        throws DOMException {

        // Adopt orphan doctypes
        if (newChild.getOwnerDocument() == null &&
            newChild instanceof DocumentTypeImpl) {
            ((DocumentTypeImpl) newChild).ownerDocument = this;
        }
        super.replaceChild(newChild, oldChild);

        int type = oldChild.getNodeType();
        if(type == Node.ELEMENT_NODE) {
            docElement = (ElementImpl)newChild;
        }
        else if (type == Node.DOCUMENT_TYPE_NODE) {
            docType = (DocumentTypeImpl)newChild;
        }
        return oldChild;
    }   // replaceChild(Node,Node):Node

    /* 
     * Get Node text content
     * @since DOM Level 3
     */
    public String getTextContent() throws DOMException {
        return null;
    }

    /*
     * Set Node text content
     * @since DOM Level 3
     */
    public void setTextContent(String textContent)
        throws DOMException {
        // no-op
    }

    //
    // Document methods
    //

    // factory methods

    /**
     * Factory method; creates an Attribute having this Document as its
     * OwnerDoc.
     *
     * @param name The name of the attribute. Note that the attribute's value
     * is _not_ established at the factory; remember to set it!
     *
     * @throws DOMException(INVALID_NAME_ERR) if the attribute name is not
     * acceptable.
     */
    public Attr createAttribute(String name)
        throws DOMException {

        if (errorChecking && !isXMLName(name)) {
            throw new DOMException(DOMException.INVALID_CHARACTER_ERR,
                                   "DOM002 Illegal character");
        }
        return new AttrImpl(this, name);

    } // createAttribute(String):Attr

    /**
     * Factory method; creates a CDATASection having this Document as
     * its OwnerDoc.
     *
     * @param data The initial contents of the CDATA
     *
     * @throws DOMException(NOT_SUPPORTED_ERR) for HTML documents. (HTML
     * not yet implemented.)
     */
    public CDATASection createCDATASection(String data)
        throws DOMException {
        return new CDATASectionImpl(this, data);
    }

    /**
     * Factory method; creates a Comment having this Document as its
     * OwnerDoc.
     *
     * @param data The initial contents of the Comment. */
    public Comment createComment(String data) {
        return new CommentImpl(this, data);
    }

    /**
     * Factory method; creates a DocumentFragment having this Document
     * as its OwnerDoc.
     */
    public DocumentFragment createDocumentFragment() {
        return new DocumentFragmentImpl(this);
    }

    /**
     * Factory method; creates an Element having this Document
     * as its OwnerDoc.
     *
     * @param tagName The name of the element type to instantiate. For
     * XML, this is case-sensitive. For HTML, the tagName parameter may
     * be provided in any case, but it must be mapped to the canonical
     * uppercase form by the DOM implementation.
     *
     * @throws DOMException(INVALID_NAME_ERR) if the tag name is not
     * acceptable.
     */
    public Element createElement(String tagName)
        throws DOMException {

        if (errorChecking && !isXMLName(tagName)) {
            String msg = DOMMessageFormatter.formatMessage(DOMMessageFormatter.DOM_DOMAIN, "INVALID_CHARACTER_ERR", null);
            throw new DOMException(DOMException.INVALID_CHARACTER_ERR, msg);
        }
        return new ElementImpl(this, tagName);

    } // createElement(String):Element

    /**
     * Factory method; creates an EntityReference having this Document
     * as its OwnerDoc.
     *
     * @param name The name of the Entity we wish to refer to
     *
     * @throws DOMException(NOT_SUPPORTED_ERR) for HTML documents, where
     * nonstandard entities are not permitted. (HTML not yet
     * implemented.)
     */
    public EntityReference createEntityReference(String name)
        throws DOMException {

        if (errorChecking && !isXMLName(name)) {
            String msg = DOMMessageFormatter.formatMessage(DOMMessageFormatter.DOM_DOMAIN, "INVALID_CHARACTER_ERR", null);
            throw new DOMException(DOMException.INVALID_CHARACTER_ERR, msg);
        }
        return new EntityReferenceImpl(this, name);

    } // createEntityReference(String):EntityReference

    /**
     * Factory method; creates a ProcessingInstruction having this Document
     * as its OwnerDoc.
     *
     * @param target The target "processor channel"
     * @param data Parameter string to be passed to the target.
     *
     * @throws DOMException(INVALID_NAME_ERR) if the target name is not
     * acceptable.
     *
     * @throws DOMException(NOT_SUPPORTED_ERR) for HTML documents. (HTML
     * not yet implemented.)
     */
    public ProcessingInstruction createProcessingInstruction(String target,
                                                             String data)
        throws DOMException {

        if (errorChecking && !isXMLName(target)) {
            String msg = DOMMessageFormatter.formatMessage(DOMMessageFormatter.DOM_DOMAIN, "INVALID_CHARACTER_ERR", null);
            throw new DOMException(DOMException.INVALID_CHARACTER_ERR, msg);
        }
        return new ProcessingInstructionImpl(this, target, data);

    } // createProcessingInstruction(String,String):ProcessingInstruction

    /**
     * Factory method; creates a Text node having this Document as its
     * OwnerDoc.
     *
     * @param data The initial contents of the Text.
     */
    public Text createTextNode(String data) {
        return new TextImpl(this, data);
    }

    // other document methods

    /**
     * For XML, this provides access to the Document Type Definition.
     * For HTML documents, and XML documents which don't specify a DTD,
     * it will be null.
     */
    public DocumentType getDoctype() {
        if (needsSyncChildren()) {
            synchronizeChildren();
        }
        return docType;
    }


    /**
     * Convenience method, allowing direct access to the child node
     * which is considered the root of the actual document content. For
     * HTML, where it is legal to have more than one Element at the top
     * level of the document, we pick the one with the tagName
     * "HTML". For XML there should be only one top-level
     *
     * (HTML not yet supported.)
     */
    public Element getDocumentElement() {
        if (needsSyncChildren()) {
            synchronizeChildren();
        }
        return docElement;
    }

    /**
     * Return a <em>live</em> collection of all descendent Elements (not just
     * immediate children) having the specified tag name.
     *
     * @param tagname The type of Element we want to gather. "*" will be
     * taken as a wildcard, meaning "all elements in the document."
     *
     * @see DeepNodeListImpl
     */
    public NodeList getElementsByTagName(String tagname) {
        return new DeepNodeListImpl(this,tagname);
    }

    /**
     * Retrieve information describing the abilities of this particular
     * DOM implementation. Intended to support applications that may be
     * using DOMs retrieved from several different sources, potentially
     * with different underlying representations.
     */
    public DOMImplementation getImplementation() {
        // Currently implemented as a singleton, since it's hardcoded
        // information anyway.
        return CoreDOMImplementationImpl.getDOMImplementation();
    }

    //
    // Public methods
    //

    // properties

    /**
     * Sets whether the DOM implementation performs error checking
     * upon operations. Turning off error checking only affects
     * the following DOM checks:
     * <ul>
     * <li>Checking strings to make sure that all characters are
     *     legal XML characters
     * <li>Hierarchy checking such as allowed children, checks for
     *     cycles, etc.
     * </ul>
     * <p>
     * Turning off error checking does <em>not</em> turn off the
     * following checks:
     * <ul>
     * <li>Read only checks
     * <li>Checks related to DOM events
     * </ul>
     */

    public void setErrorChecking(boolean check) {
        errorChecking = check;
    }

    /*
     * DOM Level 3 WD - Experimental.
     */
    public void setStrictErrorChecking(boolean check) {
        errorChecking = check;
    }

    /**
     * Returns true if the DOM implementation performs error checking.
     */
    public boolean getErrorChecking() {
        return errorChecking;
    }

    /*
     * DOM Level 3 WD - Experimental.
     */
    public boolean getStrictErrorChecking() {
        return errorChecking;
    }


    /**
     * DOM Level 3 WD - Experimental.
     * An attribute specifying the actual encoding of this document. This is
     * <code>null</code> otherwise.
     * <br> This attribute represents the property [character encoding scheme]
     * defined in .
     * @since DOM Level 3
     */
    public String getActualEncoding() {
        return actualEncoding;
    }

    /**
     * DOM Level 3 WD - Experimental.
     * An attribute specifying the actual encoding of this document. This is
     * <code>null</code> otherwise.
     * <br> This attribute represents the property [character encoding scheme]
     * defined in .
     * @since DOM Level 3
     */
    public void setActualEncoding(String value) {
        actualEncoding = value;
    }

    /**
      * DOM Level 3 WD - Experimental.
      * An attribute specifying, as part of the XML declaration,
      * the encoding of this document. This is null when unspecified.
      */
    public void setEncoding(String value) {
        encoding = value;
    }

    /**
     * DOM Level 3 WD - Experimental.
     * The encoding of this document (part of XML Declaration)
     */
    public String getEncoding() {
        return encoding;
    }

    /**
      * DOM Level 3 WD - Experimental.
      * version - An attribute specifying, as part of the XML declaration,
      * the version number of this document. This is null when unspecified
      */
    public void setVersion(String value) {
       version = value;
    }

    /**
     * DOM Level 3 WD - Experimental.
     * The version of this document (part of XML Declaration)
     */
    public String getVersion() {
        return version;
    }

    /**
     * DOM Level 3 WD - Experimental.
     * standalone - An attribute specifying, as part of the XML declaration,
     * whether this document is standalone
     */
    public void setStandalone(boolean value) {
        standalone = value;
    }

    /**
     * DOM Level 3 WD - Experimental.
     * standalone that specifies whether this document is standalone
     * (part of XML Declaration)
     */
    public boolean getStandalone() {
        return standalone;
    }


    /**
     * DOM Level 3 WD - Experimental.
     * The location of the document or <code>null</code> if undefined.
     * <br>Beware that when the <code>Document</code> supports the feature 
     * "HTML" , the href attribute of the HTML BASE element takes precedence 
     * over this attribute.
     * @since DOM Level 3
     */
    public String getDocumentURI(){
        return fDocumentURI;
    }


    /**
     * DOM Level 3 WD - Experimental.
     * Renaming node
     */
    public Node renameNode(Node n, 
                           String namespaceURI, 
                           String name)
                           throws DOMException{

	if (n.getOwnerDocument() != this) {
            String msg = DOMMessageFormatter.formatMessage(DOMMessageFormatter.DOM_DOMAIN, "WRONG_DOCUMENT_ERR", null);
            throw new DOMException(DOMException.WRONG_DOCUMENT_ERR, msg);
	}
        switch (n.getNodeType()) {
            case ELEMENT_NODE: {
                ElementImpl el = (ElementImpl) n;
		if (el instanceof ElementNSImpl) {
                    ((ElementNSImpl) el).rename(namespaceURI, name);
		}
		else {
		    if (namespaceURI == null) {
                        el.rename(name);
		    }
		    else {
			// we need to create a new object
			ElementNSImpl nel =
			    new ElementNSImpl(this, namespaceURI, name);

			// register event listeners on new node
			copyEventListeners(el, nel);

			// remove user data from old node
			Hashtable data = removeUserDataTable(el);

			// remove old node from parent if any
			Node parent = el.getParentNode();
			Node nextSib = el.getNextSibling();
			if (parent != null) {
			    parent.removeChild(el);
			}
			// move children to new node
			Node child = el.getFirstChild();
			while (child != null) {
			    el.removeChild(child);
			    nel.appendChild(child);
			    child = el.getFirstChild();
			}
			// move specified attributes to new node
			nel.moveSpecifiedAttributes(el);
		    
			// attach user data to new node
			setUserDataTable(nel, data);

			// and fire user data NODE_RENAMED event
			callUserDataHandlers(el, nel,
					     UserDataHandler.NODE_RENAMED);

			// insert new node where old one was
			if (parent != null) {
			    parent.insertBefore(nel, nextSib);
			}
			el = nel;
		    }
		}
		// fire ElementNameChanged event
		renamedElement((Element) n, el);
		return el;
	    }
            case ATTRIBUTE_NODE: {
                AttrImpl at = (AttrImpl) n;

                // dettach attr from element
                Element el = at.getOwnerElement();
                if (el != null) {
                    el.removeAttributeNode(at);
                }
		if (n instanceof AttrNSImpl) {
		    ((AttrNSImpl) at).rename(namespaceURI, name);
                    // reattach attr to element
                    if (el != null) {
                        el.setAttributeNodeNS(at);
                    }
		}
		else {
		    if (namespaceURI == null) {
			at.rename(name);
			// reattach attr to element
                        if (el != null) {
                            el.setAttributeNode(at);
                        }
		    }
		    else {
			// we need to create a new object
			AttrNSImpl nat =
			    new AttrNSImpl(this, namespaceURI, name);

			// register event listeners on new node
			copyEventListeners(at, nat);

			// remove user data from old node
			Hashtable data = removeUserDataTable(at);

			// move children to new node
			Node child = at.getFirstChild();
			while (child != null) {
			    at.removeChild(child);
			    nat.appendChild(child);
			    child = at.getFirstChild();
			}
		    
			// attach user data to new node
			setUserDataTable(nat, data);

			// and fire user data NODE_RENAMED event
			callUserDataHandlers(at, nat,
					     UserDataHandler.NODE_RENAMED);

			// reattach attr to element
			if (el != null) {
			    el.setAttributeNode(nat);
			}
			at = nat;
		    }
		}
		// fire AttributeNameChanged event
		renamedAttrNode((Attr) n, at);
		
		return at;
	    }
	    default: { 
                String msg = DOMMessageFormatter.formatMessage(DOMMessageFormatter.DOM_DOMAIN, "NOT_SUPPORTED_ERR", null);
                throw new DOMException(DOMException.NOT_SUPPORTED_ERR, msg);
	    }
	}

    }


    /**
     *  DOM Level 3 WD - Experimental
     *  Normalize document.
     */
    public void normalizeDocument(){
        // No need to normalize if already normalized.
        if (isNormalized() && !isNormalizeDocRequired()) {
            return;
        }
        if (needsSyncChildren()) {
            synchronizeChildren();
        }
        
        if (domNormalizer == null) {
            domNormalizer = new DOMNormalizer();
        }

        if (fConfiguration == null) {
            fConfiguration =  new DOMConfigurationImpl();
        }
        else {
            fConfiguration.reset();
        }

        domNormalizer.normalizeDocument(this, fConfiguration);
        isNormalized(true);
    }


    /**
     *  The configuration used when <code>Document.normalizeDocument</code> is 
     * invoked. 
     * @since DOM Level 3
     */
    public DOMConfiguration getConfig(){
        if (fConfiguration == null) {
            fConfiguration = new DOMConfigurationImpl();
        }
        return fConfiguration;
    }

    
    /**
     * DOM Level 3 WD - Experimental.
     * Retrieve baseURI
     */
    public String getBaseURI() {
        return fDocumentURI;            
    }

    /**
     * DOM Level 3 WD - Experimental.
     */
    public void setDocumentURI(String documentURI){
        fDocumentURI = documentURI;
    }


    //
    // DOM L3 LS
    //
    /**
     * DOM Level 3 WD - Experimental.
     * Indicates whether the method load should be synchronous or 
     * asynchronous. When the async attribute is set to <code>true</code> 
     * the load method returns control to the caller before the document has 
     * completed loading. The default value of this property is 
     * <code>false</code>.
     * <br>Setting the value of this attribute might throw NOT_SUPPORTED_ERR 
     * if the implementation doesn't support the mode the attribute is being 
     * set to. Should the DOM spec define the default value of this 
     * property? What if implementing both async and sync IO is impractical 
     * in some systems?  2001-09-14. default is <code>false</code> but we 
     * need to check with Mozilla and IE. 
     */
    public boolean getAsync() {
        return false;
    }
    
    /**
     * DOM Level 3 WD - Experimental.
     * Indicates whether the method load should be synchronous or 
     * asynchronous. When the async attribute is set to <code>true</code> 
     * the load method returns control to the caller before the document has 
     * completed loading. The default value of this property is 
     * <code>false</code>.
     * <br>Setting the value of this attribute might throw NOT_SUPPORTED_ERR 
     * if the implementation doesn't support the mode the attribute is being 
     * set to. Should the DOM spec define the default value of this 
     * property? What if implementing both async and sync IO is impractical 
     * in some systems?  2001-09-14. default is <code>false</code> but we 
     * need to check with Mozilla and IE. 
     */
    public void setAsync(boolean async) {
        if (async) {
            String msg = DOMMessageFormatter.formatMessage(DOMMessageFormatter.DOM_DOMAIN, "NOT_SUPPORTED_ERR", null);
            throw new DOMException(DOMException.NOT_SUPPORTED_ERR, msg);
        }
    }
    /**
     * DOM Level 3 WD - Experimental.
     * If the document is currently being loaded as a result of the method 
     * <code>load</code> being invoked the loading and parsing is 
     * immediately aborted. The possibly partial result of parsing the 
     * document is discarded and the document is cleared.
     */

    public void abort() {
    }
    /**
     * DOM Level 3 WD - Experimental.
     * Replaces the content of the document with the result of parsing the 
     * given URI. Invoking this method will either block the caller or 
     * return to the caller immediately depending on the value of the async 
     * attribute. Once the document is fully loaded the document will fire a 
     * "load" event that the caller can register as a listener for. If an 
     * error occurs the document will fire an "error" event so that the 
     * caller knows that the load failed (see <code>ParseErrorEvent</code>).
     * @param uri The URI reference for the XML file to be loaded. If this is 
     *   a relative URI...
     * @return If async is set to <code>true</code> <code>load</code> returns 
     *   <code>true</code> if the document load was successfully initiated. 
     *   If an error occurred when initiating the document load 
     *   <code>load</code> returns <code>false</code>.If async is set to 
     *   <code>false</code> <code>load</code> returns <code>true</code> if 
     *   the document was successfully loaded and parsed. If an error 
     *   occurred when either loading or parsing the URI <code>load</code> 
     *   returns <code>false</code>.
     */
    public boolean load(String uri) {
        return false;
    }
    /**
     * DOM Level 3 WD - Experimental.
     * Replace the content of the document with the result of parsing the 
     * input string, this method is always synchronous.
     * @param source A string containing an XML document.
     * @return <code>true</code> if parsing the input string succeeded 
     *   without errors, otherwise <code>false</code>.
     */
    public boolean loadXML(String source) {
        return false;
    }
    
    /**
     * DOM Level 3 WD - Experimental.
     * Save the document or the given node to a string (i.e. serialize the 
     * document or node).
     * @param snode Specifies what to serialize, if this parameter is 
     *   <code>null</code> the whole document is serialized, if it's 
     *   non-null the given node is serialized.
     * @return The serialized document or <code>null</code>.
     * @exception DOMException
     *   WRONG_DOCUMENT_ERR: Raised if the node passed in as the node 
     *   parameter is from an other document.
     */
    public String saveXML(Node snode)
                          throws DOMException {
        if ( snode != null &&                                                      
             getOwnerDocument() != snode.getOwnerDocument() ) {
            String msg = DOMMessageFormatter.formatMessage(DOMMessageFormatter.DOM_DOMAIN, "WRONG_DOCUMENT_ERR", null);
            throw new DOMException(DOMException.WRONG_DOCUMENT_ERR, msg);
        }
        DOMImplementationLS domImplLS = (DOMImplementationLS)DOMImplementationImpl.getDOMImplementation();
        DOMWriter xmlWriter = domImplLS.createDOMWriter();
        if (snode == null) {
            snode = this;
        }
        return xmlWriter.writeToString(snode);
    }


    /**
     * Sets whether the DOM implementation generates mutation events
     * upon operations.
     */
    void setMutationEvents(boolean set) {
        // does nothing by default - overidden in subclass
    }

    /**
     * Returns true if the DOM implementation generates mutation events.
     */
    boolean getMutationEvents() {
        // does nothing by default - overriden in subclass
        return false;
    }



    // non-DOM factory methods

    /**
     * NON-DOM
     * Factory method; creates a DocumentType having this Document
     * as its OwnerDoc. (REC-DOM-Level-1-19981001 left the process of building
     * DTD information unspecified.)
     *
     * @param name The name of the Entity we wish to provide a value for.
     *
     * @throws DOMException(NOT_SUPPORTED_ERR) for HTML documents, where
     * DTDs are not permitted. (HTML not yet implemented.)
     */
    public DocumentType createDocumentType(String qualifiedName,
                                           String publicID,
                                           String systemID)
        throws DOMException {

        if (errorChecking && !isXMLName(qualifiedName)) {
            String msg = DOMMessageFormatter.formatMessage(DOMMessageFormatter.DOM_DOMAIN, "INVALID_CHARACTER_ERR", null);
            throw new DOMException(DOMException.INVALID_CHARACTER_ERR, msg);
        }
        return new DocumentTypeImpl(this, qualifiedName, publicID, systemID);

    } // createDocumentType(String):DocumentType

    /**
     * NON-DOM
     * Factory method; creates an Entity having this Document
     * as its OwnerDoc. (REC-DOM-Level-1-19981001 left the process of building
     * DTD information unspecified.)
     *
     * @param name The name of the Entity we wish to provide a value for.
     *
     * @throws DOMException(NOT_SUPPORTED_ERR) for HTML documents, where
     * nonstandard entities are not permitted. (HTML not yet
     * implemented.)
     */
    public Entity createEntity(String name)
        throws DOMException {

        if (errorChecking && !isXMLName(name)) {
            String msg = DOMMessageFormatter.formatMessage(DOMMessageFormatter.DOM_DOMAIN, "INVALID_CHARACTER_ERR", null);
            throw new DOMException(DOMException.INVALID_CHARACTER_ERR, msg);
        }
        return new EntityImpl(this, name);

    } // createEntity(String):Entity

    /**
     * NON-DOM
     * Factory method; creates a Notation having this Document
     * as its OwnerDoc. (REC-DOM-Level-1-19981001 left the process of building
     * DTD information unspecified.)
     *
     * @param name The name of the Notation we wish to describe
     *
     * @throws DOMException(NOT_SUPPORTED_ERR) for HTML documents, where
     * notations are not permitted. (HTML not yet
     * implemented.)
     */
    public Notation createNotation(String name)
        throws DOMException {

        if (errorChecking && !isXMLName(name)) {
            String msg = DOMMessageFormatter.formatMessage(DOMMessageFormatter.DOM_DOMAIN, "INVALID_CHARACTER_ERR", null);
            throw new DOMException(DOMException.INVALID_CHARACTER_ERR, msg);
        }
        return new NotationImpl(this, name);

    } // createNotation(String):Notation

    /**
     * NON-DOM Factory method: creates an element definition. Element
     * definitions hold default attribute values.
     */
    public ElementDefinitionImpl createElementDefinition(String name)
        throws DOMException {

        if (errorChecking && !isXMLName(name)) {
            String msg = DOMMessageFormatter.formatMessage(DOMMessageFormatter.DOM_DOMAIN, "INVALID_CHARACTER_ERR", null);
            throw new DOMException(DOMException.INVALID_CHARACTER_ERR, msg);
        }
        return new ElementDefinitionImpl(this, name);

    } // createElementDefinition(String):ElementDefinitionImpl

    // other non-DOM methods

    /**
     * Copies a node from another document to this document. The new nodes are
     * created using this document's factory methods and are populated with the
     * data from the source's accessor methods defined by the DOM interfaces.
     * Its behavior is otherwise similar to that of cloneNode.
     * <p>
     * According to the DOM specifications, document nodes cannot be imported
     * and a NOT_SUPPORTED_ERR exception is thrown if attempted.
     */
    public Node importNode(Node source, boolean deep)
        throws DOMException {
        return importNode(source, deep, false, null);
    } // importNode(Node,boolean):Node

    /**
     * Overloaded implementation of DOM's importNode method. This method
     * provides the core functionality for the public importNode and cloneNode
     * methods.
     *
     * The reversedIdentifiers parameter is provided for cloneNode to
     * preserve the document's identifiers. The Hashtable has Elements as the
     * keys and their identifiers as the values. When an element is being
     * imported, a check is done for an associated identifier. If one exists,
     * the identifier is registered with the new, imported element. If
     * reversedIdentifiers is null, the parameter is not applied.
     */
    private Node importNode(Node source, boolean deep, boolean cloningDoc,
                            Hashtable reversedIdentifiers)
        throws DOMException {
        Node newnode=null;

        // Sigh. This doesn't work; too many nodes have private data that
        // would have to be manually tweaked. May be able to add local
        // shortcuts to each nodetype. Consider ?????
        // if(source instanceof NodeImpl &&
        //  !(source instanceof DocumentImpl))
        // {
        //  // Can't clone DocumentImpl since it invokes us...
        //  newnode=(NodeImpl)source.cloneNode(false);
        //  newnode.ownerDocument=this;
        // }
        // else
        
        int type = source.getNodeType();
        switch (type) {
            case ELEMENT_NODE: {
                Element newElement;
                boolean domLevel20 = source.getOwnerDocument().getImplementation().hasFeature("XML", "2.0");
                // Create element according to namespace support/qualification.
                if(domLevel20 == false || source.getLocalName() == null)
                    newElement = createElement(source.getNodeName());
                else
                    newElement = createElementNS(source.getNamespaceURI(),
                                                 source.getNodeName());

                // Copy element's attributes, if any.
                NamedNodeMap sourceAttrs = source.getAttributes();
                if (sourceAttrs != null) {
                    int length = sourceAttrs.getLength();
                    for (int index = 0; index < length; index++) {
                        Attr attr = (Attr)sourceAttrs.item(index);

                        // NOTE: this methods is used for both importingNode
                        // and cloning the document node. In case of the 
                        // clonning default attributes should be copied.
                        // But for importNode defaults should be ignored.
                        if (attr.getSpecified() || cloningDoc) {
                            Attr newAttr = (Attr)importNode(attr, true, cloningDoc,
                                                          reversedIdentifiers);

                            // Attach attribute according to namespace
                            // support/qualification.
                            if (domLevel20 == false ||
                                attr.getLocalName() == null)
                                newElement.setAttributeNode(newAttr);
                            else
                                newElement.setAttributeNodeNS(newAttr);
                        }
                    }
                }

                // Register element identifier.
                if (reversedIdentifiers != null) {
                    // Does element have an associated identifier?
                    Object elementId = reversedIdentifiers.get(source);
                    if (elementId != null) {
                        if (identifiers == null)
                            identifiers = new Hashtable();

                        identifiers.put(elementId, newElement);
                    }
                }

                newnode = newElement;
                break;
            }

            case ATTRIBUTE_NODE: {

                if( source.getOwnerDocument().getImplementation().hasFeature("XML", "2.0") ){
                    if (source.getLocalName() == null) {
                        newnode = createAttribute(source.getNodeName());
                    } else {
                        newnode = createAttributeNS(source.getNamespaceURI(),
                                                    source.getNodeName());
                    }
                }
                else {
                    newnode = createAttribute(source.getNodeName());
                }
                // if source is an AttrImpl from this very same implementation
                // avoid creating the child nodes if possible
                if (source instanceof AttrImpl) {
                    AttrImpl attr = (AttrImpl) source;
                    if (attr.hasStringValue()) {
                        AttrImpl newattr = (AttrImpl) newnode;
                        newattr.setValue(attr.getValue());
                        deep = false;
                    }
                    else {
                        deep = true;
                    }
                }
                else {
                    // According to the DOM spec the kids carry the value.
                    // However, there are non compliant implementations out
                    // there that fail to do so. To avoid ending up with no
                    // value at all, in this case we simply copy the text value
                    // directly.
                    if (source.getFirstChild() == null) {
                        newnode.setNodeValue(source.getNodeValue());
                        deep = false;
                    } else {
                        deep = true;
                    }
                }
                break;
            }

            case TEXT_NODE: {
                newnode = createTextNode(source.getNodeValue());
                break;
            }

            case CDATA_SECTION_NODE: {
                newnode = createCDATASection(source.getNodeValue());
                break;
            }

            case ENTITY_REFERENCE_NODE: {
                newnode = createEntityReference(source.getNodeName());
                // the subtree is created according to this doc by the method
                // above, so avoid carrying over original subtree
                deep = false;
                break;
            }

            case ENTITY_NODE: {
                Entity srcentity = (Entity)source;
                EntityImpl newentity =
                    (EntityImpl)createEntity(source.getNodeName());
                newentity.setPublicId(srcentity.getPublicId());
                newentity.setSystemId(srcentity.getSystemId());
                newentity.setNotationName(srcentity.getNotationName());
                // Kids carry additional value,
                // allow deep import temporarily
                newentity.isReadOnly(false);
                newnode = newentity;
                break;
            }

            case PROCESSING_INSTRUCTION_NODE: {
                newnode = createProcessingInstruction(source.getNodeName(),
                                                      source.getNodeValue());
                break;
            }

            case COMMENT_NODE: {
                newnode = createComment(source.getNodeValue());
                break;
            }
            
            case DOCUMENT_TYPE_NODE: {
                // unless this is used as part of cloning a Document
                // forbid it for the sake of being compliant to the DOM spec
                if (!cloningDoc) {
                    String msg = DOMMessageFormatter.formatMessage(DOMMessageFormatter.DOM_DOMAIN, "NOT_SUPPORTED_ERR", null);
                    throw new DOMException(DOMException.NOT_SUPPORTED_ERR, msg);
                }
                DocumentType srcdoctype = (DocumentType)source;
                DocumentTypeImpl newdoctype = (DocumentTypeImpl)
                    createDocumentType(srcdoctype.getNodeName(),
                                       srcdoctype.getPublicId(),
                                       srcdoctype.getSystemId());
                // Values are on NamedNodeMaps
                NamedNodeMap smap = srcdoctype.getEntities();
                NamedNodeMap tmap = newdoctype.getEntities();
                if(smap != null) {
                    for(int i = 0; i < smap.getLength(); i++) {
                        tmap.setNamedItem(importNode(smap.item(i), true, true,
                                                     reversedIdentifiers));
                    }
                }
                smap = srcdoctype.getNotations();
                tmap = newdoctype.getNotations();
                if (smap != null) {
                    for(int i = 0; i < smap.getLength(); i++) {
                        tmap.setNamedItem(importNode(smap.item(i), true, true,
                                                     reversedIdentifiers));
                    }
                }
                
                // NOTE: At this time, the DOM definition of DocumentType
                // doesn't cover Elements and their Attributes. domimpl's
                // extentions in that area will not be preserved, even if
                // copying from domimpl to domimpl. We could special-case
                // that here. Arguably we should. Consider. ?????
                newnode = newdoctype;
                break;
            }

            case DOCUMENT_FRAGMENT_NODE: {
                newnode = createDocumentFragment();
                // No name, kids carry value
                break;
            }

            case NOTATION_NODE: {
                Notation srcnotation = (Notation)source;
                NotationImpl newnotation =
                    (NotationImpl)createNotation(source.getNodeName());
                newnotation.setPublicId(srcnotation.getPublicId());
                newnotation.setSystemId(srcnotation.getSystemId());
                // Kids carry additional value
                newnode = newnotation;
                // No name, no value
                break;
            }
            case DOCUMENT_NODE : // Can't import document nodes
            default: {           // Unknown node type
                String msg = DOMMessageFormatter.formatMessage(DOMMessageFormatter.DOM_DOMAIN, "NOT_SUPPORTED_ERR", null);
                throw new DOMException(DOMException.NOT_SUPPORTED_ERR, msg);
            }
        }

        callUserDataHandlers(source, newnode, UserDataHandler.NODE_IMPORTED);

        // If deep, replicate and attach the kids.
        if (deep) {
            for (Node srckid = source.getFirstChild();
                 srckid != null;
                 srckid = srckid.getNextSibling()) {
                newnode.appendChild(importNode(srckid, true, cloningDoc,
                                               reversedIdentifiers));
            }
        }
        if (newnode.getNodeType() == Node.ENTITY_NODE) {
          ((NodeImpl)newnode).setReadOnly(true, true);
        }
        return newnode;

    } // importNode(Node,boolean,boolean,Hashtable):Node

    /**
     * DOM Level 3 WD - Experimental
     * Change the node's ownerDocument, and its subtree, to this Document
     *
     * @param source The node to adopt.
     * @see #importNode
     **/
    public Node adoptNode(Node source) {
        NodeImpl node;
        try {
            node = (NodeImpl) source;
        } catch (ClassCastException e) {
            // source node comes from a different DOMImplementation
            return null;
        }
        switch (node.getNodeType()) {
            case ATTRIBUTE_NODE: {
                AttrImpl attr = (AttrImpl) node;
                // remove node from wherever it is
                attr.getOwnerElement().removeAttributeNode(attr);
                // mark it as specified
                attr.isSpecified(true);
                // change ownership
                attr.setOwnerDocument(this);
                break;
            }
            case DOCUMENT_NODE:
            case DOCUMENT_TYPE_NODE: {
                String msg = DOMMessageFormatter.formatMessage(DOMMessageFormatter.DOM_DOMAIN, "NOT_SUPPORTED_ERR", null);
                throw new DOMException(DOMException.NOT_SUPPORTED_ERR, msg);
            }
            case ENTITY_REFERENCE_NODE: {
                // remove node from wherever it is
                Node parent = node.getParentNode();
                if (parent != null) {
                    parent.removeChild(source);
                }
                // discard its replacement value
                Node child;
                while ((child = node.getFirstChild()) != null) {
                    node.removeChild(child);
                }
                // change ownership
                node.setOwnerDocument(this);
                // set its new replacement value if any
                if (docType == null) {
                    break;
                }
                NamedNodeMap entities = docType.getEntities();
                Node entityNode = entities.getNamedItem(node.getNodeName());
                if (entityNode == null) {
                    break;
                }
                EntityImpl entity = (EntityImpl) entityNode;
                for (child = entityNode.getFirstChild();
                     child != null; child = child.getNextSibling()) {
                    Node childClone = child.cloneNode(true);
                    node.appendChild(childClone);
                }
                break;
            }
            case ELEMENT_NODE: {
                // remove node from wherever it is
                Node parent = node.getParentNode();
                if (parent != null) {
                    parent.removeChild(source);
                }
                // change ownership
                node.setOwnerDocument(this);
                // reconcile default attributes
                ((ElementImpl)node).reconcileDefaultAttributes();
                break;
            }
            default: {
                // remove node from wherever it is
                Node parent = node.getParentNode();
                if (parent != null) {
                    parent.removeChild(source);
                }
                // change ownership
                node.setOwnerDocument(this);
            }
        }
        return node;
    }

    // identifier maintenence
    /**
     * Introduced in DOM Level 2
     * Returns the Element whose ID is given by elementId. If no such element
     * exists, returns null. Behavior is not defined if more than one element
     * has this ID.
     * <p>
     * Note: The DOM implementation must have information that says which
     * attributes are of type ID. Attributes with the name "ID" are not of type
     * ID unless so defined. Implementations that do not know whether
     * attributes are of type ID or not are expected to return null.
     * @see #getIdentifier
     */
    public Element getElementById(String elementId) {
        return getIdentifier(elementId);
    }

	/**
	 * Remove all identifiers from the ID table
	 */
    protected final void clearIdentifiers (){
        if (identifiers != null){
            identifiers.clear();
        }
    }

    /**
     * Registers an identifier name with a specified element node.
     * If the identifier is already registered, the new element
     * node replaces the previous node. If the specified element
     * node is null, removeIdentifier() is called.
     *
     * @see #getIdentifier
     * @see #removeIdentifier
     */
    public void putIdentifier(String idName, Element element) {

        if (element == null) {
            removeIdentifier(idName);
            return;
        }

        if (needsSyncData()) {
            synchronizeData();
        }

        if (identifiers == null) {
            identifiers = new Hashtable();
        }

        identifiers.put(idName, element);

    } // putIdentifier(String,Element)

    /**
     * Returns a previously registered element with the specified
     * identifier name, or null if no element is registered.
     *
     * @see #putIdentifier
     * @see #removeIdentifier
     */
	public Element getIdentifier(String idName) {

		if (needsSyncData()) {
			synchronizeData();
		}

		if (identifiers == null) {
			return null;
		}
		Element elem = (Element) identifiers.get(idName);
		if (elem != null) {
			// check that the element is in the tree
			Node parent = elem.getParentNode();
			while (parent != null) {
				if (parent == this) {
					return elem;
				}
				parent = parent.getParentNode();
			}
		}
		return null;
	} // getIdentifier(String):Element

    /**
     * Removes a previously registered element with the specified
     * identifier name.
     *
     * @see #putIdentifier
     * @see #getIdentifier
     */
    public void removeIdentifier(String idName) {

        if (needsSyncData()) {
            synchronizeData();
        }

        if (identifiers == null) {
            return;
        }

        identifiers.remove(idName);

    } // removeIdentifier(String)

    /** Returns an enumeration registered of identifier names. */
    public Enumeration getIdentifiers() {

        if (needsSyncData()) {
            synchronizeData();
        }

        if (identifiers == null) {
            identifiers = new Hashtable();
        }

        return identifiers.keys();

    } // getIdentifiers():Enumeration

    //
    // DOM2: Namespace methods
    //

    /**
     * Introduced in DOM Level 2. <p>
     * Creates an element of the given qualified name and namespace URI.
     * If the given namespaceURI is null or an empty string and the
     * qualifiedName has a prefix that is "xml", the created element
     * is bound to the predefined namespace
     * "http://www.w3.org/XML/1998/namespace" [Namespaces].
     * @param namespaceURI The namespace URI of the element to
     *                     create.
     * @param qualifiedName The qualified name of the element type to
     *                      instantiate.
     * @return Element A new Element object with the following attributes:
     * @throws DOMException INVALID_CHARACTER_ERR: Raised if the specified
                            name contains an invalid character.
     * @throws DOMException NAMESPACE_ERR: Raised if the qualifiedName has a
     *                      prefix that is "xml" and the namespaceURI is
     *                      neither null nor an empty string nor
     *                      "http://www.w3.org/XML/1998/namespace", or
     *                      if the qualifiedName has a prefix different
     *                      from "xml" and the namespaceURI is null or an
     *                      empty string.
     * @since WD-DOM-Level-2-19990923
     */
    public Element createElementNS(String namespaceURI, String qualifiedName)
        throws DOMException
    {
        if (errorChecking && !isXMLName(qualifiedName)) {
            String msg = DOMMessageFormatter.formatMessage(DOMMessageFormatter.DOM_DOMAIN, "INVALID_CHARACTER_ERR", null);
            throw new DOMException(DOMException.INVALID_CHARACTER_ERR, msg);
        }
        return new ElementNSImpl(this, namespaceURI, qualifiedName);
    }

    /**
     * NON-DOM: a factory method used by the Xerces DOM parser 
     * to create an element.
     * 
     * @param namespaceURI The namespace URI of the element to
     *                     create.
     * @param qualifiedName The qualified name of the element type to
     *                      instantiate.
     * @param localpart  The local name of the attribute to instantiate.
     *
     * @return Element A new Element object with the following attributes:
     * @exception DOMException INVALID_CHARACTER_ERR: Raised if the specified
     *                   name contains an invalid character.
     */
    public Element createElementNS(String namespaceURI, String qualifiedName,
                                   String localpart)
        throws DOMException
    {
        return new ElementNSImpl(this, namespaceURI, qualifiedName, localpart);
    }
    
    /**
     * Introduced in DOM Level 2. <p>
     * Creates an attribute of the given qualified name and namespace URI.
     * If the given namespaceURI is null or an empty string and the
     * qualifiedName has a prefix that is "xml", the created element
     * is bound to the predefined namespace
     * "http://www.w3.org/XML/1998/namespace" [Namespaces].
     *
     * @param namespaceURI  The namespace URI of the attribute to
     *                      create. When it is null or an empty string,
     *                      this method behaves like createAttribute.
     * @param qualifiedName The qualified name of the attribute to
     *                      instantiate.
     * @return Attr         A new Attr object.
     * @throws DOMException INVALID_CHARACTER_ERR: Raised if the specified
                            name contains an invalid character.
     * @since WD-DOM-Level-2-19990923
     */
    public Attr createAttributeNS(String namespaceURI, String qualifiedName)
        throws DOMException
    {
        return new AttrNSImpl(this, namespaceURI, qualifiedName);
    }

    /**
     * NON-DOM: a factory method used by the Xerces DOM parser 
     * to create an element.
     *
     * @param namespaceURI  The namespace URI of the attribute to
     *                      create. When it is null or an empty string,
     *                      this method behaves like createAttribute.
     * @param qualifiedName The qualified name of the attribute to
     *                      instantiate.
     * @param localpart     The local name of the attribute to instantiate.
     *
     * @return Attr         A new Attr object.
     * @throws DOMException INVALID_CHARACTER_ERR: Raised if the specified
                            name contains an invalid character.
     */
    public Attr createAttributeNS(String namespaceURI, String qualifiedName,
                                  String localpart)
        throws DOMException
    {
        return new AttrNSImpl(this, namespaceURI, qualifiedName, localpart);
    }

    /**
     * Introduced in DOM Level 2. <p>
     * Returns a NodeList of all the Elements with a given local name and
     * namespace URI in the order in which they would be encountered in a
     * preorder traversal of the Document tree.
     * @param namespaceURI  The namespace URI of the elements to match
     *                      on. The special value "*" matches all
     *                      namespaces. When it is null or an empty
     *                      string, this method behaves like
     *                      getElementsByTagName.
     * @param localName     The local name of the elements to match on.
     *                      The special value "*" matches all local names.
     * @return NodeList     A new NodeList object containing all the matched
     *                      Elements.
     * @since WD-DOM-Level-2-19990923
     */
    public NodeList getElementsByTagNameNS(String namespaceURI,
                                           String localName)
    {
        return new DeepNodeListImpl(this, namespaceURI, localName);
    }

    //
    // Object methods
    //

    /** Clone. */
    public Object clone() throws CloneNotSupportedException {
        CoreDocumentImpl newdoc = (CoreDocumentImpl) super.clone();
        newdoc.docType = null;
        newdoc.docElement = null;
        return newdoc;
    }

    //
    // Public static methods
    //

    /**
     * Check the string against XML's definition of acceptable names for
     * elements and attributes and so on using the XMLCharacterProperties
     * utility class
     */
    public static boolean isXMLName(String s) {

        if (s == null) {
            return false;
        }
        return XMLChar.isValidName(s);

    } // isXMLName(String):boolean

   
    //
    // Protected methods
    //

    /**
     * Uses the kidOK lookup table to check whether the proposed
     * tree structure is legal.
     */
    protected boolean isKidOK(Node parent, Node child) {
        if (allowGrammarAccess &&
            parent.getNodeType() == Node.DOCUMENT_TYPE_NODE) {
            return child.getNodeType() == Node.ELEMENT_NODE;
        }
        return 0 != (kidOK[parent.getNodeType()] & 1 << child.getNodeType());
    }

    /**
     * Denotes that this node has changed.
     */
    protected void changed() {
        changes++;
    }

    /**
     * Returns the number of changes to this node.
     */
    protected int changes() {
        return changes;
    }

    //  NodeListCache pool

    /**
     * Returns a NodeListCache for the given node.
     */
    NodeListCache getNodeListCache(ParentNode owner) {
        if (fFreeNLCache == null) {
            return new NodeListCache(owner);
        }
        NodeListCache c = fFreeNLCache;
        fFreeNLCache = fFreeNLCache.next;
        c.fChild = null;
        c.fChildIndex = -1;
        c.fLength = -1;
        // revoke previous ownership
        if (c.fOwner != null) {
            c.fOwner.fNodeListCache = null;
        }
        c.fOwner = owner;
        // c.next = null; not necessary, except for confused people...
        return c;
    }

    /**
     * Puts the given NodeListCache in the free list.
     * Note: The owner node can keep using it until we reuse it
     */
    void freeNodeListCache(NodeListCache c) {
        c.next = fFreeNLCache;
        fFreeNLCache = c;
    }


    /*
     * a class to store some user data along with its handler
     */
    class UserDataRecord implements Serializable {
        Object fData;
        UserDataHandler fHandler;
        UserDataRecord(Object data, UserDataHandler handler) {
            fData = data;
            fHandler = handler;
        }
    }

    /**
     * Associate an object to a key on this node. The object can later be 
     * retrieved from this node by calling <code>getUserData</code> with the 
     * same key.
     * @param n The node to associate the object to.
     * @param key The key to associate the object to.
     * @param data The object to associate to the given key, or 
     *   <code>null</code> to remove any existing association to that key.
     * @param handler The handler to associate to that key, or 
     *   <code>null</code>.
     * @return Returns the <code>DOMObject</code> previously associated to 
     *   the given key on this node, or <code>null</code> if there was none.
     * @since DOM Level 3
     *
     * REVISIT: we could use a free list of UserDataRecord here
     */
    public Object setUserData(Node n, String key, 
                              Object data, UserDataHandler handler) {
        if (data == null) {
            if (userData != null) {
                Hashtable t = (Hashtable) userData.get(n);
                if (t != null) {
                    Object o = t.remove(key);
                    if (o != null) {
                        UserDataRecord r = (UserDataRecord) o;
                        return r.fData;
                    }
                }
            }
            return null;
        }
        else {
            Hashtable t;
            if (userData == null) {
                userData = new Hashtable();
                t = new Hashtable();
                userData.put(n, t);
            }
            else {
                t = (Hashtable) userData.get(n);
                if (t == null) {
                    t = new Hashtable();
                    userData.put(n, t);
                }
            }
            Object o = t.put(key, new UserDataRecord(data, handler));
            if (o != null) {
                UserDataRecord r = (UserDataRecord) o;
                return r.fData;
            }
            return null;
        }
    }

    /**
     * Retrieves the object associated to a key on a this node. The object 
     * must first have been set to this node by calling 
     * <code>setUserData</code> with the same key.
     * @param n The node the object is associated to.
     * @param key The key the object is associated to.
     * @return Returns the <code>DOMObject</code> associated to the given key 
     *   on this node, or <code>null</code> if there was none.
     * @since DOM Level 3
     */
    public Object getUserData(Node n, String key) {
        if (userData == null) {
            return null;
        }
        Hashtable t = (Hashtable) userData.get(n);
        if (t == null) {
            return null;
        }
        Object o = t.get(key);
        if (o != null) {
            UserDataRecord r = (UserDataRecord) o;
            return r.fData;
        }
        return null;
    }

    /**
     * Remove user data table for the given node.
     * @param n The node this operation applies to.
     * @return The removed table.
     */
    Hashtable removeUserDataTable(Node n) {
        if (userData == null) {
            return null;
        }
        return (Hashtable) userData.get(n);
    }

    /**
     * Set user data table for the given node.
     * @param n The node this operation applies to.
     * @param data The user data table.
     */
    void setUserDataTable(Node n, Hashtable data) {
        if (data != null) {
	    userData.put(n, data);
	}
    }

    /**
     * Call user data handlers when a node is deleted (finalized)
     * @param n The node this operation applies to.
     * @param c The copy node or null.
     * @param operation The operation - import, clone, or delete.
     */
    void callUserDataHandlers(Node n, Node c, short operation) {
        if (userData == null) {
            return;
        }
        Hashtable t = (Hashtable) userData.get(n);
        if (t == null || t.isEmpty()) {
            return;
        }
        Enumeration keys = t.keys();
        while (keys.hasMoreElements()) {
            String key = (String) keys.nextElement();
            UserDataRecord r = (UserDataRecord) t.get(key);
            if (r.fHandler != null) {
                r.fHandler.handle(operation, key, r.fData, n, c);
            }
        }
    }

    /**
     * Call user data handlers to let them know the nodes they are related to
     * are being deleted. The alternative would be to do that on Node but
     * because the nodes are used as the keys we have a reference to them that
     * prevents them from being gc'ed until the document is. At the same time,
     * doing it here has the advantage of avoiding a finalize() method on Node,
     * which would affect all nodes and not just the ones that have a user
     * data.
     */
    // Temporarily comment out this method, because
    // 1. It seems that finalizers are not guaranteed to be called, so the
    //    functionality is not implemented.
    // 2. It affects the performance greatly in multi-thread environment.
    // -SG
    /*public void finalize() {
        if (userData == null) {
            return;
        }
        Enumeration nodes = userData.keys();
        while (nodes.hasMoreElements()) {
            Object node = nodes.nextElement();
            Hashtable t = (Hashtable) userData.get(node);
            if (t != null && !t.isEmpty()) {
                Enumeration keys = t.keys();
                while (keys.hasMoreElements()) {
                    String key = (String) keys.nextElement();
                    UserDataRecord r = (UserDataRecord) t.get(key);
                    if (r.fHandler != null) {
                        r.fHandler.handle(UserDataHandler.NODE_DELETED,
                                          key, r.fData, null, null);
                    }
                }
            }
        }
    }*/
    
    protected final void checkNamespaceWF( String qname, int colon1,
		                                             int colon2) {
                                                        
		if (!errorChecking) {
			return;
		}
		// it is an error for NCName to have more than one ':'
		// check if it is valid QName [Namespace in XML production 6]
		if (colon1 == 0 || colon1 == qname.length() - 1 || colon2 != colon1) {
			String msg =
				DOMMessageFormatter.formatMessage(
					DOMMessageFormatter.DOM_DOMAIN,
					"NAMESPACE_ERR",
					null);
			throw new DOMException(DOMException.NAMESPACE_ERR, msg);
		}
	}
 	protected final void checkDOMNSErr(String prefix,
		                                 String namespace) {
		if (errorChecking) {
			if (namespace == null) {
				String msg =
					DOMMessageFormatter.formatMessage(
						DOMMessageFormatter.DOM_DOMAIN,
						"NAMESPACE_ERR",
						null);
				throw new DOMException(DOMException.NAMESPACE_ERR, msg);
			}
			else if (prefix.equals("xml")
					&& !namespace.equals(NamespaceContext.XML_URI)) {
				String msg =
					DOMMessageFormatter.formatMessage(
						DOMMessageFormatter.DOM_DOMAIN,
						"NAMESPACE_ERR",
						null);
				throw new DOMException(DOMException.NAMESPACE_ERR, msg);
			}
			else if (
				prefix.equals("xmlns")
					&& !namespace.equals(NamespaceContext.XMLNS_URI)
					|| (!prefix.equals("xmlns")
						&& namespace.equals(NamespaceContext.XMLNS_URI))) {
				String msg =
					DOMMessageFormatter.formatMessage(
						DOMMessageFormatter.DOM_DOMAIN,
						"NAMESPACE_ERR",
						null);
				throw new DOMException(DOMException.NAMESPACE_ERR, msg);
			}
		}
	}

    /**
     * 
     * @param n
     * @param data
     */
     protected final void checkQName(String prefix, 
                                       String local){
        if (!errorChecking) {
            return;
        }
        int length;
        if (prefix != null) {
            length=prefix.length();
			// check that prefix is NCName
            if (!XMLChar.isNCNameStart(prefix.charAt(0))) {
                String msg =
                    DOMMessageFormatter.formatMessage(
                        DOMMessageFormatter.DOM_DOMAIN,
                        "INVALID_CHARACTER_ERR",
                        null);
                throw new DOMException(DOMException.INVALID_CHARACTER_ERR, msg);
            }
            for (int i = 1; i < length; i++) {
                if (!XMLChar.isNCName(prefix.charAt(i))) {
                    String msg =
                        DOMMessageFormatter.formatMessage(
                            DOMMessageFormatter.DOM_DOMAIN,
                            "INVALID_CHARACTER_ERR",
                            null);
                    throw new DOMException(
                        DOMException.INVALID_CHARACTER_ERR,
                        msg);
                }
            }            

		} 
        length = local.length();
        // check local part 
        if (!XMLChar.isNCNameStart(local.charAt(0))) {
            // REVISIT: add qname parameter to the message
            String msg =
                DOMMessageFormatter.formatMessage(
                    DOMMessageFormatter.DOM_DOMAIN,
                    "INVALID_CHARACTER_ERR",
                    null);
            throw new DOMException(DOMException.INVALID_CHARACTER_ERR, msg);
        }
        for (int i = 1; i < length; i++) {
            if (!XMLChar.isNCName(local.charAt(i))) {
                String msg =
                    DOMMessageFormatter.formatMessage(
                        DOMMessageFormatter.DOM_DOMAIN,
                        "INVALID_CHARACTER_ERR",
                        null);
                throw new DOMException(DOMException.INVALID_CHARACTER_ERR, msg);
            }
        }           
    }

    boolean isNormalizeDocRequired (){        
        // REVISIT: Implement to optimize when normalization 
        // is required 
        return true;
    }
    
    /**
     * NON-DOM: kept for backward compatibility
     * Store user data related to a given node
     * This is a place where we could use weak references! Indeed, the node
     * here won't be GC'ed as long as some user data is attached to it, since
     * the userData table will have a reference to the node.
     */
    protected void setUserData(NodeImpl n, Object data) {
        setUserData(n, "XERCES1DOMUSERDATA", data, null);
    }

    /**
     * NON-DOM: kept for backward compatibility
     * Retreive user data related to a given node
     */
    protected Object getUserData(NodeImpl n) {
        return getUserData(n, "XERCES1DOMUSERDATA");
    }


    // Event related methods overidden in subclass

    protected void addEventListener(NodeImpl node, String type,
                                    EventListener listener,
                                    boolean useCapture) {
        // does nothing by default - overidden in subclass
    }

    protected void removeEventListener(NodeImpl node, String type,
                                       EventListener listener,
                                       boolean useCapture) {
        // does nothing by default - overidden in subclass
    }

    protected void copyEventListeners(NodeImpl src, NodeImpl tgt) {
        // does nothing by default - overidden in subclass
    }

    protected boolean dispatchEvent(NodeImpl node, Event event) {
        // does nothing by default - overidden in subclass
        return false;
    }

    // Notification methods overidden in subclasses

    /**
     * A method to be called when some text was changed in a text node,
     * so that live objects can be notified.
     */
    void replacedText(NodeImpl node) {
    }

    /**
     * A method to be called when some text was deleted from a text node,
     * so that live objects can be notified.
     */
    void deletedText(NodeImpl node, int offset, int count) {
    }

    /**
     * A method to be called when some text was inserted into a text node,
     * so that live objects can be notified.
     */
    void insertedText(NodeImpl node, int offset, int count) {
    }

    /**
     * A method to be called when a character data node has been modified
     */
    void modifyingCharacterData(NodeImpl node) {
    }

    /**
     * A method to be called when a character data node has been modified
     */
    void modifiedCharacterData(NodeImpl node, String oldvalue, String value) {
    }

    /**
     * A method to be called when a node is about to be inserted in the tree.
     */
    void insertingNode(NodeImpl node, boolean replace) {
    }

    /**
     * A method to be called when a node has been inserted in the tree.
     */
    void insertedNode(NodeImpl node, NodeImpl newInternal, boolean replace) {
    }

    /**
     * A method to be called when a node is about to be removed from the tree.
     */
    void removingNode(NodeImpl node, NodeImpl oldChild, boolean replace) {
    }

    /**
     * A method to be called when a node has been removed from the tree.
     */
    void removedNode(NodeImpl node, boolean replace) {
    }

    /**
     * A method to be called when a node is about to be replaced in the tree.
     */
    void replacingNode(NodeImpl node) {
    }

    /**
     * A method to be called when a node has been replaced in the tree.
     */
    void replacedNode(NodeImpl node) {
    }

    /**
     * A method to be called when an attribute value has been modified
     */
    void modifiedAttrValue(AttrImpl attr, String oldvalue) {
    }

    /**
     * A method to be called when an attribute node has been set
     */
    void setAttrNode(AttrImpl attr, AttrImpl previous) {
    }

    /**
     * A method to be called when an attribute node has been removed
     */
    void removedAttrNode(AttrImpl attr, NodeImpl oldOwner, String name) {
    }

    /**
     * A method to be called when an attribute node has been renamed
     */
    void renamedAttrNode(Attr oldAt, Attr newAt) {
    }

    /**
     * A method to be called when an element has been renamed
     */
    void renamedElement(Element oldEl, Element newEl) {
    }

} // class CoreDocumentImpl
