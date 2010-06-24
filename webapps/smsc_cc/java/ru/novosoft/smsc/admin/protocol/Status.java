package ru.novosoft.smsc.admin.protocol;


class Status {

  Status(String name, byte status) {
    this.status = 0;
    this.name = "Undefined";
    this.status = status;
    this.name = name;
  }

  byte status = 0;
  String name = null;
}
