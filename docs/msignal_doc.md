# MSignal 信号类

## 基本概念

首先定义如下的概念：

- 接收元件：`Receiver<Y,Ym>`
- 发送元件：`Sender<Ym>`
- 接收者：拥有至少一个接受元（公开成员）的类
- 发送者：拥有至少一个发送元件（公开成员）的类
- 中转者：既是发送者也是接收者的类

其中：`Ym`是信息类型, `Y`是接收者类型，接收元件和发送元件必须使用同样的信息类型`Ym`, 才能建立联系进行信息传递。

信息会从建立连接的发送元件传递到接收元件，发送元件和接收元件之间可以是一对多或多对多地建议联系。


## 具体说明

### 元件之间的连接

发送元件和接收元件之间的连接是双向且对等的（与QT的信号和槽函数机制不同），
它们都可以与对方绑定与解绑（一对多，多对多）
```cpp
sender.bind(receiver); // 发送元件可以绑定接收元件
sender.unbind(receiver); // 发送元件可以解绑接收元件
sender.unbind_all(); // 发送元件可以解绑所有的接收元件, 在析构时会自动调用

receiver.bind(sender); // 接收元件可以绑定发送元件
receiver.unbind(sender); // 接收元件可以解绑发送元件
receiver.unbind_all(); // 接收元件可以解绑所有的发送元件, 在析构时会自动调用
```

在发送元件内部会维持一个与自己连接的接收元件记录列表, 可以查询自己的状态
```cpp
sender.num()             // 查询接收元件记录的个数
sender.is_bind(receiver) // 查询接收元件是否已被记录
```
对接收元件也是一样的
```cpp
receiver.num()           // 查询发送元件记录的个数
receiver.is_bind(sender) // 查询发送元件是否已被记录
receiver.is_init()       // 查询现在是否已经初始化
```

### 发送者的发送行为

1. 创建信息`message`
2. 在自己的方法内部主动调用发送元件的`sender.exec(message)`方法，将消息发送出去

例如
```cpp
class TestSender {
    std::string m_name;

public:
    MSender<int> int_sender_kernel;

    explicit TestSender(const std::string &name): m_name(name) {
        int_sender_kernel.name(nullptr);
    }

    // 调用发送元件来发送信息
    void emit(int m) {
        int_sender_kernel.exec(m);
    }
};
```

### 接收者的接收行为

接收者在构造函数中需要调用`init`方法初始化接受元，将接收者的指针`X`和指定的公开方法指针`Y`传递给它。然后接收元件在收到消息后，就可以正确调用接收者的指定的公开方法，否则无效但不会报错，只是消息丢失了。
```cpp
receiver.init(X,Y)
```

例如
```cpp
class TestReceiver {
    std::string m_name;

public:
    MReceiver<TestReceiver, int> int_receiver_kernel;  // 接收元件

    explicit TestReceiver(const std::string &name): m_name(name) {
        // 把接收元件绑定到接收者自身和自身的方法
        int_receiver_kernel.init(this, &TestReceiver::method);
    }

    void method(int m) {
        // 接受元会自动调用这个方法
        std::cout << "     " << m_the_name << " receive " << m << " ("
                  << int_receiver_kernel.num() << ")\n";
    }
};
```

## 补充

为了让消息传递过程可视化，支持给发送元件和接收元件指定名称，
```cpp
sender.name(name_str)
receiver.name(name_str)
```
如果设置了, 则在传递消息时会首先把名称字符串传递给`std::cout`，可以传入一个`nullptr`用来移除名称字符串。

> 不要使用`detail_`开头的接口, 这是不合适的操作, 但又不方便设置私有权限，可能导致指针和析构的错误。
