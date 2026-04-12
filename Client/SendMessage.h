#pragma once
#include "IMessageSender.h"
#include "ISendMessagePresenter.h"
#include "Message.h"
#include "IMessageSerializer.h"
#include <memory>

class SendMessage {
public:
    SendMessage(IMessageSender& message_sender, ISendMessagePresenter& output, std::shared_ptr<IMessageSerializer> serializer);
    void execute(const std::string& sender_name, const std::string& text);

private:
    IMessageSender& m_message_sender;
    ISendMessagePresenter& m_output;
    std::shared_ptr<IMessageSerializer> m_serializer;
};
