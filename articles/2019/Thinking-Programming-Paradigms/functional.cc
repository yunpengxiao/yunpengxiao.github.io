
#include <functional>
#include <memory>
#include <unordered_map>

enum Action {
  ClickNew,
  ClickOpen,
  ClickSave,
};

class File {
 public:
  static std::unique_ptr<File> New();   // opreation
  static std::unique_ptr<File> Open();  // opreation
  void Save() const;                    // opreation

 private:
  // data
};

using Command = std::function<void()>;

class Client {
  // Receiver
 private:
  std::unique_ptr<File> file_;  // data storage

  // Indirection
 private:
  std::unordered_map<Action, Command> handlers_ = {
      {ClickNew, [this] { file_ = File::New(); }},
      {ClickOpen, [this] { file_ = File::Open(); }},
      {ClickSave, [this] { if (file_) file_->Save(); }},
  };

  // Sender
 public:
  void OnAction() const {
    Action action /* = ... */;
    handlers_.at(action)();
  }
};
