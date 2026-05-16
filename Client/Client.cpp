//#include "Logger.h"
//#include "MessengerWindow.h"
//#include "MessageSender.h"
//#include "SendMessageUseCase.h"
//#include "SendMessagePresenter.h"
//#include "SendMessageController.h"
//#include "JsonMessageSerializer.h"
//#include "Message.h"
//
//int main() {
//    
//    // Logger
//    Logger::getInstance().initialize("messenger.log");
//    Logger::getInstance().setShowFileInfo(false);
//    Logger::getInstance().setShowLevelPrefix(true);
//    Logger::getInstance().setMinimalLevel(Logger::Level::Info);
//
//    // Network
//    MessageSender network("192.168.0.107", 8080);
//    //network.connect();  // установить соединение
//
//    // Serializer
//    std::shared_ptr<JsonMessageSerializer> serializer = std::make_shared<JsonMessageSerializer>();
//
//    // UI
//    MessengerWindow ui(1280, 720, "Messenger Client");
//
//    // Presenter (output port)
//    SendMessagePresenter presenter(ui);
//
//    // Use case
//    SendMessageUseCase send_message_use_case(network, presenter, serializer);
//
//    // Controller
//    SendMessageController controller(send_message_use_case);
//
//    ui.setSendMessageHandler(&controller);
//
//    // Запуск UI
//    ui.run();
//
//    return 0;
//}
//


// Client/TestDB.cpp (временно)
#include "SQLiteMessageRepository.h"
#include <iostream>
#include <random>

// Простой генератор UUID
static std::string generateId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static const char* hex = "0123456789abcdef";
    std::string id;
    for (int i = 0; i < 32; ++i) {
        id += hex[dis(gen)];
        if (i == 7 || i == 11 || i == 15 || i == 19) id += '-';
    }
    return id;
}

int main() {
    SQLiteMessageRepository repo("test_messenger.db", 100);

    // Создать пользователей
    User alice;
    alice.id = generateId();
    alice.username = "@alice";
    alice.display_name = "Alice";
    alice.birthday = "1995-03-15";
    repo.createUser(alice);

    User bob;
    bob.id = generateId();
    bob.username = "@bob";
    bob.display_name = "Bob";
    bob.birthday = "1994-07-22";
    repo.createUser(bob);

    // Отправить сообщение
    Message msg(alice.id, bob.id, "Hello Bob!");
    msg.id = generateId();
    msg.status = MessageStatus::Sent;
    repo.saveMessage(msg);

    // Получить переписку
    auto messages = repo.getMessagesBetween(alice.id, bob.id, 10, 0);
    for (const auto& m : messages) {
        std::cout << "From: " << m.sender_id << " Text: " << m.text << std::endl;
    }

    // Получить всех пользователей
    auto users = repo.getAllUsers();
    std::cout << "Total users: " << users.size() << std::endl;
    for (const auto& u : users) {
        std::cout << "  - " << u.username << " (" << u.display_name << ")" << std::endl;
    }

    std::cout << "Test completed!" << std::endl;
    return 0;
}