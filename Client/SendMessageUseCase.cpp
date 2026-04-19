#include "SendMessageUseCase.h"

SendMessageUseCase::SendMessageUseCase(IMessageSender& message_sender, ISendMessagePresenter& output, std::shared_ptr<IMessageSerializer> serializer) :
    m_message_sender(message_sender), m_output(output), m_serializer(serializer)
{

}

void SendMessageUseCase::execute(const std::string& sender, const std::string& reciever, const std::string& text) {
    Message message(sender, reciever, text);
    if (message.text.empty()) {
        m_output.onError("Message cannot be empty");
        return;
    }
    std::string serialized_message = m_serializer->serialize(message);
    if (!m_message_sender.send(serialized_message)) {
        m_output.onError("Failed to send message (no connection)");
        return;
    }
    m_output.onMessageSent("todo: id", "message was sent");
}