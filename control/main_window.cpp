#include "main_window.hpp"
#include <iostream>


// initialize static class member

const std::string main_window::ANY("*");


// public : constructor
main_window::main_window(QWidget * parent) :
QMainWindow(parent),
ui(new Ui::main_window),
icon_enabled_(":/icons/enabled"),
icon_disabled_(":/icons/disabled"),
intf_info_socket_(new QLocalSocket(this)),
rule_set_socket_(new QLocalSocket(this)),
rule_upload_server_(new QLocalServer(this)),
update_timer_(new QTimer(this)),
update_frequency_(1000),
upload_rules_(false),
server_connection_(true)
{
  ui->setupUi(this);

  setWindowTitle("Gemini Control");
}



// public : destructor
main_window::~main_window()
{
  // stop listening for connections
  rule_upload_server_->close();

  // stop update timer
  update_timer_->stop();


  // deallocate memory
  delete ui;
  delete intf_info_socket_;
  delete rule_set_socket_;
  delete rule_upload_server_;
  delete update_timer_;
}



// public : intialization
bool main_window::init() const
{
  bool valid_start = true;


  init_button_icons();
  init_interaction_connection();
  init_network();
  init_rule_table_header();
  init_rule_table_column_width();
  init_timer();


  // server doesn't listen connections
  if(!rule_upload_server_->listen("gemini_rule_update"))
  {
    valid_start = false;
  }

  else
  {
    // init device tree
    trigger_device_update();
  }


  return valid_start;
}



// private SLOT : network
void main_window::read_interface_info()
{
  std::vector<gemini::device_info> device_info;

  std::vector<std::string> input(read_stream(intf_info_socket_));


  // for every input string
  for(auto input_it = input.begin() ; input_it != input.end() ; ++input_it)
  {
    // parse device info
    device_info.push_back(*input_it);
  }

  // update the device tree with new device info
  update_device_tree(device_info);

  if(!server_connection_)
  {
    QSize label_size(ui->server_icon_label->sizeHint());

    QPixmap error_pixmap(icon_enabled_.pixmap(label_size));

    ui->server_icon_label->setPixmap(error_pixmap);

    // abort previos connections to rule set server
    rule_set_socket_->abort();
    // read init rule set
    rule_set_socket_->connectToServer("gemini_rule_set");

    server_connection_ = true;
  }
}



// private SLOT : network
void main_window::read_rule_set()
{
  std::vector<std::string> input(read_stream(rule_set_socket_));

  ui->rule_table->setRowCount(input.size());

  // delete previos rule set
  rule_nodes_.clear();

  // for every input string
  for(auto input_it = input.begin() ; input_it != input.end() ; ++input_it)
  {
    // parse rule
    rule_nodes_.push_back(*input_it);
  }

  update_rule_table();
}



// private SLOT : network
void main_window::send_rule_set()
{
  // lock the rule set server for rule transfer
  mutex_rule_transfer_.lock();

  // get actual connection
  QLocalSocket * client_connection =

  rule_upload_server_->nextPendingConnection();

  // register destruction of connection after usage
  connect(client_connection , SIGNAL(disconnected()),
          client_connection , SLOT(deleteLater())    );

  if(upload_rules_ && server_connection_)
  {
    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);

    out.setVersion(QDataStream::Qt_5_0);

    // mark the beginning of the block
    out << (quint16)0;

    for(auto rule_it  = rule_nodes_.begin() ;
             rule_it != rule_nodes_.end()   ; ++rule_it)
    {
      out << rule_it->rule_string().c_str();
    }


    // set index back to the beginning of the block
    out.device()->seek(0);
    // stream block size in
    out << (quint16)(block.size() - sizeof(quint16));


    client_connection->write(block);
    client_connection->flush();

    upload_rules_ = false;
  }

  client_connection->disconnectFromServer();

  // unlock server for new other transfers
  mutex_rule_transfer_.unlock();
}



// private SLOT : network
void main_window::trigger_device_update() const
{
  // abort old connection
  intf_info_socket_->abort();

  intf_info_socket_->connectToServer("gemini_interface_info");
}



