package mobi.eyeline.mcahdb.engine.event.ds.impl.file;

/**
 * User: artem
 * Date: 30.01.2009
 */
class Position {
  final String msisdn;
  final long pos;

  public Position(String msisdn, long pos) {
    this.msisdn = msisdn;
    this.pos = pos;
  }
}
