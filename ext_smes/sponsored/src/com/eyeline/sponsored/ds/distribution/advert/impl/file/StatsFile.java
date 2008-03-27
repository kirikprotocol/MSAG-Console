package com.eyeline.sponsored.ds.distribution.advert.impl.file;

import com.eyeline.sponsored.ds.distribution.advert.DeliveryStat;
import com.eyeline.sponsored.ds.distribution.advert.impl.file.DeliveryStatImpl;

import java.io.IOException;
import java.io.FileNotFoundException;
import java.util.Collection;
import java.util.ArrayList;
import java.nio.channels.WritableByteChannel;

/**
 * User: artem
 * Date: 24.03.2008
 */

public interface StatsFile {
  public String getName();
  public void addStat(DeliveryStat stat) throws StatsFileException;
  public ArrayList<DeliveryStatImpl> getRecords(int start, int count) throws StatsFileException;
  public void transferTo(WritableByteChannel target) throws StatsFileException;
  public void compress() throws StatsFileException;
  public void close() throws StatsFileException;
}
