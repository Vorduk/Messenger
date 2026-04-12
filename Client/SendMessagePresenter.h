#pragma once
#include "ISendMessagePresenter.h"
#include "IUI.h"

class SendMessagePresenter : public ISendMessagePresenter {
public:
    SendMessagePresenter(IUI& ui);
    void onMessageSent(const std::string& message_id, const std::string& confirmation) override;
    void onError(const std::string& error) override;
private:
    IUI& m_ui;
};
