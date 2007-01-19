package ru.novosoft.smsc.jsp.util.helper.dynamictable;

/**
 * User: artem
 * Date: 16.01.2007
 */

public abstract class Column {

  private final String name;
  private final String uid;
  private final int width;
  private final Type type;

  protected Column(Type type, String name, String uid, int width) {
    this.name = name;
    this.uid = uid;
    this.width = width;
    this.type = type;
  }

  public String getName() {
    return name;
  }

  public String getUid() {
    return uid;
  }

  public int getWidth() {
    return width;
  }

  public abstract Object getValue(String valueId) throws IncorrectValueException; 

  Type getType() {
    return type;
  }

  protected static class Type {

    public static final Type TEXT_COLUMN = new Type(1);
    public static final Type ROW_CONTROL_BUTTON_COLUMN = new Type(2);
    public static final Type SELECT_COLUMN = new Type(3);

    private final int type;

    private Type(int type) {
      this.type = type;
    }

    public int getType() {
      return type;
    }
  }
}
