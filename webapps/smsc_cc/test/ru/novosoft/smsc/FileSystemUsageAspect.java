package ru.novosoft.smsc;

import junit.framework.AssertionFailedError;
import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Before;
import org.aspectj.lang.annotation.Pointcut;
import org.junit.Test;

/**
 * User: artem
 * Date: 28.10.11
 */
@Aspect
public class FileSystemUsageAspect {

  @Pointcut("call (boolean java.io.File.exists())")
  public void fileExistsCall() {
  }

  @Pointcut("call (java.io.FileInputStream.new(..))")
  public void fileInputStreamCreationCall() {
  }

  @Before("(fileExistsCall() || fileInputStreamCreationCall()) " +
      "&& !(within(ru.novosoft.smsc.admin.filesystem.*) || within(ru.novosoft.smsc.util..*) || within (ru.novosoft.smsc..*Test)" +
      "|| within(testutils.TestUtils) || within(ru.novosoft.smsc.admin.config.ManagedConfigHelper) || within(ru.novosoft.smsc.admin.TestAdminContext)) ")
  public void deprecatedIOOperations() {
    throw new AssertionFailedError("Deprecated IO operation call ");
  }
}
