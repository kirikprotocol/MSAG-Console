// xparse.java - A simple command-line XML parser

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

package org.apache.xml.resolver.apps;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Vector;
import java.util.Date;
import java.util.GregorianCalendar;

import org.xml.sax.*;
import org.xml.sax.helpers.*;

import javax.xml.parsers.*;

import org.apache.xml.resolver.tools.ResolvingParser;
import org.apache.xml.resolver.Catalog;
import org.apache.xml.resolver.helpers.Debug;

/**
 * <p>A simple command-line XML parsing application.</p>
 * <p/>
 * <p>This class implements a simple command-line XML Parser. It's
 * just a little wrapper around the JAXP Parser with support for
 * catalogs.
 * </p>
 * <p/>
 * <p>Usage: xparse [options] document.xml</p>
 * <p/>
 * <p>Where:</p>
 * <p/>
 * <dl>
 * <dt><code>-c</code> <em>catalogfile</em></dt>
 * <dd>Load a particular catalog file.</dd>
 * <dt><code>-w</code></dt>
 * <dd>Perform a well-formed parse, not a validating parse.</dd>
 * <dt><code>-v</code> (the default)</dt>
 * <dd>Perform a validating parse.</dd>
 * <dt><code>-n</code></dt>
 * <dd>Perform a namespace-ignorant parse.</dd>
 * <dt><code>-N</code> (the default)</dt>
 * <dd>Perform a namespace-aware parse.</dd>
 * <dt><code>-d</code> <em>integer</em></dt>
 * <dd>Set the debug level. Warnings are shown if the debug level
 * is &gt; 2.</dd>
 * <dt><code>-E</code> <em>integer</em></dt>
 * <dd>Set the maximum number of errors to display.</dd>
 * </dl>
 * <p/>
 * <p>The process ends with error-level 1, if there errors.</p>
 *
 * @author Norman Walsh
 *         <a href="mailto:Norman.Walsh@Sun.COM">Norman.Walsh@Sun.COM</a>
 * @version 1.0
 * @see org.apache.xml.resolver.tools.ResolvingParser
 * @deprecated This interface has been replaced by the
 *             {@link org.apache.xml.resolver.tools.ResolvingXMLReader} for SAX2.
 */
public class xparse
{
  /**
   * The main entry point
   */
  public static void main(String[] args)
          throws FileNotFoundException, IOException
  {

    String xmlfile = null;
    int debug = 0;
    int maxErrs = 10;
    boolean nsAware = true;
    boolean validating = true;
    boolean showWarnings = (debug > 2);
    boolean showErrors = true;
    Vector catalogFiles = new Vector();

    for (int i = 0; i < args.length; i++) {
      if (args[i].equals("-c")) {
        ++i;
        catalogFiles.add(args[i]);
        continue;
      }

      if (args[i].equals("-w")) {
        validating = false;
        continue;
      }

      if (args[i].equals("-v")) {
        validating = true;
        continue;
      }

      if (args[i].equals("-n")) {
        nsAware = false;
        continue;
      }

      if (args[i].equals("-N")) {
        nsAware = true;
        continue;
      }

      if (args[i].equals("-d")) {
        ++i;
        String debugstr = args[i];
        try {
          debug = Integer.parseInt(debugstr);
          if (debug >= 0) {
            Debug.setDebug(debug);
            showWarnings = (debug > 2);
          }
        } catch (Exception e) {
          // nop
        }
        continue;
      }

      if (args[i].equals("-E")) {
        ++i;
        String errstr = args[i];
        try {
          int errs = Integer.parseInt(errstr);
          if (errs >= 0) {
            maxErrs = errs;
          }
        } catch (Exception e) {
          // nop
        }
        continue;
      }

      xmlfile = args[i];
    }

    if (xmlfile == null) {
      System.out.println("Usage: org.apache.xml.resolver.apps.xparse [opts] xmlfile");
      System.exit(1);
    }

    ResolvingParser.validating = validating;
    ResolvingParser.namespaceAware = nsAware;
    ResolvingParser reader = new ResolvingParser();
    Catalog catalog = reader.getCatalog();

    for (int count = 0; count < catalogFiles.size(); count++) {
      String file = (String) catalogFiles.elementAt(count);
      catalog.parseCatalog(file);
    }

    XParseError xpe = new XParseError(showErrors, showWarnings);
    xpe.setMaxMessages(maxErrs);
    reader.setErrorHandler(xpe);

    String parseType = validating ? "validating" : "well-formed";
    String nsType = nsAware ? "namespace-aware" : "namespace-ignorant";
    if (maxErrs > 0) {
      System.out.println("Attempting "
              + parseType
              + ", "
              + nsType
              + " parse");
    }

    Date startTime = new Date();

    try {
      reader.parse(xmlfile);
    } catch (SAXException sx) {
      System.out.println("SAX Exception: " + sx);
    } catch (Exception e) {
      e.printStackTrace();
    }

    Date endTime = new Date();

    long millisec = endTime.getTime() - startTime.getTime();
    long secs = 0;
    long mins = 0;
    long hours = 0;

    if (millisec > 1000) {
      secs = millisec / 1000;
      millisec = millisec % 1000;
    }

    if (secs > 60) {
      mins = secs / 60;
      secs = secs % 60;
    }

    if (mins > 60) {
      hours = mins / 60;
      mins = mins % 60;
    }

    if (maxErrs > 0) {
      System.out.print("Parse ");
      if (xpe.getFatalCount() > 0) {
        System.out.print("failed ");
      }
      else {
        System.out.print("succeeded ");
        System.out.print("(");
        if (hours > 0) {
          System.out.print(hours + ":");
        }
        if (hours > 0 || mins > 0) {
          System.out.print(mins + ":");
        }
        System.out.print(secs + "." + millisec);
        System.out.print(") ");
      }
      System.out.print("with ");

      int errCount = xpe.getErrorCount();
      int warnCount = xpe.getWarningCount();

      if (errCount > 0) {
        System.out.print(errCount + " error");
        System.out.print(errCount > 1 ? "s" : "");
        System.out.print(" and ");
      }
      else {
        System.out.print("no errors and ");
      }

      if (warnCount > 0) {
        System.out.print(warnCount + " warning");
        System.out.print(warnCount > 1 ? "s" : "");
        System.out.print(".");
      }
      else {
        System.out.print("no warnings.");
      }

      System.out.println("");
    }

    if (xpe.getErrorCount() > 0) {
      System.exit(1);
    }
  }
}


