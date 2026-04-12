#include "SendMessagePresenter.h"

SendMessagePresenter::SendMessagePresenter(IUI& ui) : 
    m_ui(ui)
{ 

}

void SendMessagePresenter::onMessageSent(const std::string& message_id, const std::string& confirmation) {
    m_ui.showSendMessageConfirmation(confirmation);
}

void SendMessagePresenter::onError(const std::string& error) {
    m_ui.showSendMessageError(error);
}