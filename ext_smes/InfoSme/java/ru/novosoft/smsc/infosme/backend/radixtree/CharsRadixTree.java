package ru.novosoft.smsc.infosme.backend.radixtree;

import java.util.Collection;
import java.util.HashSet;

/**
 * Date: August 10, 2007
 * @author artem
 */
public class CharsRadixTree {
  
  private static int vertexCount = 0;
  
  private final CharArraysComparator insertComparator;
  private final CharArraysComparator searchComparator;
  protected final Vertex start = new Vertex();
  
  public CharsRadixTree(CharArraysComparator insertComparator, CharArraysComparator searchComparator) {
    this.insertComparator = insertComparator;
    this.searchComparator = searchComparator;
  }
  
  private static boolean isEmpty(char[] chars) {
    return chars == null || chars.length == 0;
  }
  
  public void add(final char[] chars, final Object value) {
    if (value == null)
      return;
    add(start, chars, value, new CharArraysCompareResult());
  }
  
  private void add(final Vertex currentVertex, final char[] chars, final Object value, CharArraysCompareResult compareResult) {
    if (isEmpty(chars)) {
      currentVertex.value = value;
      return;
    }
    
    for (int i=0; i<currentVertex.edges.length; i++) {
      
      if (currentVertex.edges[i] == null)
        break;
      
      // Search common part in two strings
      insertComparator.compare(currentVertex.edges[i].chars, chars, compareResult);
      
      if (!isEmpty(compareResult.commonPrefix)) {
        
        if (isEmpty(compareResult.syffix1) && (!isEmpty(compareResult.syffix2) || currentVertex.edges[i].vertex.value == null || currentVertex.edges[i].vertex.value == value)) {
          add(currentVertex.edges[i].vertex, compareResult.syffix2, value, compareResult);
          return;
        }
        
        if (!isEmpty(compareResult.syffix1)) {
          splitEdge(currentVertex.edges[i], compareResult, value);
          return;
        }
      }
    }
    
    createVertexAndEdge(chars, value, currentVertex);
  }
  
  public Object getValue(final char[] key) {
    return getValue(start, key, new CharArraysCompareResult());
  }
  
  private Object getValue(Vertex currentVertex, char[] key, CharArraysCompareResult compareResult) {
    if (isEmpty(key))
      return currentVertex.value;
    
    Object value;
    for (int i=0; i<currentVertex.edges.length; i++) {
      if (currentVertex.edges[i] == null)
        break;
      
      // Search common part in two strings
      searchComparator.compare(currentVertex.edges[i].chars, key, compareResult);
      if (isEmpty(compareResult.syffix1)) {
        value = getValue(currentVertex.edges[i].vertex, compareResult.syffix2, compareResult);
        if (value != null)
          return value;
      }
    }
    
    return null;
  }
  
  public Collection getValues(final char[] key) {
    final Collection result = new HashSet(10);
    getValues(start, key, new CharArraysCompareResult(), result);
    return result;
  }
  
  private void getValues(Vertex currentVertex, char[] key, CharArraysCompareResult compareResult, Collection values) {
    if (isEmpty(key)) {
      values.add(currentVertex.value);
      return;
    }
    
    for (int i=0; i<currentVertex.edges.length; i++) {
      if (currentVertex.edges[i] == null)
        break;
      
      // Search common part in two strings
      searchComparator.compare(currentVertex.edges[i].chars, key, compareResult);
      if (isEmpty(compareResult.syffix1)) 
        getValues(currentVertex.edges[i].vertex, compareResult.syffix2, compareResult, values);
    }
  }
  
  public int size() {
    final HashSet set = new HashSet(10);
    size(start, set);
    return set.size();
  }
  
  private static void size(Vertex currentVertex, HashSet set) {
    set.add(currentVertex);
    for (int i=0; i<currentVertex.edges.length; i++) {
      if (currentVertex.edges[i] == null)
        break;
      size(currentVertex.edges[i].vertex, set);
    }
  }
  
  private void createVertexAndEdge(char[] chars, Object value, Vertex parent) {
    final Vertex newVertex = new Vertex();
    newVertex.value = value;
    final Edge edge = new Edge();
    edge.chars = chars;
    edge.vertex = newVertex;
    parent.addEdge(edge);
  }

  private void splitEdge(Edge edge, CharArraysCompareResult compareResult, Object value) {
    final Edge edge1 = new Edge();
    edge1.chars = compareResult.syffix1;
    edge1.vertex = edge.vertex;

    final Vertex vertex1 = new Vertex();
    vertex1.addEdge(edge1);
    edge.vertex = vertex1;
    edge.chars = compareResult.commonPrefix;

    if (!isEmpty(compareResult.syffix2))
      createVertexAndEdge(compareResult.syffix2, value, vertex1);
    else
      vertex1.value = value;
  }





  protected static class Vertex {
    int edgesPos = 0;
    Edge[] edges = new Edge[10];
    final int id = vertexCount++;
      
    private Object value = null;
    
    boolean isFinal() {
      return value != null;
    }
    
    private void addEdge(Edge edge) {
      if (edgesPos == edges.length) {
        Edge[] newEdges = new Edge[edges.length + 10];
        System.arraycopy(edges, 0, newEdges, 0, edges.length);
        edges = newEdges;
      }
      edges[edgesPos++] = edge;
    }
    
  }





  protected static class Edge {
    char[] chars;
    Vertex vertex;
  }
  
}
