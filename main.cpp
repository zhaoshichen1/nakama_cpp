#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include "nakama-cpp/Nakama.h"
#include "json.hpp"
#include "nakama-cpp/StrUtil.h"

using bytes=std::vector<char>;
class Client {
private:
    Nakama::NClientPtr m_client;
    Nakama::NRtClientPtr m_rt;
    Nakama::NSessionPtr m_season;
//    Nakama::NRtClientListenerPtr m_listen;
    Nakama::NRtDefaultClientListener m_listen;
    std::string email, password, username;
    bool m_realtime;
public:
    Client(const std::string &email = "test@test.com", const std::string &password = "testpass")
            : email(email), password(password), username(email) {
        Nakama::NClientParameters parameters{};
        m_client = Nakama::createRestClient(parameters);
    }

    void signUp() {
        m_client->authenticateEmail(email, password, username, true, {},
                                    [&](Nakama::NSessionPtr ptr) {
                                        Nakama::NLogger::Debug(ptr->getAuthToken(), "signUp-authenticateEmail");
                                    }, [](const Nakama::NError &err) {
                    NLOG_DEBUG(email);
                    Nakama::NLogger::Debug(err.message, "signUp-authenticateEmail");
                });
    }

    void signIn() {
        m_client->authenticateEmail(email, password, "", false, {},
                                    [&](Nakama::NSessionPtr ptr) {
                                        m_season = ptr;
                                        rt();
                                        Nakama::NLogger::Debug(ptr->getAuthToken(), "signIn-authenticateEmail");
                                    }, [](const Nakama::NError &err) {
                    NLOG_DEBUG(email);
                    Nakama::NLogger::Debug(err.message, "signIn-authenticateEmail");
                });
    }

    void rt() {
        m_rt = m_client->createRtClient(Nakama::DEFAULT_PORT);
        m_listen.setConnectCallback([this]() {
            auto msg = "connect";
            m_client->setUserData(const_cast<char *>(msg));
            join("test-room");
            m_realtime=true;
        });
        m_listen.setChannelMessageCallback([this](const Nakama::NChannelMessage&msg){
            Nakama::NLogger::Debug(msg.content,"m_listen-getmsg");
        });

        m_listen.setNotificationsCallback([this](const Nakama::NNotificationList&list){
            for(auto &notify:list.notifications){
                Nakama::NLogger::Debug(notify.id+":"+notify.content,"notify-callback");
            }
        });

        m_rt->setListener(&m_listen);
        m_rt->connect(m_season, true);
    }

    void connect() {
        signUp();
        signIn();
    }
    void join(const std::string&room){
        m_rt->joinChat(room,Nakama::NChannelType::ROOM,{},{},[&](Nakama::NChannelPtr ptr){
            Nakama::NLogger::Debug(ptr->roomName, "join-joinChat");
            for(auto &i:ptr->presences){
                Nakama::NLogger::Debug(i.username, "join");
            }
            m_rt->writeChatMessage(ptr->id,u8"{\"test\":\"233\"}");
        },[](const Nakama::NRtError&err){
            Nakama::NLogger::Debug(err.message, "join-joinChat");
        });

        Nakama::NLogger::Debug("send hello", "rpc-test");
//        m_rt->rpc("world",u8"{\"test\":2}");
//        m_client->rpc(m_season,"hello");
    }
    void send(const std::string&msg) {
        auto jstr=R"( {"abdefawdc":"233"} )";
//        auto jstr=R"( {"abc":"233"} )";
//        auto jstr=R"( {"abcdefghijk":"233"} )";
//        auto j=nlohmann::json::parse(jstr);
//        std::cout<<j.dump(2)<<std::endl;
        if (m_realtime) {
//            auto data=std::string(j.dump());
#if 1
            m_client->rpc(m_season,"hello","jstr22222222222",[](const Nakama::NRpc&rpc){
                Nakama::NLogger::Debug(rpc.payload, "m_client-hello-callback");
            });
#endif
#if 0
            m_rt->rpc("hello",jstr, [&](const Nakama::NRpc &rpc) {
                Nakama::NLogger::Debug(rpc.payload, "hello-callback");
            });
            m_rt->rpc("bar", jstr, [&](const Nakama::NRpc &rpc) {
                Nakama::NLogger::Debug(rpc.payload, "bar-callback");
            });
#endif
        }
    }

    void tick() {
        while (true) {
            m_client->tick();
            if (m_rt) {
                m_rt->tick();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
};

int main() {

    Nakama::NLogger::initWithConsoleSink(Nakama::NLogLevel::Debug);

    Client c1{"test1@test.com"};

    c1.connect();

    auto t3 = std::thread{[&]() {
        while (true) {
            c1.send("233333");
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }};
    auto t1 = std::thread{[&]() { c1.tick(); }};

    Client c2{"test2@test.com"};
    c2.connect();
    auto t2 = std::thread{[&]() { c2.tick(); }};

    auto t4 = std::thread{[&]() {
        while (true) {
            c2.send("322222");
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }};

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "fuck" << std::endl;

    return 0;
}