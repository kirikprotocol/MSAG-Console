package com.eyeline.sponsored.ds.distribution.advert.impl.file.deliverystats;

import com.eyeline.sponsored.ds.distribution.advert.DeliveryStat;
import com.eyeline.sponsored.ds.distribution.advert.DeliveryStatsQuery;

import java.util.ArrayList;
import java.io.OutputStream;
import java.nio.channels.WritableByteChannel;

/**
 * User: artem
 * Date: 24.03.2008
 */

interface StatsFile {
  public String getName();
  public void addStat(DeliveryStat stat) throws StatsFileException;
  public ArrayList<DeliveryStatImpl> getRecords(int start, int count) throws StatsFileException;
  public void transferTo(DeliveryStatsQuery query, OutputStream target) throws StatsFileException;
  public void transferTo(WritableByteChannel target) throws StatsFileException;
  public void compress(DeliveryStatsQuery query) throws StatsFileException;
  public void close() throws StatsFileException;
}
