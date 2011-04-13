package ru.sibinco.sponsored.stats.backend.datasource;

import ru.sibinco.sponsored.stats.backend.StatisticsException;

import java.io.OutputStream;
import java.nio.channels.WritableByteChannel;
import java.util.ArrayList;

/**
 * @author Aleksandr Khalitov
 */
interface StatsFile {
  public String getName();
  public void addStat(DeliveryStat stat) throws StatisticsException;
  public ArrayList getRecords(int start, int count) throws StatisticsException;
  public void transferTo(DeliveryStatsQuery query, OutputStream target) throws StatisticsException;
  public void transferTo(WritableByteChannel target) throws StatisticsException;
  public void compress(DeliveryStatsQuery query) throws StatisticsException;
  public void close() throws StatisticsException;
}
