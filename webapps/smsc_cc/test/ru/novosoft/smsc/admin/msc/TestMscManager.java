package ru.novosoft.smsc.admin.msc;

import ru.novosoft.smsc.admin.cluster_controller.ClusterController;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.Address;

import java.io.*;
import java.util.ArrayList;
import java.util.Collection;

/**
 * @author Artem Snopkov
 */
public class TestMscManager extends MscManagerImpl {

  public TestMscManager(File aliasesFile, ClusterController cc, FileSystem fs) {
    super(aliasesFile, cc, fs);
  }

}
