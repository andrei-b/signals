//
// Created by aitonomi on 31.03.26.
//

#ifndef UNTITLED_CONNECTION_H
#define UNTITLED_CONNECTION_H
#include <any>
#include <functional>
#include <memory>

class Connection : public std::enable_shared_from_this<Connection> {
public:
    static std::shared_ptr<Connection> create_connection(std::any * object);
    void set_object(std::any * object);
    std::any * get_object();
    void disconnect();
    bool is_connected();
    ~Connection() = default;
private:
    explicit Connection(std::any * object);
    std::any* object_;

};



#endif //UNTITLED_CONNECTION_H
