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
 *  Provides a caption for a <code>FIELDSET</code> grouping.  See the  LEGEND 
 * element definition in HTML 4.0.
 */
public interface HTMLLegendElement extends HTMLElement {
    /**
     *  Returns the <code>FORM</code> element containing this control. Returns 
     * <code>null</code> if this control is not within the context of a form. 
     */
    public HTMLFormElement getForm();

    /**
     *  A single character access key to give access to the form control. See 
     * the  accesskey attribute definition in HTML 4.0.
     */
    public String getAccessKey();
    public void setAccessKey(String accessKey);

    /**
     *  Text alignment relative to <code>FIELDSET</code> . See the  align 
     * attribute definition in HTML 4.0. This attribute is deprecated in HTML 
     * 4.0.
     */
    public String getAlign();
    public void setAlign(String align);

}

