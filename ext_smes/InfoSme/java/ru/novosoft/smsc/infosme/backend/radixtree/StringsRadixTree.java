package ru.novosoft.smsc.infosme.backend.radixtree;

import java.util.Collection;

/**
 * Date: August 10, 2007
 * @author artem
 */
public class StringsRadixTree extends CharsRadixTree {
  
  public StringsRadixTree() {
    super(new StringsComparator(), new StringsComparator());
  }
  
  protected StringsRadixTree(CharArraysComparator insertComparator, CharArraysComparator searchComparator) {
    super(insertComparator, searchComparator);
  }
  
  public void printTree() {
    printTree(start);
  }
  
  private static void printTree(Vertex vertex) {
    for (int i=0; i < vertex.edges.length; i++) {
      if (vertex.edges[i] == null)
        break;
      System.out.println(printVertex(vertex) + " -- " + printBytes(vertex.edges[i].chars) + " --> " + printVertex(vertex.edges[i].vertex));
      printTree(vertex.edges[i].vertex);
    }
  }
  
  private static String printBytes(char[] bytes) {
    final StringBuffer buffer = new StringBuffer(bytes.length);
    for (int i=0; i< bytes.length; i++)
      buffer.append((char)bytes[i]);
    return buffer.toString();
  }
  
  private static String printVertex(Vertex vertex) {
    return vertex.isFinal() ? "|" + vertex.id + '|' : String.valueOf(vertex.id);
  }
  
  public void add(final Collection strings, final Object value) {
    for (java.util.Iterator it = strings.iterator(); it.hasNext();) {
      String key = (String) it.next();
      add(key, value);
    }
  }
  
  public void add(final String word, final Object value) {
    final char[] chars = new char[word.length()];
    word.getChars(0, word.length(), chars, 0);
    add(chars, value);
  }
  
  public Object getValue(final String key) {
    final char[] chars = new char[key.length()];
    key.getChars(0, key.length(), chars, 0);
    return getValue(chars);
  }
  
  public Collection getValues(final String key) {
    final char[] chars = new char[key.length()];
    key.getChars(0, key.length(), chars, 0);
    return getValues(chars);
  }
  
}
