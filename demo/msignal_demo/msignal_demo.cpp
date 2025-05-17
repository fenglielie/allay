// main.cpp
#include <string>

#include "allay/msignal/msignal.hpp"

// 具体的接收者
class TestReceiver {
    std::string m_the_name;

public:
    MReceiver<TestReceiver, int> int_receiver_kernel;  // 接收元
    MSender<int> int_sender_kernel;                    // 发送元

    explicit TestReceiver(const char *name) {
        m_the_name = std::string(name);
        // 把接收元绑定接收者自身和自身的方法
        int_receiver_kernel.init(this, &TestReceiver::method);
    }

    // receiver_exec会调用这个方法
    void method(int m) {
        std::cout << "     " << m_the_name << " receive " << m << " ("
                  << int_receiver_kernel.num() << ")\n";

        if (int_sender_kernel.num() > 0) { emit(m); }
    }

    void emit(int m) {
        std::cout << m_the_name << " send " << m << ": ("
                  << int_sender_kernel.num() << ")\n";

        int_sender_kernel.exec(m);
    }
};

// 具体的发送者
class TestSender {
    std::string m_the_name;

public:
    MSender<int> int_sender_kernel;

    explicit TestSender(const char *name) {
        m_the_name = std::string(name);
        int_sender_kernel.name("TestSender int_sender_kernel\n");
        int_sender_kernel.name(nullptr);
    }

    void emit(int m) {
        std::cout << m_the_name << " send " << m << ": ("
                  << int_sender_kernel.num() << ")\n";

        int_sender_kernel.exec(m);
    }
};

int main() {
    TestSender Ada("Ada");  // 拥有一个发送元

    TestReceiver Mike("Mike");  // 拥有一个发送元和一个接收元
    TestReceiver John("John");
    TestReceiver Jack("Jack");

    Ada.int_sender_kernel.bind(&Mike.int_receiver_kernel);  // Ada-> Mike
    Ada.int_sender_kernel.bind(&John.int_receiver_kernel);  // Ada-> John

    Ada.emit(1);
    // Ada发出信号
    // -> Mike
    // -> John

    Ada.int_sender_kernel.bind(&Jack.int_receiver_kernel);   // Ada-> Jack
    Jack.int_sender_kernel.bind(&John.int_receiver_kernel);  // Jack-> John

    Ada.emit(2);
    // Ada发出信号, 都会收到, 这时John会收到两次
    // -> Mike
    // -> John
    // -> Jack
    // Jack发出信号
    // -> John

    Ada.int_sender_kernel.unbind(
        &Jack.int_receiver_kernel);  // Ada不再绑定到Jack

    Ada.emit(3);
    // Ada发出信号
    // -> Mike
    // -> John

    John.int_receiver_kernel.unbind_all();  // John的接收元解除了所有的绑定

    Ada.emit(4);
    // Ada发出信号
    // -> Mike

    return 0;
}
