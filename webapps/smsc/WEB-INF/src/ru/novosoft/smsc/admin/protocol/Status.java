// Decompiled by Jad v1.5.5.3. Copyright 1997-98 Pavel Kouznetsov.
// Jad home page:      http://web.unicom.com.cy/~kpd/jad.html
// Decompiler options: packimports(3) printdflt
// Source File Name:   Response.java

package ru.novosoft.smsc.admin.protocol;


class Status
{

  Status(String name, byte status)
  {
    this.status = 0;
    this.name = "Undefined";
    this.status = status;
    this.name = name;
  }

  byte status = 0;
  String name = null;
}
