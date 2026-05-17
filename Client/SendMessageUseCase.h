#pragma once
#include "IServerAPI.h"
#include "IMessageRepository.h"
#include "ISendMessagePresenter.h"
#include <string>
#include <memory>

class SendMessageUseCase {
public:
    SendMessageUseCase(IServerAPI& server, ISendMessagePresenter& presenter, IMessageRepository& local_repo);
    void execute(const std::string& senderId, const std::string& receiverId,
        const std::string& text);
private:
    IServerAPI& m_server;
    ISendMessagePresenter& m_presenter;
    IMessageRepository& m_local_repo;
};