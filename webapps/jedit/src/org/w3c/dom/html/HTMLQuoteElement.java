/*
 * Copyright (c) 2000 World Wide Web Consortium,
 * (Massachusetts Institute of Technology, Institut National de
 * Recherche en Informatique et en Automatique, Keio University). All
 * Rights Reserved. This program is distributed under the W3C's Software
 * Intellectual Property License. This program is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See W3C License http://www.w3.org/Consortium/Legal/ for more
 * details.
 */

package org.w3c.dom.html;

/**
 *  For the <code>Q</code> and <code>BLOCKQUOTE</code> elements. See the  Q 
 * element definition in HTML 4.0.
 */
public interface HTMLQuoteElement extends HTMLElement {
    /**
     *  A URI designating a source document or message. See the  cite 
     * attribute definition in HTML 4.0.
     */
    public String getCite();
    public void setCite(String cite);

}

