
#ifndef UNTITLED_SIGNAL_H
#define UNTITLED_SIGNAL_H

#include <algorithm>
#include <any>
#include <functional>
#include <utility>
#include <vector>

#include "Connection.h"

template <typename... Args>
using Callback = std::function<bool(std::any * object, Args...)>;


template <typename... Args>
class Signal {
    public:
    explicit Signal(Callback<Args...> cb) : cbs_{std::move(cb)} {
    }
    std::shared_ptr<Connection> connect(std::any * object) {
        auto connection = std::make_shared<Connection>(object);
        connections_.push_back(connection);
        return connection;
    }
    void disconnect(const std::any * object) {
        auto connection = std::find(connections_.begin(), connections_.end(), [&](const std::shared_ptr<Connection>& conn) {
            return conn->get_object() == object;
        });
        if (connection != connections_.end()) {
            connections_.erase(connection);
        }
    }
    void disconnect_all() {
        connections_.clear();
    }
    bool is_connected() {
        auto it = std::find_if(cbs_.begin(), cbs_.end(), [](const std::shared_ptr<Connection>& conn) {
            return conn->get_object() != nullptr;
        });
        return it != cbs_.end();
    }
    void append_callback(Callback<Args...> cb) {
        cbs_.push_back(std::move(cb));
    }
    void emit(Args... args) {
        std::erase_if(connections_, [](const std::shared_ptr<Connection>& connection) {
                    return connection->get_object() == nullptr;
                });
        for (const auto& connection : connections_) {
            if (auto * object = connection->get_object(); object != nullptr) {
                for (const auto &cb : cbs_) {
                    if ( cb(object, args...) ) {
                        break;
                    }
                }
            }
        }
    }
private:
    std::vector<Callback<Args...>> cbs_;
    std::vector<std::shared_ptr<Connection>> connections_;
};



#endif //UNTITLED_SIGNAL_H
