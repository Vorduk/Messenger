#include "MessengerWindow.h"
#include "MessageSender.h"
#include "SendMessage.h"
#include "SendMessagePresenter.h"
#include "SendMessageController.h"
#include "JsonMessageSerializer.h"

int main() {
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
    SendMessage send_message_use_case(network, presenter, serializer);

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

