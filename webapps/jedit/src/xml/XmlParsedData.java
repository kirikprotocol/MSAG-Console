/*
 * XmlParsedData.java
 * :tabSize=8:indentSize=8:noTabs=false:
 * :folding=explicit:collapseFolds=1:
 *
 * Copyright (C) 2003 Slava Pestov
 *
 * The XML plugin is licensed under the GNU General Public License, with
 * the following exception:
 *
 * "Permission is granted to link this code with software released under
 * the Apache license version 1.1, for example used by the Xerces XML
 * parser package."
 */

package xml;

//{{{ Imports
import javax.swing.tree.TreeModel;
import java.util.*;
import org.gjt.sp.jedit.Buffer;
import org.gjt.sp.jedit.EditPane;
import org.gjt.sp.util.Log;
import sidekick.SideKickParsedData;
import xml.completion.*;
import xml.parser.*;
//}}}

/**
 * Encapsulates the results of parsing a buffer, either using Xerces or the
 * Swing HTML parser.
 */
public class XmlParsedData extends SideKickParsedData
{
 public boolean html;
 public Map mappings;
 public List ids;

 //{{{ XmlParsedData constructor
 public XmlParsedData(String fileName, boolean html)
 {
  super(fileName);
  this.html = html;
  mappings = new HashMap();
  ids = new ArrayList();
 } //}}}

 //{{{ getNoNamespaceCompletionInfo() method
 public CompletionInfo getNoNamespaceCompletionInfo()
 {
  CompletionInfo info = (CompletionInfo)mappings.get("");
  if(info == null)
  {
   info = new CompletionInfo();
   System.out.println("xml.parser.XmlParserData getNoNamespaceCompletionInfo() mappings.put for empty string line 54 info= "+info);
      mappings.put("",info);
  }

  return info;
 } //}}}

 //{{{ getElementDecl() method
 public ElementDecl getElementDecl(String name)
 {
  if(html)
   name = name.toLowerCase();

  String prefix = getElementNamePrefix(name);
  CompletionInfo info = (CompletionInfo)mappings.get(prefix);

  if(info == null)
   return null;
  else
  {
   String lName;
   int prefixLen = prefix.length();
   if(prefixLen == 0)
    lName = name;
   else
    lName = name.substring(prefixLen + 1);

   ElementDecl decl = (ElementDecl)info.elementHash.get(lName);
   if(decl == null)
    return null;
   else
    return decl.withPrefix(prefix);
  }
 } //}}}

 //{{{ getAllowedElements() method
 public List getAllowedElements(Buffer buffer, int pos)
 {
  System.out.println("xml.parser.XmlParserData getAllowedElements in complete line 131");
    Log.log(Log.ERROR,this,"xml.parser.XmlParserData parse line 131" );
    TagParser.Tag parentTag = TagParser.findLastOpenTag(
   buffer.getText(0,pos),pos,this);

  ArrayList returnValue = new ArrayList();

  if(parentTag == null)
  {
   // add everything
   Iterator iter = mappings.keySet().iterator();
   while(iter.hasNext())
   {
    String prefix = (String)iter.next();
    CompletionInfo info = (CompletionInfo)mappings.get(prefix);
        System.out.println("xml.parser.XmlParserData getAllowedElements if(parentTag == null) line 105 info= "+info);
    info.getAllElements(prefix,returnValue);
   }
  }
  else
  {
   System.out.println("xml.parser.XmlParserData getAllowedElements if(parentTag != null) line 111");
      String parentPrefix = getElementNamePrefix(parentTag.tag);
   ElementDecl parentDecl = getElementDecl(parentTag.tag);
   if(parentDecl != null) {
     System.out.println("xml.parser.XmlParserData getAllowedElements if(parentTag != null) line 115 ");
     returnValue.addAll(parentDecl.getChildElements(parentPrefix));
      }
   // add everything but the parent's prefix now
   Iterator iter = mappings.keySet().iterator();
   while(iter.hasNext())
   {
    String prefix = (String)iter.next();
    if(!prefix.equals(parentPrefix))
    {
     CompletionInfo info = (CompletionInfo)mappings.get(prefix);
     System.out.println("xml.parser.XmlParserData getAllowedElements if(parentTag != null) line 126 info= "+info);
          info.getAllElements(prefix,returnValue);
    }
   }
  }

  Collections.sort(returnValue,new ElementDecl.Compare());
  System.out.println("xml.parser.XmlParserData getAllowedElements in complete line 131 returnValue= "+returnValue.toString());
    return returnValue;
 } //}}}

 //{{{ Private members

 //{{{ getElementPrefix() method
 private static String getElementNamePrefix(String name)
 {
  int index = name.indexOf(':');
  if(index == -1)
   return "";
  else
   return name.substring(0,index);
 } //}}}

 //}}}
}