// private SLOT : content update
void main_window::update_rule_node(int row,int collumn)
{
  // get current table item
  QTableWidgetItem * table_item(ui->rule_table->item(row,collumn));

  // get text of current item
  QString cell_text(table_item->text());

  unsigned short cell_value(0);

  bool conversion_correct(true);


  if(cell_text == ANY.c_str())
  {
    rule_nodes_[row].values_[collumn] = 0;
  }

  else
  {
    try
    {
      cell_value = std::stoi(cell_text.toStdString());
    }

    catch(std::invalid_argument const&)
    {
      table_item->setTextColor(Qt::red);

      conversion_correct = false;
    }

    catch(std::out_of_range const&)
    {
      table_item->setTextColor(Qt::red);

      conversion_correct = false;
    }
  }


  if(conversion_correct)
  {
    rule_nodes_[row].values_[collumn] = cell_value;

    if(cell_value == 0) table_item->setText(ANY.c_str());

    table_item->setTextColor(Qt::black);
  }
}



// private SLOT : content update
void main_window::update_class(int)
{
   QComboBox * class_combo_box;

   unsigned short class_index;

  for(unsigned short row = 0 ; row < ui->rule_table->rowCount() ; ++row)
  {
    class_combo_box = reinterpret_cast<QComboBox *>

                      (ui->rule_table->cellWidget(row,RCLASS));


    class_index = static_cast<unsigned short> (class_combo_box->currentIndex());


    if(class_index > 3) ++class_index;

    rule_nodes_[row].values_[RCLASS] = class_index;
  }
}



// private SLOT : content update
void main_window::update_permission(int)
{
  QComboBox * permission_combo_box;

  bool permission;

  for(unsigned short row = 0 ; row < ui->rule_table->rowCount() ; ++row)
  {
    permission_combo_box = reinterpret_cast<QComboBox *>

                           (ui->rule_table->cellWidget(row,RPERMISSION));


    permission = static_cast<bool> (permission_combo_box->currentIndex());

    rule_nodes_[row].values_[RPERMISSION] = permission;
  }
}



// private SLOT : error handling
void main_window::socket_error(QLocalSocket::LocalSocketError socket_error)
{
  if(socket_error != QLocalSocket::PeerClosedError)
  {
    QSize label_size(ui->server_icon_label->sizeHint());

    QPixmap error_pixmap(icon_disabled_.pixmap(label_size));

    ui->server_icon_label->setPixmap(error_pixmap);

    server_connection_ = false;
  }

  // start timer for next update trigger
  update_timer_->start();
}



// private SLOT : interaction
void main_window::add_rule()
{
  add_rule(gemini::rule_info());
}



// private SLOT : interaction
void main_window::create_interface_rule(QTreeWidgetItem * clicked_item)
{
  QTreeWidgetItem * parent = clicked_item->parent();

  if(parent != nullptr)
  {
    for(auto device_node_it  = device_nodes_.begin() ;
             device_node_it != device_nodes_.end()   ; ++device_node_it)
    {
      if(device_node_it->second == parent)
      {
        gemini::device_info parent_info(device_node_it->first);

        add_rule(gemini::rule_info(parent_info));

        ui->tab_widget->setCurrentWidget(ui->tab_rule_editor);
        ui->rule_table->setCurrentCell(0,0);

        break;
      }
    }
  }
}



// private SLOT : interaction
void main_window::remove_rule()
{
  unsigned short row_index = ui->rule_table->currentRow();

  ui->rule_table->removeRow(row_index);

  rule_nodes_.erase(rule_nodes_.begin() + row_index);
}



// private SLOT : interaction
void main_window::rule_up()
{
  int row = ui->rule_table->currentRow();

  if(row > 0)
  {
    update_priority(row,true);
  }
}



// private SLOT : interaction
void main_window::rule_down()
{
  int row = ui->rule_table->currentRow();

  if(row >= 0 && row < ui->rule_table->rowCount() - 1)
  {
    update_priority(row,false);
  }
}



// private SLOT : interaction
void main_window::server_upload()
{
  upload_rules_ = true;
}



// private : initialization
void main_window::init_button_icons() const
{
  ui->rule_up_button->setArrowType(Qt::UpArrow);
  ui->rule_down_button->setArrowType(Qt::DownArrow);
  ui->rule_add_button->setIcon(QIcon(":/icons/add"));
  ui->rule_remove_button->setIcon(QIcon(":/icons/remove"));
  ui->server_upload_button->setIcon(QIcon(":/icons/upload"));
}



// private : initialization
void main_window::init_interaction_connection() const
{
  connect(ui->server_upload_button,SIGNAL(clicked(bool)),
          this                    ,SLOT(server_upload()) );

  connect(ui->rule_add_button,SIGNAL(clicked(bool)),
          this               ,SLOT(add_rule())      );

  connect(ui->rule_remove_button,SIGNAL(clicked(bool)),
          this                  ,SLOT(remove_rule())   );

  connect(ui->rule_up_button,SIGNAL(clicked(bool)),
          this              ,SLOT(rule_up())       );

  connect(ui->rule_down_button,SIGNAL(clicked(bool)),
          this                ,SLOT(rule_down())     );

  connect(ui->device_tree,SIGNAL(itemDoubleClicked(QTreeWidgetItem *,int)),
          this           ,SLOT(create_interface_rule(QTreeWidgetItem *))   );
}



