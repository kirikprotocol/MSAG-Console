package ru.sibinco.sponsored.stats.backend.datasource;

import org.apache.log4j.Category;
import ru.sibinco.sponsored.stats.backend.StatisticsException;

import java.io.IOException;
import java.io.Writer;

/**
 * @author Aleksandr Khalitov
 */
public class SponsoredConverter { //todo перенести в backend

  private static final Category logger = Category.getInstance(SponsoredConverter.class);

  private static final String comma = ",";
  private static final String sep = System.getProperty("line.separator");

  public static void write(Writer writer, String address, float bonus) throws StatisticsException {
    try{
      writer.write(address);
      writer.write(comma);
      writer.write(Float.toString(bonus));
      writer.write(sep);
    }catch (IOException e){
      throw new StatisticsException(e);
    }
  }

  public static SponsoredRecord parseRecord(String line) throws StatisticsException{
    String[] ss = line.split(comma, 2);
    try{
      return new SponsoredRecord(ss[0], Float.parseFloat(ss[1]));
    }catch (Exception e) {
      logger.error("Can't parse line '"+line+'\'');
      throw new StatisticsException(e);
    }
  }



}
