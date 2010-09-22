/*
 * PropertyManager.java - Manages property files
 * :tabSize=8:indentSize=8:noTabs=false:
 * :folding=explicit:collapseFolds=1:
 *
 * Copyright (C) 2004 Slava Pestov
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

package org.gjt.sp.jedit;

import java.io.*;
import java.util.*;

class PropertyManager
{
 //{{{ getProperties() method
 Properties getProperties()
 {
  Properties total = new Properties();
  total.putAll(system);
  Iterator iter = plugins.iterator();
  while(iter.hasNext())
   total.putAll((Properties)iter.next());
  total.putAll(site);
  total.putAll(user);
  return total;
 } //}}}

 //{{{ loadSystemProps() method
 void loadSystemProps(InputStream in)
  throws IOException
 {
  loadProps(system,in);
 } //}}}

 //{{{ loadSiteProps() method
 void loadSiteProps(InputStream in)
  throws IOException
 {
  loadProps(site,in);
 } //}}}
  //{{{ loadSitePropsA() method
  void loadSitePropsA(LinkedList list)
  {
    loadPropsA(site,list);
  } //}}}

 //{{{ loadUserProps() method
 void loadUserProps(InputStream in)
  throws IOException
 {
  loadProps(user,in);
 } //}}}

  //{{{ loadUserPropsA() method
  void loadUserPropsA(LinkedList list)
  {
    loadPropsA(user,list);
  } //}}}

  //{{{ saveUserProps() method
 void saveUserProps(OutputStream out)
  throws IOException
 {
  user.store(out,"jEdit properties");
  out.close();
 } //}}}

 //{{{ loadPluginProps() method
 Properties loadPluginProps(InputStream in)
  throws IOException
 {
  Properties plugin = new Properties();
  loadProps(plugin,in);
  plugins.add(plugin);
  return plugin;
 } //}}}

 //{{{ addPluginProps() method
 void addPluginProps(Properties props)
 {
  plugins.add(props);
 } //}}}

 //{{{ removePluginProps() method
 void removePluginProps(Properties props)
 {
  plugins.remove(props);
 } //}}}

 //{{{ getProperty() method
 String getProperty(String name)
 {
  String value = user.getProperty(name);
  if(value != null)
   return value;
  else
   return getDefaultProperty(name);
 } //}}}

 //{{{ setProperty() method
 void setProperty(String name, String value)
 {
  String prop = getDefaultProperty(name);

  /* if value is null:
   * - if default is null, unset user prop
   * - else set user prop to ""
   * else
   * - if default equals value, ignore
   * - if default doesn't equal value, set user
   */
  if(value == null)
  {
   if(prop == null || prop.length() == 0)
    user.remove(name);
   else
    user.put(name,"");
  }
  else
  {
   if(value.equals(prop))
    user.remove(name);
   else
    user.put(name,value);
  }
 } //}}}

 //{{{ setTemporaryProperty() method
 public void setTemporaryProperty(String name, String value)
 {
  user.remove(name);
  system.put(name,value);
 } //}}}

 //{{{ unsetProperty() method
 void unsetProperty(String name)
 {
  if(getDefaultProperty(name) != null)
   user.put(name,"");
  else
   user.remove(name);
 } //}}}

 //{{{ resetProperty() method
 public void resetProperty(String name)
 {
  user.remove(name);
 } //}}}

 //{{{ Private members
 private Properties system = new Properties();
 private List plugins = new LinkedList();
 private Properties site = new Properties();
 private Properties user = new Properties();

 //{{{ getDefaultProperty() method
 private String getDefaultProperty(String name)
 {
  String value = site.getProperty(name);
  if(value != null)
   return value;

  Iterator iter = plugins.iterator();
  while(iter.hasNext())
  {
   value = ((Properties)iter.next()).getProperty(name);
   if(value != null)
    return value;
  }

  return system.getProperty(name);
 } //}}}

 //{{{ loadProps() method
 private void loadProps(Properties into, InputStream in)
  throws IOException
 {
  try
  {
   System.out.println("PropertyManager 195");
   into.load(in);
  }
  finally
  {
   in.close();
  }
 } //}}}

   //{{{ loadProps() method
 private void loadPropsA(Properties into, LinkedList list)

 {
   load(into,list);

 } //}}}
     /*
     * Returns true if the given line is a line that must
     * be appended to the next line
     */
    private boolean continueLine(String line) {
        int slashCount = 0;
        int index = line.length() - 1;
        while ((index >= 0) && (line.charAt(index--) == '\\'))
            slashCount++;
        return (slashCount % 2 == 1);
    }
  private static final String keyValueSeparators = "=: \t\r\n\f";

  private static final String strictKeyValueSeparators = "=:";

  //private static final String specialSaveChars = "=: \t\r\n\f#!";

  private static final String whiteSpaceChars = " \t\r\n\f";



  public synchronized void load(Properties into ,LinkedList list) {

      int i=0;
 while (true) {
            // Get next line
           if (i>list.size()-1) return;
            String line = (String) list.get(i); i=i+1;
           // if (line == null) return;

            if (line.length() > 0) {

                // Find start of key
                int len = line.length();
                int keyStart;
                for (keyStart=0; keyStart<len; keyStart++)
                    if (whiteSpaceChars.indexOf(line.charAt(keyStart)) == -1)
                        break;

                // Blank lines are ignored
                if (keyStart == len)
                    continue;

                // Continue lines that end in slashes if they are not comments
                char firstChar = line.charAt(keyStart);
                if ((firstChar != '#') && (firstChar != '!')) {
                    while (continueLine(line)) {
                      if (i>list.size()-1) return;
                      String nextLine = (String) list.get(i);i=i+1;
                        if (nextLine == null)
                            nextLine = "";
                        String loppedLine = line.substring(0, len-1);
                        // Advance beyond whitespace on new line
                        int startIndex;
                        for (startIndex=0; startIndex<nextLine.length(); startIndex++)
                            if (whiteSpaceChars.indexOf(nextLine.charAt(startIndex)) == -1)
                                break;
                        nextLine = nextLine.substring(startIndex,nextLine.length());
                        line = new String(loppedLine+nextLine);
                        len = line.length();
                    }

                    // Find separation between key and value
                    int separatorIndex;
                    for (separatorIndex=keyStart; separatorIndex<len; separatorIndex++) {
                        char currentChar = line.charAt(separatorIndex);
                        if (currentChar == '\\')
                            separatorIndex++;
                        else if (keyValueSeparators.indexOf(currentChar) != -1)
                            break;
                    }

                    // Skip over whitespace after key if any
                    int valueIndex;
                    for (valueIndex=separatorIndex; valueIndex<len; valueIndex++)
                        if (whiteSpaceChars.indexOf(line.charAt(valueIndex)) == -1)
                            break;

                    // Skip over one non whitespace key value separators if any
                    if (valueIndex < len)
                        if (strictKeyValueSeparators.indexOf(line.charAt(valueIndex)) != -1)
                            valueIndex++;

                    // Skip over white space after other separators if any
                    while (valueIndex < len) {
                        if (whiteSpaceChars.indexOf(line.charAt(valueIndex)) == -1)
                            break;
                        valueIndex++;
                    }
                    String key = line.substring(keyStart, separatorIndex);
                    String value = (separatorIndex < len) ? line.substring(valueIndex, len) : "";

                    // Convert then store key and value
                    key = loadConvert(key);
                    value = loadConvert(value);
                    into.setProperty(key, value);
                }
            }
 }
    }
  /*
   * Converts encoded &#92;uxxxx to unicode chars
   * and changes special saved chars to their original forms
   */
     private String loadConvert(String theString) {
         char aChar;
         int len = theString.length();
         StringBuffer outBuffer = new StringBuffer(len);

         for (int x=0; x<len; ) {
             aChar = theString.charAt(x++);
             if (aChar == '\\') {
                 aChar = theString.charAt(x++);
                 if (aChar == 'u') {
                     // Read the xxxx
                     int value=0;
         for (int i=0; i<4; i++) {
             aChar = theString.charAt(x++);
             switch (aChar) {
               case '0': case '1': case '2': case '3': case '4':
               case '5': case '6': case '7': case '8': case '9':
                  value = (value << 4) + aChar - '0';
            break;
         case 'a': case 'b': case 'c':
                           case 'd': case 'e': case 'f':
            value = (value << 4) + 10 + aChar - 'a';
            break;
         case 'A': case 'B': case 'C':
                           case 'D': case 'E': case 'F':
            value = (value << 4) + 10 + aChar - 'A';
            break;
         default:
                               throw new IllegalArgumentException(
                                            "Malformed \\uxxxx encoding.");
                         }
                     }
                     outBuffer.append((char)value);
                 } else {
                     if (aChar == 't') aChar = '\t';
                     else if (aChar == 'r') aChar = '\r';
                     else if (aChar == 'n') aChar = '\n';
                     else if (aChar == 'f') aChar = '\f';
                     outBuffer.append(aChar);
                 }
             } else
                 outBuffer.append(aChar);
         }
         return outBuffer.toString();
     }

   //}}}
}