// private : initialization
void main_window::init_network() const
{
  // remove old server files
  QLocalServer::removeServer("gemini_rule_update");


  connect(intf_info_socket_,SIGNAL(readyRead()),
          this             ,SLOT(read_interface_info()));

  connect(intf_info_socket_,SIGNAL(error(QLocalSocket::LocalSocketError)),
          this             ,SLOT(socket_error(QLocalSocket::LocalSocketError)));

  connect(rule_set_socket_,SIGNAL(readyRead()),
          this            ,SLOT(read_rule_set()));

  connect(rule_upload_server_,SIGNAL(newConnection()),
          this               ,SLOT(send_rule_set()))  ;


  // abort previos connections to rule set server
  rule_set_socket_->abort();
  // read init rule set
  rule_set_socket_->connectToServer("gemini_rule_set");
}



// private : initialization
void main_window::init_rule_table_header() const
{
  QStringList table_header;

  table_header << "bus" << "port" << "vendor" << "product"
               << "interface class"
               << "permission";

  ui->rule_table->setColumnCount(table_header.size());
  ui->rule_table->setHorizontalHeaderLabels(table_header);
}



// private : initialization
void main_window::init_rule_table_column_width() const
{
  const unsigned short value_column_width(65),
                       class_column_width(126),
                       permission_column_width(110);

  unsigned short width(0);

  for(unsigned short column = RBUS ; column != RUNDEFINED ; ++column)
  {
    if(column < RCLASS)
    {
      width = value_column_width;
    }

    else if(column == RCLASS)
    {
      width = class_column_width;
    }

    else if(column == RPERMISSION)
    {
      width = permission_column_width;
    }

    ui->rule_table->setColumnWidth(column,width);
  }
}



// private : initialization
void main_window::init_timer() const
{
  update_timer_->setSingleShot(true);
  update_timer_->setInterval(update_frequency_);

  connect(update_timer_,SIGNAL(timeout()),this,SLOT(trigger_device_update()));
}



// private : network
std::vector<std::string> const main_window::

read_stream(QLocalSocket * local_socket) const
{
  std::vector<std::string> input;


  quint16 block_size = 0;

  QDataStream in(local_socket);

  in.setVersion(QDataStream::Qt_5_0);


  if(local_socket->bytesAvailable() > static_cast<int> (sizeof(quint16)))
  {
    in >> block_size;

    char * info_buffer = new char[block_size];


    while(!in.atEnd())
    {
      in >> info_buffer;

      input.push_back(std::string(info_buffer));
    }


    delete[] info_buffer;
  }


  return input;
}



// private : content update
void main_window::add_rule(gemini::rule_info const& rule)
{
  rule_nodes_.insert(rule_nodes_.begin(),rule);

  unsigned short row(0);

  ui->rule_table->insertRow(row);

  QTableWidgetItem * rule_value_item;

  QString collumn_text;

  unsigned short class_index;


  for(unsigned short collumn = RBUS ; collumn != RUNDEFINED ; ++collumn)
  {
    if(collumn < RCLASS)
    {
      rule_value_item = new QTableWidgetItem(ANY.c_str());

      rule_value_item->setTextAlignment(Qt::AlignCenter);


      if(rule.values_[collumn] == 0)
      {
        collumn_text = ANY.c_str();
      }

      else
      {
        collumn_text = QString::number(rule.values_[collumn]);
      }

      rule_value_item->setText(collumn_text);

      ui->rule_table->setItem(row,collumn,rule_value_item);
    }

    else if(collumn == RCLASS)
    {
      QComboBox * class_item = class_combo_box();


      class_index = rule.values_[RCLASS];

      if(class_index > 3) --class_index;

      class_item->setCurrentIndex(class_index);


      connect(class_item,SIGNAL(currentIndexChanged(int)),
              this      ,SLOT(update_class(int))          );

      ui->rule_table->setCellWidget(row , collumn , class_item);
    }

    else if(collumn == RPERMISSION)
    {
      QComboBox * permission_item = permission_combo_box();

      permission_item->setCurrentIndex(rule.permission_);

      connect(permission_item,SIGNAL(currentIndexChanged(int)),
              this           ,SLOT(update_permission(int))     );

      ui->rule_table->setCellWidget(row , collumn , permission_item);
    }
  }
}



