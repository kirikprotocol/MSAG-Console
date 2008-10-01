import org.junit.Test;

import java.util.regex.Pattern;
import java.util.regex.Matcher;

/**
 * author: alkhal
 */
// todo ?????
public class RegexpTest {
    @Test
    public void mather() {

    }
public static void checkEmail(String sEmail) {

    Pattern p = Pattern.compile("(on|ON|On|oN)");
    Matcher m = p.matcher(sEmail);

    String sResult = m.matches()?sEmail + ": passed.":sEmail + ": not passed.";

    System.out.println(sResult);
    }

    public static void main(String[] args) {
        checkEmail("oN");
    }
}
