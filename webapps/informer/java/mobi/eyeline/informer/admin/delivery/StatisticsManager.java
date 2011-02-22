package mobi.eyeline.informer.admin.delivery;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.delivery.stat.*;
import mobi.eyeline.informer.admin.filesystem.FileSystem;

import java.io.File;
import java.util.Date;

/**
 * @author Aleksandr Khalitov
 */
public class StatisticsManager {

  private final DeliveryStatProvider statsProvider;

  private final UserStatProvider userStatsProvider;

  public StatisticsManager(File statsDirectory, FileSystem fileSys) {
    this.statsProvider = new DeliveryStatProvider(statsDirectory, fileSys);
    this.userStatsProvider = new UserStatProvider(statsDirectory, fileSys);
  }

  protected StatisticsManager(DeliveryStatProvider statsProvider, UserStatProvider userStatsProvider) {
    this.statsProvider = statsProvider;
    this.userStatsProvider = userStatsProvider;
  }

  /**
   * Поочередно передает в visitor все записи статистики, удовлетворяющие условиям, накладываемыми в filter.
   * Процесс продолжается до тех пор, пока метод visit в visitor возвращает true, либо записи не закончатся.
   * Если filter == null, то провайдер перебирает все записи.
   *
   * @param filter  фильтр, описывающий ограничения на записи
   * @param visitor визитор, обрабатывающий найденные записи
   * @throws mobi.eyeline.informer.admin.AdminException если произошла ошибка при обращении к стораджу статистики
   */
  public void statistics(DeliveryStatFilter filter, DeliveryStatVisitor visitor) throws AdminException {
    statsProvider.accept(filter, visitor);
  }

  public void statisticsByUser(UserStatFilter filter, UserStatVisitor visitor) throws AdminException {
    userStatsProvider.accept(filter, visitor);
  }


  public void getStatEntities(StatEntityProvider.EntityVisitor v, Date from, Date till) throws AdminException {
    userStatsProvider.visitEntities(from, till, v);
    statsProvider.visitEntities(from, till,  v);

  }

  public void dropStatEntities(Date from, Date till) throws AdminException {
    userStatsProvider.dropEntities(from, till);
    statsProvider.dropEntities(from, till);
  }
}
