#include "Logger.h"
#include "MessengerWindow.h"
#include "MessageSender.h"
#include "SendMessageUseCase.h"
#include "SendMessagePresenter.h"
#include "SendMessageController.h"
#include "JsonMessageSerializer.h"
#include "Message.h"

int main() {
    
    // Logger
    Logger::getInstance().initialize("messenger.log");
    Logger::getInstance().setShowFileInfo(false);
    Logger::getInstance().setShowLevelPrefix(true);
    Logger::getInstance().setMinimalLevel(Logger::Level::Info);

    // Network
    MessageSender network("192.168.0.107", 8080);
    //network.connect();  // установить соединение

    // Serializer
    std::shared_ptr<JsonMessageSerializer> serializer = std::make_shared<JsonMessageSerializer>();

    // UI
    MessengerWindow ui(1280, 720, "Messenger Client");

    // Presenter (output port)
    SendMessagePresenter presenter(ui);

    // Use case
    SendMessageUseCase send_message_use_case(network, presenter, serializer);

    // Controller
    SendMessageController controller(send_message_use_case);

    // Bind ui to controller
    ui.setSendMessageCallback([&controller](const std::string& sender, const std::string& reciever, const std::string& text) {
        controller.onUserSendMessage(sender, reciever, text);
        });

    // Запуск UI
    ui.run();

    return 0;
}

