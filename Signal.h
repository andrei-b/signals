#ifndef UNTITLED_SIGNAL_H
#define UNTITLED_SIGNAL_H

#include <algorithm>
#include <any>
#include <functional>
#include <utility>
#include <vector>

#include "Connection.h"

template <typename... Args>
using Callback = std::function<bool(std::any*, Args...)>;


template <typename... Args>
class Signal {
    public:
    explicit Signal(Callback<Args...> cb) : cbs_{std::move(cb)} {
    }
    std::shared_ptr<Connection> connect(std::any* object) {
        auto connection = Connection::create_connection(object);
        connections_.push_back(connection);
        return connection;
    }
    // Disconnect by connection handle
    void disconnect(const std::shared_ptr<Connection>& conn) {
        auto it = std::find(connections_.begin(), connections_.end(), conn);
        if (it != connections_.end())
            connections_.erase(it);
    }
    void disconnect_all() {
        connections_.clear();
    }
    bool is_connected() {
        return std::find_if(connections_.begin(), connections_.end(),
            [](const std::shared_ptr<Connection>& c) {
                return c->is_connected();
            }) != connections_.end();
    }
    void add_callback(Callback<Args...> cb) {
        cbs_.push_back(std::move(cb));
    }
    void emit(Args... args) {
        std::erase_if(connections_, [](const std::shared_ptr<Connection>& c) {
            return !c->is_connected();
        });
        for (const auto& connection : connections_) {
            if (connection->is_connected()) {
                auto* object = connection->get_object();
                for (const auto& cb : cbs_) {
                    if (cb(object, args...)) break;
                }
            }
        }
    }
private:
    std::vector<Callback<Args...>> cbs_;
    std::vector<std::shared_ptr<Connection>> connections_;
};



#endif //UNTITLED_SIGNAL_H
