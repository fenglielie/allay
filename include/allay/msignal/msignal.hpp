#pragma once

#include <iostream>
#include <list>

// 基于std::list在发送元和接收元内部记录链表

// 把两个基类放置在m_signal子命名空间
// MSender和MReceiver放置在m_signal命名空间
// 使用bind和unbind两个方法进行绑定和解绑, 发送元和接收元都可以发起, 而且等价

namespace m_signal {

// 接收元基类的声明前置, 因为两个基类存在相互的调用关系

template <typename Ym>
class MReceiverBase;

// 发送元基类
// 需要使用接收元基类指针作为参数
template <typename Ym>
class MSenderBase {
public:
    virtual ~MSenderBase() = default;

    virtual void exec(Ym message) const = 0;  // 发送行为

    // 这两个也需要被接收元调用，因此首先在基类中定义

    virtual bool
    detail_sender_append(MReceiverBase<Ym> *receiver) = 0;  // 保存接收元记录
    virtual bool
    detail_sender_remove(MReceiverBase<Ym> *receiver) = 0;  // 删除接收元记录
};

// 接收元基类
// 需要使用发送元基类指针作为参数
template <typename Ym>
class MReceiverBase {
public:
    virtual ~MReceiverBase() = default;

    virtual void exec(Ym message) const = 0;  // 接收行为

    // 这两个也需要被发送元调用，因此首先在基类中定义

    virtual bool
    detail_receiver_append(MSenderBase<Ym> *) = 0;  // 保存发送元记录
    virtual bool
    detail_receiver_remove(MSenderBase<Ym> *) = 0;  // 删除发送元记录
};

}  // namespace m_signal

// 前置声明
template <typename Ym>
class MSender;

// 接收元
// Y 接收者类型 Ym 信息类型
template <typename Y, typename Ym>
class MReceiver final : public m_signal::MReceiverBase<Ym> {
private:
    Y *m_the_receiver = nullptr;          // 接收者指针
    void (Y::*m_the_func)(Ym) = nullptr;  // 接收者将要调用的方法指针

    std::list<m_signal::MSenderBase<Ym> *> m_the_senders;  // 发送元记录

    const char *m_the_name = nullptr;  // 名称字符串

public:
    //*********************************************************************************************************//

    // 默认构造函数
    MReceiver() = default;

    MReceiver(const MReceiver &) = delete;
    MReceiver &operator=(const MReceiver &) = delete;

    // 自带初始化的构造函数
    MReceiver(Y *receiver, void (Y::*func)(Ym))
        : m_the_receiver(receiver), m_the_func(func) {}

    // 接收元的初始化
    void init(Y *receiver, void (Y::*func)(Ym)) {
        m_the_receiver = receiver;
        m_the_func = func;
    }

    // 添加名称字符串, 会在exec的开头自动调用
    void name(const char *name_str) { m_the_name = name_str; }

    // 接收行为
    // 被发送元的发送行为主动调用
    void exec(Ym message) const {
        if (m_the_name != nullptr) std::cout << m_the_name;

        // 只有在初始化完成的情况下, 才会调用; 否则空指针不会调用
        if (m_the_receiver && m_the_func) {
            (m_the_receiver->*m_the_func)(message);
        }
    }

    // 解绑全部, 析构会自动调用
    // 通知所有记录的发送元调用sender_remove方法, 删除这个接收元记录
    void detail_receiver_unbind_all() {
        auto it = m_the_senders.begin();
        while (it != m_the_senders.end()) {
            (*it)->detail_sender_remove(this);
            it = m_the_senders.erase(it);
        }
    }

    // 接收元析构
    ~MReceiver() { detail_receiver_unbind_all(); }

    //*********************************************************************************************************//

    // 保存发送元记录
    bool detail_receiver_append(m_signal::MSenderBase<Ym> *sender) {
        auto it = m_the_senders.begin();
        while (it != m_the_senders.end()) {
            if (*it == sender) {  // 发送元已存在
                return false;
            }
            ++it;
        }

        m_the_senders.push_back(sender);  // 保存发送元记录
        return true;
    }

    // 删除发送元记录
    bool detail_receiver_remove(m_signal::MSenderBase<Ym> *sender) {
        auto it = m_the_senders.begin();
        while (it != m_the_senders.end()) {
            if (*it == sender) {
                it = m_the_senders.erase(it);  // 删除发送元记录
                return true;
            }
            ++it;
        }
        return false;  // 发送元不存在
    }

    //*********************************************************************************************************//

    // 绑定发送元
    void detail_receiver_bind(m_signal::MSenderBase<Ym> *sender) {
        if (detail_receiver_append(sender)) {
            sender->detail_sender_append(this);  // 添加新的发送元时, 通知发送元
        }
    }

    // 解绑发送元
    void detail_receiver_unbind(m_signal::MSenderBase<Ym> *sender) {
        if (detail_receiver_remove(sender)) {
            sender->detail_sender_remove(
                this);  // 删除已有的发送元时, 通知发送元
        }
    }

