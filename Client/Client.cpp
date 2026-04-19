#include "Logger.h"
#include "MessengerWindow.h"
#include "MessageSender.h"
#include "SendMessageUseCase.h"
#include "SendMessagePresenter.h"
#include "SendMessageController.h"
#include "JsonMessageSerializer.h"
#include "Message.h"

int main() {

    Logger::getInstance().initialize("messenger.log");
    Logger::getInstance().setShowFileInfo(false);
    Logger::getInstance().setShowLevelPrefix(true);
    Logger::getInstance().setMinimalLevel(Logger::Level::Info);

    // UI
    MessengerWindow ui(1280, 720, "Messenger Client");

    // Сеть
    MessageSender network("192.168.0.107", 8080);
    network.connect();  // установить соединение

    // Сериализатор
    auto serializer = std::make_shared<JsonMessageSerializer>();

    // Презентер (output port)
    SendMessagePresenter presenter(ui);

    // Use case
    SendMessageUseCase send_message_use_case(network, presenter, serializer);

    // Контроллер
    SendMessageController controller(send_message_use_case);

    // Привязка UI к контроллеру
    ui.setSendMessageCallback([&controller](const std::string& sender, const std::string& reciever, const std::string& text) {
        controller.onUserSendMessage(sender, reciever, text);
        });

    // Запуск UI
    ui.run();

    return 0;
}

