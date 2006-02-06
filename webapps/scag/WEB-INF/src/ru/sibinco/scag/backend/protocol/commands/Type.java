/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.protocol.commands;

/**
 * The <code>Type</code> class represents
 * <p><p/>
 * Date: 18.01.2006
 * Time: 16:32:21
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Type {

    public static final byte UNKNOWN_TYPE = 0;
    public static final byte STRING_TYPE = 1;
    public static final byte INT_TYPE = 2;
    public static final byte BOOLEAN_TYPE = 3;
    public static final byte STRING_LIST_TYPE = 4;

    protected String name;
    protected byte id;

    public Type(String name, byte id) {
        this.name = name;
        this.id = id;
    }

    public static final Type[] Types = {
            new Type("undefind", UNKNOWN_TYPE),
            new Type("string", STRING_TYPE),
            new Type("int", INT_TYPE),
            new Type("bool", BOOLEAN_TYPE),
            new Type("stringlist", STRING_LIST_TYPE)
    };


    public boolean equals(Object obj) {
        if (obj != null && obj instanceof Type) {
            Type type = (Type) obj;
            return id == type.id;
        }
        return false;
    }

    public static Type getInstance(String typeName) {
        for (int i = 0; i < Types.length; i++) {
            if (typeName.equals(Types[i].name))
                return Types[i];
        }
        return Types[UNKNOWN_TYPE];
    }

    public String getName() {
        return name;
    }

    public byte getId() {
        return id;
    }
}
