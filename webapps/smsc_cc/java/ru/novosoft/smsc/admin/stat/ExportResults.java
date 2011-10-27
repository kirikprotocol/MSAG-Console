package ru.novosoft.smsc.admin.stat;

/**
 * author: Aleksandr Khalitov
 */
public class ExportResults {

  public static class Pair {
    long records = 0;
    long errors = 0;

    public long getRecords() {
      return records;
    }

    public long getErrors() {
      return errors;
    }
  }

  final Pair total = new Pair();
  final Pair smes = new Pair();
  final Pair routes = new Pair();

  public Pair getTotal() {
    return total;
  }

  public Pair getSmes() {
    return smes;
  }

  public Pair getRoutes() {
    return routes;
  }
}