// private : content update
void main_window::add_device(gemini::device_info const& info)
{
  QTreeWidgetItem * device_item = new QTreeWidgetItem(ui->device_tree);

  // get device description
  QString device_text(info.device_strings_[gemini::PRODUCT_STRING].c_str()),
          vendor_text(info.device_strings_[gemini::VENDOR_STRING].c_str());

  // set description
  device_item->setText(0,device_text);
  device_item->setText(1,vendor_text);

  // for every interface
  for(auto intf_it  = info.intf_settings_.begin() ;
           intf_it != info.intf_settings_.end()   ; ++intf_it)
  {
    QTreeWidgetItem * intf_item = new QTreeWidgetItem(device_item);

    // for every interface setting
    for(auto setting_it  = intf_it->first.begin() ;
             setting_it != intf_it->first.end()   ; ++setting_it)
    {
      // get actual text on interface item
      QString intf_text(intf_item->text(0));

      // if the actual setting isn't the first, append seperator
      if(setting_it != intf_it->first.begin()) intf_text + " | ";

      // add interface class of setting
      intf_text += gemini::device_info::class_string(*(setting_it)).c_str();

      // set concatenated text
      intf_item->setText(0,intf_text);
    }

    // set permission icon
    if(intf_it->second) intf_item->setIcon(0,icon_enabled_);
    else                intf_item->setIcon(0,icon_disabled_);

    // add complete interface item to device item
    device_item->addChild(intf_item);
  }

  // add device item to device tree
  ui->device_tree->addTopLevelItem(device_item);

  device_item->setExpanded(true);


  // memorize pair of device item and info
  device_nodes_.insert(std::make_pair(info,device_item));
}



// private : content update
void main_window::

update_device_tree(std::vector<gemini::device_info> & update)
{
  // for every device present on server
  for(auto device_info(update.begin()) ;
           device_info != update.end() ; ++device_info)
  {
    // find node in device tree
    auto device_node(device_nodes_.find(*(device_info)));


    // device isn't in tree
    if(device_node == device_nodes_.end())
    {
      add_device(*(device_info));
    }

    // device is in tree
    else
    {
      unsigned short intf_id = 0;

      QTreeWidgetItem * intf_item;


      // for every interface on device
      for(auto intf_it  = device_info->intf_settings_.begin() ;
               intf_it != device_info->intf_settings_.end()   ; ++intf_it)
      {
        if(device_node->second->childCount() < intf_id + 1)
        {
          QTreeWidgetItem * parent = device_node->second;

          device_node->second->addChild(new QTreeWidgetItem(parent));
        }

        intf_item = device_node->second->child(intf_id);

        // reset text
        intf_item->setText(0,"");

        // for every interface setting
        for(auto setting_it  = intf_it->first.begin() ;
                 setting_it != intf_it->first.end()   ; ++setting_it)
        {
          // get actual text on interface item
          QString intf_text(intf_item->text(0));

          // if the actual setting isn't the first, append seperator
          if(setting_it != intf_it->first.begin()) intf_text + " | ";

          // add interface class of setting
          intf_text += gemini::device_info::class_string(*(setting_it)).c_str();

          // set concatenated text
          intf_item->setText(0,intf_text);
        }

        // set permission icon
        if(intf_it->second) intf_item->setIcon(0,icon_enabled_);
        else                intf_item->setIcon(0,icon_disabled_);

        ++intf_id;
      }

      if(intf_id < device_node->second->childCount())
      {
        for(unsigned short child_id = intf_id ;
                           child_id < device_node->second->childCount() ;
                         ++child_id)
        {
          intf_item = device_node->second->child(child_id);

          // delete removed interfaces
          device_node->second->removeChild(intf_item);
        }
      }

      QTreeWidgetItem * device = device_node->second;
      device_nodes_.erase(device_node->first);
      device_nodes_.insert(std::make_pair(*device_info,device));
    }
  }

  // for every node in device tree
  for(auto device_it  = device_nodes_.begin() ;
           device_it != device_nodes_.end()   ; ++device_it)
  {
    // find device present on server
    auto device_info(std::find(update.begin(),update.end(),device_it->first));

    // node isn't present on server
    if(device_info == update.end())
    {
      // remove device from device tree
      delete device_it->second;

      device_nodes_.erase(device_it);
    }
  }
}