    //*********************************************************************************************************//
    // 更实用的接口

    // 绑定, 转换指针为基类指针
    void bind(MSender<Ym> *sender) {
        detail_receiver_bind(static_cast<m_signal::MSenderBase<Ym> *>(sender));
    }

    // 解绑, 转换指针为基类指针
    void unbind(MSender<Ym> *sender) {
        detail_receiver_unbind(
            static_cast<m_signal::MSenderBase<Ym> *>(sender));
    }

    // 解绑所有的发送元
    void unbind_all() { detail_receiver_unbind_all(); }

    // 查询发送元记录
    bool is_bind(m_signal::MSenderBase<Ym> *sender) const {
        auto it = m_the_senders.begin();
        while (it != m_the_senders.end()) {
            if (*it == sender) {  // 发送元已存在
                return true;
            }
            ++it;
        }

        return false;  // 发送元不存在
    }

    // 查询接收元记录的个数
    size_t num() const { return m_the_senders.size(); }

    // 查询接收元是否被正确地初始化
    bool is_init() const {
        return static_cast<bool>(m_the_receiver && m_the_func);
    }

    //*********************************************************************************************************//
};

// 发送元
// Ym 信息类型
template <typename Ym>
class MSender final : public m_signal::MSenderBase<Ym> {
private:
    std::list<m_signal::MReceiverBase<Ym> *> m_the_receivers;  // 接收元记录

    const char *m_the_name = nullptr;  // 名称字符串

public:
    //*********************************************************************************************************//

    // 默认构造函数
    MSender() = default;

    MSender(const MSender &) = delete;
    MSender &operator=(const MSender &) = delete;

    // 添加名称字符串, 会在exec的开头自动调用
    void name(const char *name_str) { m_the_name = name_str; }

    // 发送行为
    // 遍历所有接收元记录, 依次触发接收元的接收行为, 传递信息
    void exec(Ym message) const {
        if (m_the_name != nullptr) std::cout << m_the_name;

        auto it = m_the_receivers.begin();
        while (it != m_the_receivers.end()) {
            (*it)->exec(message);
            ++it;
        }
    }

    // 解绑全部, 析构会自动调用
    // 通知所有记录的的接收元调用receiver_remove方法, 删除这个发送元记录
    void detail_sender_unbind_all() {
        auto it = m_the_receivers.begin();
        while (it != m_the_receivers.end()) {
            (*it)->detail_receiver_remove(this);
            it = m_the_receivers.erase(it);
        }
    }

    // 发送元析构
    ~MSender() { detail_sender_unbind_all(); }

    //*********************************************************************************************************//

    // 保存接收元记录
    bool detail_sender_append(m_signal::MReceiverBase<Ym> *receiver) {
        auto it = m_the_receivers.begin();
        while (it != m_the_receivers.end()) {
            if (*it == receiver) {  // 接收元已存在
                return false;
            }
            ++it;
        }

        m_the_receivers.push_back(receiver);  // 保存接收元记录
        return true;
    }

    // 删除接收元记录
    bool detail_sender_remove(m_signal::MReceiverBase<Ym> *receiver) {
        auto it = m_the_receivers.begin();
        while (it != m_the_receivers.end()) {
            if (*it == receiver) {
                it = m_the_receivers.erase(it);  // 删除接收元记录
                return true;
            }
            ++it;
        }
        return false;  // 接收元不存在
    }

    //*********************************************************************************************************//

    // 绑定接收元
    void detail_sender_bind(m_signal::MReceiverBase<Ym> *receiver) {
        if (detail_sender_append(receiver)) {
            receiver->detail_receiver_append(
                this);  // 添加新的接收元时, 通知接收元
        }
    }

    // 解绑接收元
    void detail_sender_unbind(m_signal::MReceiverBase<Ym> *receiver) {
        if (detail_sender_remove(receiver)) {
            receiver->detail_receiver_remove(
                this);  // 删除已有的接收元时, 通知接收元
        }
    }

    //*********************************************************************************************************//
    // 更实用的接口

    // 绑定, 转换指针为基类指针
    template <typename Y>
    void bind(MReceiver<Y, Ym> *receiver) {
        detail_sender_bind(
            static_cast<m_signal::MReceiverBase<Ym> *>(receiver));
    }

    // 解绑, 转换指针为基类指针
    template <typename Y>
    void unbind(MReceiver<Y, Ym> *receiver) {
        detail_sender_unbind(
            static_cast<m_signal::MReceiverBase<Ym> *>(receiver));
    }

    // 解绑所有的接收元
    void unbind_all() { detail_sender_unbind_all(); }

    // 查询接收元记录
    bool is_bind(m_signal::MReceiverBase<Ym> *receiver) const {
        auto it = m_the_receivers.begin();
        while (it != m_the_receivers.end()) {
            if (*it == receiver) {  // 接收元已存在
                return true;
            }
            ++it;
        }

        return false;  // 接收元不存在
    }

    // 查询接收元记录的个数
    size_t num() const { return m_the_receivers.size(); }

    //*********************************************************************************************************//
};
