package ru.sibinco.smsx.stats.backend.datasource;

import org.apache.log4j.Category;
import ru.sibinco.smsx.stats.backend.StatisticsException;

import java.io.IOException;
import java.io.Writer;

/**
 * @author Aleksandr Khalitov
 */
public class SmsxConverter {  // todo Может перенести этот класс в пакет ru.sibinco.smsx.stats.backend? Вроде используется только оттуда.

  private static final Category logger = Category.getInstance(SmsxConverter.class);

  private static final String comma = ",";

  private static final String one = "1";

  private static final String zero = "0";

  private static String lineSeparator = System.getProperty("line.separator");

  public static void write(Writer writer, SmsxUsers users) throws StatisticsException {
    try{
      writer.write(users.getRegion());
      writer.write(comma);
      writer.write(Integer.toString(users.getServiceId()));
      writer.write(comma);
      writer.write(Integer.toString(users.getCount()));
      writer.write(lineSeparator);
    }catch (IOException e){
      throw new StatisticsException(e);
    }
  }

  public static SmsxUsers parseSmsxUsers(String line) throws StatisticsException {
    try{
      String[] ss = line.split(comma, 3);
      return new SmsxUsers(Integer.parseInt(ss[1]), ss[0], Integer.parseInt(ss[2]));
    }catch (IndexOutOfBoundsException e){
      logger.error(e,e);
      throw new StatisticsException("Illegal data format. Line: "+line);
    }catch (NumberFormatException e){
      logger.error(e,e);
      throw new StatisticsException("Illegal data format. Line: "+line);
    }
  }

  public static void write(Writer writer, Traffic traffic) throws StatisticsException{
    try{
      writer.write(traffic.getRegion());
      writer.write(comma);
      writer.write(Integer.toString(traffic.getServiceId()));
      writer.write(comma);
      writer.write(traffic.isMsc() ? one : zero);
      writer.write(comma);
      writer.write(Integer.toString(traffic.getCount()));
      writer.write(lineSeparator);
    }catch (IOException e){
      throw new StatisticsException(e);
    }
  }

  public static Traffic parseTraffic(String line) throws StatisticsException {
    try{
      String[] ss = line.split(comma, 4);
      return new Traffic(ss[0], Integer.parseInt(ss[1]), ss[2].equals(one), Integer.parseInt(ss[3]));
    }catch (IndexOutOfBoundsException e){
      logger.error(e,e);
      throw new StatisticsException("Illegal data format. Line: "+line);
    }catch (NumberFormatException e){
      logger.error(e,e);
      throw new StatisticsException("Illegal data format. Line: "+line);
    }
  }

  public static void write(Writer writer, WebRegion webRegion) throws StatisticsException{
    try{
      writer.write(webRegion.getRegion());
      writer.write(comma);
      writer.write(webRegion.isMsc() ? one : zero);
      writer.write(comma);
      writer.write(Integer.toString(webRegion.getSrcCount()));
      writer.write(comma);
      writer.write(Integer.toString(webRegion.getDstCount()));
      writer.write(lineSeparator);
    }catch (IOException e){
      throw new StatisticsException(e);
    }
  }

  public static WebRegion parseWebRegion(String line) throws StatisticsException{
    try{
      String[] ss = line.split(comma, 4);
      return new WebRegion(ss[0], ss[1].equals(one), Integer.parseInt(ss[2]), Integer.parseInt(ss[3]));
    }catch (IndexOutOfBoundsException e){
      logger.error(e,e);
      throw new StatisticsException("Illegal data format. Line: "+line);
    }catch (NumberFormatException e){
      logger.error(e,e);
      throw new StatisticsException("Illegal data format. Line: "+line);
    }
  }


  public static void write(Writer writer, WebDaily webDaily) throws StatisticsException{
    try{
      writer.write(webDaily.getDate());
      writer.write(comma);
      writer.write(webDaily.getRegion());
      writer.write(comma);
      writer.write(webDaily.isMsc() ? one : zero);
      writer.write(comma);
      writer.write(Integer.toString(webDaily.getCount()));
      writer.write(lineSeparator);
    }catch (IOException e){
      throw new StatisticsException(e);
    }
  }

  public static WebDaily parseWebDaily(String line) throws StatisticsException {
    try{
      String[] ss = line.split(comma, 4);
      return new WebDaily(ss[0], ss[1], ss[2].equals(one), Integer.parseInt(ss[3]));
    }catch (IndexOutOfBoundsException e){
      logger.error(e,e);
      throw new StatisticsException("Illegal data format. Line: "+line);
    }catch (NumberFormatException e){
      logger.error(e,e);
      throw new StatisticsException("Illegal data format. Line: "+line);
    }
  }

}