// private : content update
void main_window::update_priority(unsigned short row,bool up)
{
  unsigned short prev_row = row;

  if(up) --prev_row;
  else   ++prev_row;


  QTableWidgetItem * item , * item_prev;

  QComboBox * combo_item  , * combo_item_prev;


  QString prev;

  int     prev_index;


  for(unsigned short collumn = 0 ; collumn != RCLASS ; ++collumn)
  {
    item_prev = ui->rule_table->item(row     , collumn);
    item      = ui->rule_table->item(prev_row , collumn);

    prev = item_prev->text();

    item_prev->setText(item->text());
    item->setText(prev);
  }

  for(unsigned short collumn = RCLASS ; collumn != RUNDEFINED ; ++collumn)
  {
    combo_item      = reinterpret_cast<QComboBox *>

                      (ui->rule_table->cellWidget(row     , collumn));

    combo_item_prev = reinterpret_cast<QComboBox *>

                      (ui->rule_table->cellWidget(prev_row , collumn));

    prev_index = combo_item_prev->currentIndex();

    combo_item_prev->setCurrentIndex(combo_item->currentIndex());
    combo_item->setCurrentIndex(prev_index);
  }

  ui->rule_table->setCurrentCell(prev_row,ui->rule_table->currentColumn());
}



// private : content update
void main_window::update_rule_table()
{
  QTableWidgetItem * rule_value_item;


  unsigned short rule_index = 0,
                 class_index;

  QString token;


  for(auto rule_it  = rule_nodes_.begin() ;
           rule_it != rule_nodes_.end()   ; ++rule_it)
  {
    for(unsigned short collumn = RBUS ; collumn != RUNDEFINED ; ++collumn)
    {
      if(collumn < RCLASS)
      {
        if(rule_it->values_[collumn] == RMASKED)
        {
          token = ANY.c_str();
        }

        else
        {
          token = QString::number(rule_it->values_[collumn]);
        }


        rule_value_item = new QTableWidgetItem(token);

        rule_value_item->setTextAlignment(Qt::AlignCenter);


        ui->rule_table->setItem(rule_index,collumn,rule_value_item);
      }

      else if(collumn == RCLASS)
      {
        QComboBox * class_item = class_combo_box();


        class_index = rule_it->values_[RCLASS];

        if(class_index > 3) --class_index;

        class_item->setCurrentIndex(class_index);


        connect(class_item,SIGNAL(currentIndexChanged(int)),
                this      ,SLOT(update_class(int))          );

        ui->rule_table->setCellWidget(rule_index,collumn,class_item);
      }

      else if(collumn == RPERMISSION)
      {
        QComboBox * permission_item = permission_combo_box();

        permission_item->setCurrentIndex(rule_it->permission_);

        connect(permission_item,SIGNAL(currentIndexChanged(int)),
                this      ,SLOT(update_permission(int))           );

        ui->rule_table->setCellWidget(rule_index,collumn,permission_item);
      }
    }

    ++rule_index;
  }


  connect(ui->rule_table,SIGNAL(cellChanged(int,int)),
          this          ,SLOT(update_rule_node(int,int)));
}



// private : content initialization
QComboBox * main_window::class_combo_box() const
{
  QComboBox * class_combo_box = new QComboBox(ui->rule_table);

  class_combo_box->addItem(ANY.c_str());

  class_combo_box->addItem("AUDIO");
  class_combo_box->addItem("COMMUNICATION");
  class_combo_box->addItem("HID");
  class_combo_box->addItem("PHYSICAL");
  class_combo_box->addItem("PRINTER");
  class_combo_box->addItem("PTP");
  class_combo_box->addItem("MASS STORAGE");
  class_combo_box->addItem("HUB");
  class_combo_box->addItem("DATA");
  class_combo_box->addItem("SMART CARD");
  class_combo_box->addItem("CONTENT SECURITY");
  class_combo_box->addItem("VIDEO");
  class_combo_box->addItem("PERSONAL HEALTHCARE");
  class_combo_box->addItem("DIAGNOSTIC DEVICE");
  class_combo_box->addItem("WIRELESS");
  class_combo_box->addItem("APPLICATION");
  class_combo_box->addItem("VENDOR SPEC");


  return class_combo_box;
}



// private : content initialization
QComboBox *  main_window::permission_combo_box() const
{
  QComboBox * permission_combo_box = new QComboBox(ui->rule_table);

  permission_combo_box->addItem(icon_disabled_,"prohibited");
  permission_combo_box->addItem(icon_enabled_,"permitted");


  return permission_combo_box;
}
