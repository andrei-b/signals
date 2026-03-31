//
// Created by aitonomi on 31.03.26.
//

#include "Connection.h"

Connection::Connection(std::any* object): object_(object) {}

std::shared_ptr<Connection> Connection::create_connection(std::any* object) {
    return std::shared_ptr<Connection>(new Connection(object));
}

void Connection::set_object(std::any* object) {
    object_ = object;
}

std::any* Connection::get_object() {
    return object_;
}

void Connection::disconnect() {
    object_ = nullptr;
}

bool Connection::is_connected() {
    return object_ != nullptr;
}
