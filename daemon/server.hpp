#ifndef GEMINI_SERVER
#define GEMINI_SERVER

#include <QtNetwork>

#include <control.hpp>


namespace gemini
{
  class server : public QObject
  {
    Q_OBJECT

    public :

    server();
    ~server();

    bool start();


    private slots :

    void update();
    void send_intf_info() const;
    void send_rule_set() const;
    void read_rule_update();


    private :

    // server
    QLocalServer * intf_info_server,
                 * rule_set_server;

    // sockets
    QLocalSocket * rule_update_socket_;

    // timer update parameter
    unsigned short update_timer_frequency_;

    // update parameter
    unsigned short update_counter_,
                   update_frequency_;

    // gemini control
    control control_;
  };

}

#endif // GEMINI_SERVER
