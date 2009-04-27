package ru.novosoft.smsc.infosme.backend.tables.messages;

/**
 * User: artem
 * Date: 21.04.2009
 */
public interface MessageVisitor {

  public boolean visit(MessageDataItem msg);
}
