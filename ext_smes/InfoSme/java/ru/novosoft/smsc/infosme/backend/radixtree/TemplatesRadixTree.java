package ru.novosoft.smsc.infosme.backend.radixtree;

/**
 * Date: August 13, 2007
 * @author artem
 */
public class TemplatesRadixTree extends StringsRadixTree {
  
  /** Creates a new instance of TemplatesRadixTree */
  public TemplatesRadixTree() {
    super(new StringsComparator(), new TemplatesComparator());
  }
  
}
