package ru.novosoft.smsc.admin.callable;

/*
 * Created by igork
 * Date: Mar 29, 2002
 * Time: 6:24:02 PM
 */


class Type {
  public boolean equals(Object obj) {
    if (obj != null && obj instanceof Type) {
      Type t = (Type) obj;
      return id == t.id;
    }
    return false;
  }

  static final public byte StringType = 1;
  static final public byte IntType = 2;
  static final public byte BooleanType = 3;
  static final public byte StringListType = 4;
  static final public byte UnknownType = 0;

  static final public Type[] Types = {
      new Type("undefined", UnknownType), // must be on 0 position
      new Type("string", StringType),
      new Type("int", IntType),
      new Type("bool", BooleanType),
      new Type("stringlist", StringListType)
  };

  protected String name;
  protected byte id;

  static public Type getInstance(String typeName) {
    for (int i = 1; i < Types.length; i++) {
      if (typeName.equals(Types[i].name))
        return Types[i];
    }
    return Types[0];
  }

  private Type(String name, byte id) {
    this.name = name;
    this.id = id;
  }

  public String getName() {
    return name;
  }

  public byte getId() {
    return id;
  }
}
