#ifndef UI_MAIN_WINDOW
#define UI_MAIN_WINDOW

// std
#include <map>
// Qt
#include <QtNetwork>
#include <QComboBox>
#include <QMainWindow>
#include <QMutex>
// gemini
#include <rule_info.hpp>
// class
#include "ui_main_window.h"


namespace Ui
{
  class main_window;
}

class main_window : public QMainWindow
{
  Q_OBJECT


  public :

  main_window(QWidget * parent = nullptr);
  ~main_window();

  bool init() const;


  private slots :

  // network
  void read_interface_info();
  void read_rule_set();
  void send_rule_set();
  void trigger_device_update() const;

  // content update
  void update_rule_node(int row,int collumn);
  void update_class(int class_index);
  void update_permission(int permission_index);

  // error handling
  void socket_error(QLocalSocket::LocalSocketError);

  // interaction
  void add_rule();
  void create_interface_rule(QTreeWidgetItem * clicked_item);
  void remove_rule();
  void rule_up();
  void rule_down();
  void server_upload();


  private :

  // initialization
  void init_button_icons()            const;
  void init_interaction_connection()  const;
  void init_network()                 const;
  void init_rule_table_column_width() const;
  void init_rule_table_header()       const;
  void init_timer()                   const;

  // network
  std::vector<std::string> const read_stream(QLocalSocket * local_socket) const;

  // content update
  void add_rule(gemini::rule_info const& rule);
  void add_device(gemini::device_info const& info);
  void update_device_tree(std::vector<gemini::device_info> & update);
  void update_priority(unsigned short row,bool up);
  void update_rule_table();

  // content intialization
  QComboBox * class_combo_box() const;
  QComboBox * permission_combo_box() const;


  // class member

  static const std::string ANY;


  //object member

  // interface
  Ui::main_window * ui;
  QIcon             icon_enabled_,
                    icon_disabled_;

  //network
  QLocalSocket    * intf_info_socket_,
                  * rule_set_socket_;
  QLocalServer    * rule_upload_server_;

  //timer
  QTimer          * update_timer_;
  unsigned short    update_frequency_;

  // condition
  bool              upload_rules_,
                    server_connection_;

  // thread safety
  QMutex            mutex_rule_transfer_;

  // gemini
  std::vector<gemini::rule_info>                  rule_nodes_;
  std::map<gemini::device_info,QTreeWidgetItem *> device_nodes_;
};

#endif // UI_MAIN_WINDOW
