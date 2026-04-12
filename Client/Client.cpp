#include "ClientConsoleUI.h"
#include "MessageSender.h"
#include "SendMessage.h"
#include "SendMessagePresenter.h"
#include "SendMessageController.h"
#include "JsonMessageSerializer.h"

int main() {
    // UI
    ClientConsoleUI ui;

    // Сеть
    MessageSender network("127.0.0.1", 8080);
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
    ui.setSendMessageCallback([&controller](const std::string& sender, const std::string& text) {
        controller.onUserSendMessage(sender, text);
        });

    // Запуск UI
    ui.run();

    return 0;
}

