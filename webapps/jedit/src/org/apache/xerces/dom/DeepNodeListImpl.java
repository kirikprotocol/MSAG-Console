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

import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import java.util.Vector;

/**
 * This class implements the DOM's NodeList behavior for
 * Element.getElementsByTagName()
 * <P>
 * The DOM describes NodeList as follows:
 * <P>
 * 1) It may represent EITHER nodes scattered through a subtree (when
 * returned by Element.getElementsByTagName), or just the immediate
 * children (when returned by Node.getChildNodes). The latter is easy,
 * but the former (which this class addresses) is more challenging.
 * <P>
 * 2) Its behavior is "live" -- that is, it always reflects the
 * current state of the document tree. To put it another way, the
 * NodeLists obtained before and after a series of insertions and
 * deletions are effectively identical (as far as the user is
 * concerned, the former has been dynamically updated as the changes
 * have been made).
 * <P>
 * 3) Its API accesses individual nodes via an integer index, with the
 * listed nodes numbered sequentially in the order that they were
 * found during a preorder depth-first left-to-right search of the tree.
 * (Of course in the case of getChildNodes, depth is not involved.) As
 * nodes are inserted or deleted in the tree, and hence the NodeList,
 * the numbering of nodes that follow them in the NodeList will
 * change.
 * <P>
 * It is rather painful to support the latter two in the
 * getElementsByTagName case. The current solution is for Nodes to
 * maintain a change count (eventually that may be a Digest instead),
 * which the NodeList tracks and uses to invalidate itself.
 * <P>
 * Unfortunately, this does _not_ respond efficiently in the case that
 * the dynamic behavior was supposed to address: scanning a tree while
 * it is being extended. That requires knowing which subtrees have
 * changed, which can become an arbitrarily complex problem.
 * <P>
 * We save some work by filling the vector only as we access the
 * item()s... but I suspect the same users who demanded index-based
 * access will also start by doing a getLength() to control their loop,
 * blowing this optimization out of the water.
 * <P>
 * NOTE: Level 2 of the DOM will probably _not_ use NodeList for its
 * extended search mechanisms, partly for the reasons just discussed.
 *
 * @version $Id$
 * @since  PR-DOM-Level-1-19980818.
 */
public class DeepNodeListImpl 
    implements NodeList {

    //
    // Data
    //

    protected NodeImpl rootNode; // Where the search started
    protected String tagName;   // Or "*" to mean all-tags-acceptable
    protected int changes=0;
    protected Vector nodes;
    
    protected String nsName;
    protected boolean enableNS = false;

    //
    // Constructors
    //

    /** Constructor. */
    public DeepNodeListImpl(NodeImpl rootNode, String tagName) {
        this.rootNode = rootNode;
        this.tagName  = tagName;
        nodes = new Vector();
    }  

    /** Constructor for Namespace support. */
    public DeepNodeListImpl(NodeImpl rootNode,
                            String nsName, String tagName) {
        this(rootNode, tagName);
        this.nsName = (nsName != null && !nsName.equals("")) ? nsName : null;
        enableNS = true;
    }
    
    //
    // NodeList methods
    //

    /** Returns the length of the node list. */
    public int getLength() {
        // Preload all matching elements. (Stops when we run out of subtree!)
        item(java.lang.Integer.MAX_VALUE);
        return nodes.size();
    }  

    /** Returns the node at the specified index. */
    public Node item(int index) {
    	Node thisNode;

        // Tree changed. Do it all from scratch!
    	if(rootNode.changes() != changes) {
            nodes   = new Vector();     
            changes = rootNode.changes();
    	}
    
        // In the cache
    	if (index < nodes.size())      
    	    return (Node)nodes.elementAt(index);
    
        // Not yet seen
    	else {
    
            // Pick up where we left off (Which may be the beginning)
    		if (nodes.size() == 0)     
    		    thisNode = rootNode;
    		else
    		    thisNode=(NodeImpl)(nodes.lastElement());
    
    		// Add nodes up to the one we're looking for
    		while(thisNode != null && index >= nodes.size()) {
    			thisNode=nextMatchingElementAfter(thisNode);
    			if (thisNode != null)
    			    nodes.addElement(thisNode);
    		    }

            // Either what we want, or null (not avail.)
		    return thisNode;           
	    }

    } // item(int):Node

    //
    // Protected methods (might be overridden by an extending DOM)
    //

    /** 
     * Iterative tree-walker. When you have a Parent link, there's often no
     * need to resort to recursion. NOTE THAT only Element nodes are matched
     * since we're specifically supporting getElementsByTagName().
     */
    protected Node nextMatchingElementAfter(Node current) {

	    Node next;
	    while (current != null) {
		    // Look down to first child.
		    if (current.hasChildNodes()) {
			    current = (current.getFirstChild());
		    }

		    // Look right to sibling (but not from root!)
		    else if (current != rootNode && null != (next = current.getNextSibling())) {
				current = next;
			}

			// Look up and right (but not past root!)
			else {
				next = null;
				for (; current != rootNode; // Stop when we return to starting point
					current = current.getParentNode()) {

					next = current.getNextSibling();
					if (next != null)
						break;
				}
				current = next;
			}

			// Have we found an Element with the right tagName?
			// ("*" matches anything.)
		    if (current != rootNode 
		        && current != null
		        && current.getNodeType() ==  Node.ELEMENT_NODE) {
			if (!enableNS) {
			    if (tagName.equals("*") ||
				((ElementImpl) current).getTagName().equals(tagName))
			    {
				return current;
			    }
			} else {
			    // DOM2: Namespace logic. 
			    if (tagName.equals("*")) {
				if (nsName != null && nsName.equals("*")) {
				    return current;
				} else {
				    ElementImpl el = (ElementImpl) current;
				    if ((nsName == null
					 && el.getNamespaceURI() == null)
					|| (nsName != null
					    && nsName.equals(el.getNamespaceURI())))
				    {
					return current;
				    }
				}
			    } else {
				ElementImpl el = (ElementImpl) current;
				if (el.getLocalName() != null
				    && el.getLocalName().equals(tagName)) {
				    if (nsName != null && nsName.equals("*")) {
					return current;
				    } else {
					if ((nsName == null
					     && el.getNamespaceURI() == null)
					    || (nsName != null &&
						nsName.equals(el.getNamespaceURI())))
					{
					    return current;
					}
				    }
				}
			    }
			}
		    }

		// Otherwise continue walking the tree
	    }

	    // Fell out of tree-walk; no more instances found
	    return null;

    } // nextMatchingElementAfter(int):Node

} // class DeepNodeListImpl
