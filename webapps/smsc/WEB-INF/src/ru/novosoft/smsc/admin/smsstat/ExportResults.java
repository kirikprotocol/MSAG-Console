package ru.novosoft.smsc.admin.smsstat;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 06.05.2005
 * Time: 18:25:18
 * To change this template use File | Settings | File Templates.
 */
public class ExportResults
{
    public class Pair {
        public long records = 0;
        public long errors  = 0;
    };
    
    public Pair total = new Pair();
    public Pair smes = new Pair();
    public Pair routes = new Pair();
}
