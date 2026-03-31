#include <gtest/gtest.h>
#include <any>
#include <string>
#include <vector>

#include "Signal.h"

// ── helpers ───────────────────────────────────────────────────────────────────
static auto make_noop() {
    return [](std::any*) -> bool { return false; };
}

// ── connect / is_connected ────────────────────────────────────────────────────

TEST(SignalTest, ConnectReturnsValidConnection) {
    Signal<> sig(make_noop());
    std::any obj = 42;
    auto conn = sig.connect(&obj);

    ASSERT_NE(conn, nullptr);
    EXPECT_TRUE(conn->is_connected());
    EXPECT_EQ(conn->get_object(), &obj);
}

TEST(SignalTest, IsConnectedFalseBeforeConnect) {
    Signal<> sig(make_noop());
    EXPECT_FALSE(sig.is_connected());
}

TEST(SignalTest, IsConnectedTrueAfterConnect) {
    Signal<> sig(make_noop());
    std::any obj = 1;
    sig.connect(&obj);
    EXPECT_TRUE(sig.is_connected());
}

// ── disconnect ────────────────────────────────────────────────────────────────

// Disconnect via the signal, passing back the connection handle.
TEST(SignalTest, DisconnectViaSignal) {
    Signal<> sig(make_noop());
    std::any obj = 1;
    auto conn = sig.connect(&obj);
    sig.disconnect(conn);
    EXPECT_FALSE(sig.is_connected());
}

TEST(SignalTest, DisconnectViaConnectionObject) {
    Signal<> sig(make_noop());
    std::any obj = 1;
    auto conn = sig.connect(&obj);
    conn->disconnect();
    EXPECT_FALSE(sig.is_connected());
}

// Trying to disconnect a handle that is no longer in the signal should do nothing.
TEST(SignalTest, DisconnectUnknownConnectionDoesNothing) {
    Signal<> sig(make_noop());
    std::any obj1 = 1, obj2 = 2;
    auto conn1 = sig.connect(&obj1);
    sig.connect(&obj2);
    sig.disconnect(conn1);          // remove conn1
    sig.disconnect(conn1);          // second attempt – no-op
    EXPECT_TRUE(sig.is_connected()); // conn2 still alive
}

TEST(SignalTest, DisconnectAll) {
    Signal<> sig(make_noop());
    std::any o1 = 1, o2 = 2, o3 = 3;
    sig.connect(&o1);
    sig.connect(&o2);
    sig.connect(&o3);
    sig.disconnect_all();
    EXPECT_FALSE(sig.is_connected());
}

// ── emit – argument passing ───────────────────────────────────────────────────

TEST(SignalTest, EmitNoArgs_CallbackReceivesObject) {
    std::any* received = nullptr;
    Signal<> sig([&](std::any* obj) -> bool {
        received = obj;
        return false;
    });
    std::any obj = 99;
    sig.connect(&obj);
    sig.emit();
    EXPECT_EQ(received, &obj);
}

TEST(SignalTest, EmitSingleIntArg) {
    int received = 0;
    Signal<int> sig([&](std::any*, int v) -> bool {
        received = v;
        return false;
    });
    std::any obj = 1;
    sig.connect(&obj);
    sig.emit(42);
    EXPECT_EQ(received, 42);
}

TEST(SignalTest, EmitMultipleArgs) {
    int got_i = 0;
    std::string got_s;
    Signal<int, std::string> sig([&](std::any*, int i, std::string s) -> bool {
        got_i = i;
        got_s = std::move(s);
        return false;
    });
    std::any obj = 1;
    sig.connect(&obj);
    sig.emit(7, "hello");
    EXPECT_EQ(got_i, 7);
    EXPECT_EQ(got_s, "hello");
}

// ── emit – multiple connections ───────────────────────────────────────────────

TEST(SignalTest, EmitReachesAllConnections) {
    int count = 0;
    Signal<> sig([&](std::any*) -> bool {
        ++count;
        return false;
    });
    std::any o1 = 1, o2 = 2, o3 = 3;
    sig.connect(&o1);
    sig.connect(&o2);
    sig.connect(&o3);
    sig.emit();
    EXPECT_EQ(count, 3);
}

// ── emit – two objects of different classes ───────────────────────────────────

