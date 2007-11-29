package ru.sibinco.smpp.ub_sme;

import ru.aurorisoft.smpp.Message;

public class Response {

    public static final int TYPE_ARRAY = 0;

    private static final int[] TYPES = {TYPE_ARRAY};

    private int responseType = -1;
    private Message[] messages = null;

    public Response(int type) throws IllegalArgumentException {
        setResponseType(type);
    }

    public Response(int type, Message[] messages) throws IllegalArgumentException {
        setResponseType(type);
        this.messages = messages;
    }

    private void setResponseType(int responseType) throws IllegalArgumentException {
        for (int i = 0; i < TYPES.length; i++) {
            if (responseType == TYPES[i]) {
                this.responseType = responseType;
                break;
            }
        }
        if (this.responseType == -1)
            throw new IllegalArgumentException("Inllegal resoponse type: " + responseType);
    }

    public int getResponseType() {
        return responseType;
    }

    public Message[] getMessages() {
        return messages;
    }

    public void addMessage(Message message) {
        if (messages == null) {
            messages = new Message[1];
            messages[0] = message;
        } else {
            Message[] _messages = new Message[messages.length + 1];
            System.arraycopy(messages, 0, _messages, 0, messages.length);
            _messages[messages.length] = message;
            messages = _messages;
        }
    }
}
