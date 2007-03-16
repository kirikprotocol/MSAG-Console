package ru.sibinco.smsx.services.sponsored;

import ru.aurorisoft.smpp.Message;

/**
 * User: artem
 * Date: 13.03.2007
 */

public class SponsoredRequestParser {
  private static final String SPONSORED = "(S|s)(P|p)(O|o)(N|n)(S|s)(O|o)(R|r)(E|e)(D|d)";
  private static final String NONE = "(N|n)(O|o)(N|n)(E|e)";
  private static final String ANY_NUMBER = "\\d+";
  private static final String ONE_OR_MORE_SPACES = "\\s+";

  private static final String SPONSORED_SUBSCRIBE = SPONSORED + ONE_OR_MORE_SPACES + ANY_NUMBER;
  private static final String SPONSORED_UNSUBSCRIBE = SPONSORED + ONE_OR_MORE_SPACES + NONE;

  public static ParsedMessage parseRequest(final Message request) throws WrongSubscriptionCountException {
    if (request.getMessageString().matches(SPONSORED_SUBSCRIBE)) {
      try {
        final int count = Integer.parseInt(request.getMessageString().split(" ")[1]);

        for (int i=0; i < SponsoredServiceNew.Properties.ALLOWED_COUNTS.length; i++)
          if (SponsoredServiceNew.Properties.ALLOWED_COUNTS[i] == count)
            return new ParsedMessage(request, ParsedMessage.SUBSCRIBE, count);
      } catch (NumberFormatException e) {
      }
      throw new WrongSubscriptionCountException();
    } else if (request.getMessageString().matches(SPONSORED_UNSUBSCRIBE))
      return new ParsedMessage(request, ParsedMessage.UNSUBSCRIBE, 0);
    else if (request.getMessageString().matches(SPONSORED + ".*"))
      throw new WrongSubscriptionCountException();
    return null;
  }

  public static class WrongSubscriptionCountException extends Exception {}
}