TEST(SignalTest, ConnectObjectsOfDifferentClasses) {
    struct Cat  { int lives = 9; };
    struct Door { bool open = false; };

    std::vector<std::any*> visited;
    Signal<> sig([&](std::any* obj) -> bool {
        visited.push_back(obj);
        return false;
    });

    std::any cat  = Cat{};
    std::any door = Door{};
    sig.connect(&cat);
    sig.connect(&door);
    sig.emit();

    ASSERT_EQ(visited.size(), 2u);
    EXPECT_NO_THROW(std::any_cast<Cat> (*visited[0]));
    EXPECT_NO_THROW(std::any_cast<Door>(*visited[1]));
}

// ── emit – callback chain control ────────────────────────────────────────────

TEST(SignalTest, CallbackReturningTrueStopsChain) {
    int first = 0, second = 0;
    Signal<> sig([&](std::any*) -> bool {
        ++first;
        return true;    // stop – second callback must NOT run
    });
    sig.add_callback([&](std::any*) -> bool {
        ++second;
        return false;
    });
    std::any obj = 1;
    sig.connect(&obj);
    sig.emit();
    EXPECT_EQ(first,  1);
    EXPECT_EQ(second, 0);
}

TEST(SignalTest, CallbackReturningFalseContinuesChain) {
    int first = 0, second = 0;
    Signal<> sig([&](std::any*) -> bool {
        ++first;
        return false;   // continue
    });
    sig.add_callback([&](std::any*) -> bool {
        ++second;
        return false;
    });
    std::any obj = 1;
    sig.connect(&obj);
    sig.emit();
    EXPECT_EQ(first,  1);
    EXPECT_EQ(second, 1);
}

// ── emit – stale (empty) connections are pruned ───────────────────────────────

TEST(SignalTest, NullConnectionsPrunedDuringEmit) {
    int count = 0;
    Signal<> sig([&](std::any*) -> bool {
        ++count;
        return false;
    });
    std::any o1 = 1, o2 = 2;
    auto conn1 = sig.connect(&o1);    // will be disconnected
    sig.connect(&o2);                 // stays alive

    conn1->disconnect();            // object_ becomes nullptr
    sig.emit();

    EXPECT_EQ(count, 1);            // only the second connection fires
}

// ── add_callback ──────────────────────────────────────────────────────────────

TEST(SignalTest, AppendCallbackCalledInOrder) {
    std::vector<int> order;
    Signal<> sig([&](std::any*) -> bool { order.push_back(1); return false; });
    sig.add_callback([&](std::any*) -> bool { order.push_back(2); return false; });
    sig.add_callback([&](std::any*) -> bool { order.push_back(3); return false; });
    std::any obj = 1;
    sig.connect(&obj);
    sig.emit();
    EXPECT_EQ(order, (std::vector<int>{1, 2, 3}));
}

// ── dispatch to methods of two different classes ──────────────────────────────
// The callback receives std::any* (pointer to any object).
// Use std::any_cast<T>(obj) to get T* or throw, or std::any_cast<T*>(obj) to get T** (rare).

TEST(SignalTest, CallMethodsOfTwoDifferentObjects) {
    struct Cat { void meow() { ++meows; } int meows = 0; };
    struct Dog { void bark() { ++barks; } int barks = 0; };

    std::vector<std::string> log;

    Signal<> sig([&](std::any* obj) -> bool {
        try {
            Cat* cat = std::any_cast<Cat*>(*obj);  // dereference std::any*, then cast to Cat*
            cat->meow();
            log.push_back("cat");
            return true;
        } catch (const std::bad_any_cast&) {
            // not a Cat
        }
        return false;
    });
    sig.add_callback([&](std::any* obj) -> bool {
        try {
            Dog* dog = std::any_cast<Dog*>(*obj);
            dog->bark();
            log.push_back("dog");
            return true;
        } catch (const std::bad_any_cast&) {
            // not a Dog
        }
        return false;
    });

    Cat cat;
    Dog dog;

    std::any cat_any = &cat;
    std::any dog_any = &dog;
    sig.connect(&cat_any);
    sig.connect(&dog_any);

    sig.emit();

    EXPECT_EQ(log, (std::vector<std::string>{"cat", "dog"}));
}
