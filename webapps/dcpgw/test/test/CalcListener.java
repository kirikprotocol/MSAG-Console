package test;

import org.junit.runner.Description;
import org.junit.runner.notification.Failure;
import org.junit.runner.notification.RunListener;

/**
 * Created by IntelliJ IDEA.
 * User: Stepanov Dmitry Nikolaevich
 * Date: 23.09.11
 * Time: 14:31
 */
public class CalcListener extends RunListener {

    @Override
    public void testStarted(Description desc) {
        System.out.println("Started:" + desc.getDisplayName());
    }

    @Override
    public void testFinished(Description desc) {
        System.out.println("Finished:" + desc.getDisplayName());
    }

    @Override
    public void testFailure(Failure fail) {
        System.out.println("Failed:" + fail.getDescription().getDisplayName() + " [" + fail.getMessage() + "]");
    }
}
