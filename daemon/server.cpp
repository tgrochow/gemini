#include <server.hpp>
#include <iostream>

namespace gemini
{

  server::server() :
  QObject(),
  update_timer_frequency_(200),
  update_counter_(0),
  update_frequency_(5)
  {
    intf_info_server    = new QLocalServer(this);
    rule_set_server     = new QLocalServer(this);
    rule_update_socket_ = new QLocalSocket(this);
  }

  server::~server()
  {
    // stop listening for connections
    intf_info_server->close();
    rule_set_server->close();

    delete intf_info_server;
    delete rule_set_server;

    delete rule_update_socket_;
  }

  bool server::start()
  {
    bool valid_start = true;
    control_.rule_set_.load("default.rules");


    // register handle for rule updates
    connect(rule_update_socket_,SIGNAL(readyRead()),
            this               ,SLOT(read_rule_update()));

    // remove old server file
    QLocalServer::removeServer("gemini_interface_info");
    QLocalServer::removeServer("gemini_rule_set");

    // register handle of interface info requests
    connect(intf_info_server,SIGNAL(newConnection()),
            this,            SLOT(send_intf_info()));

    // register handle of rule set requests
    connect(rule_set_server,SIGNAL(newConnection()),
            this,            SLOT(send_rule_set()));


    // server doesn't listen connections
    if(!intf_info_server->listen("gemini_interface_info"))
    {
      valid_start = false;
    }

    else if(!rule_set_server->listen("gemini_rule_set"))
    {
      valid_start = false;
    }

    // correct initialization
    else
    {
      // init update
      update_counter_ = update_frequency_;

      // start update timer
      QTimer::singleShot(update_timer_frequency_,this,SLOT(update()));
    }


    return valid_start;
  }

  void server::send_intf_info() const
  {
    std::vector<std::string> interface_strings = control_.interface_info();

    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);

    out.setVersion(QDataStream::Qt_5_0);

    // mark the beginning of the block
    out << (quint16)0;

    // stream interface info strings in block
    for(unsigned short index = 0 ; index < interface_strings.size() ; ++index)
    {
      out << interface_strings[index].c_str();
    }

    // set index back to the beginning of the block
    out.device()->seek(0);
    // stream block size in
    out << (quint16)(block.size() - sizeof(quint16));


    // get actual connection
    QLocalSocket * client_connection =

    intf_info_server->nextPendingConnection();

    // register destruction of connection after usage
    connect(client_connection , SIGNAL(disconnected()),
            client_connection , SLOT(deleteLater())    );


    client_connection->write(block);
    client_connection->flush();
    client_connection->disconnectFromServer();
  }

  void server::send_rule_set() const
  {
    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);

    out.setVersion(QDataStream::Qt_5_0);


    // mark the beginning of the block
    out << (quint16)0;

    // stream rule set
    out << control_.rule_set_;

    // set index back to the beginning of the block
    out.device()->seek(0);
    // stream block size in

    out << static_cast<quint16>(block.size() - sizeof(quint16));

    // get actual connection
    QLocalSocket * client_connection =

    rule_set_server->nextPendingConnection();

    // register destruction of connection after usage
    connect(client_connection , SIGNAL(disconnected()),
            client_connection , SLOT(deleteLater())    );


    client_connection->write(block);
    client_connection->flush();
    client_connection->disconnectFromServer();
  }


  void server::read_rule_update()
  {
    control_.rule_set_.clear();


    quint16 block_size = 0;

    QDataStream in(rule_update_socket_);

    in.setVersion(QDataStream::Qt_5_0);


    if(rule_update_socket_->bytesAvailable() >

       static_cast<int> (sizeof(quint16)))
    {
      in >> block_size;

      char * info_buffer = new char[block_size];


      while(!in.atEnd())
      {
        in >> info_buffer;

        control_.rule_set_.push_back(rule(info_buffer));
      }


      delete[] info_buffer;
    }

    // save rule set
    control_.rule_set_.save();
  }


  void server::update()
  {
    bool client_update = false;

    if(update_counter_ >= update_frequency_)
    {
      client_update = true;
      update_counter_ = 0;
    }

    // abort old connection
    rule_update_socket_->abort();

    // load rule updates
    rule_update_socket_->connectToServer("gemini_rule_update");

    control_.enforce_rule_set(client_update);

    QTimer::singleShot(update_timer_frequency_,this,SLOT(update()));

    ++update_counter_;
  }
}
