
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

class Command {
 public:
  virtual ~Command() = default;
  virtual void Run() = 0;  // indirection
};

class NewCommand : public Command {
 public:
  NewCommand(std::unique_ptr<File>& file) : file_(file) {}
  void Run() override { file_.get() = File::New(); }

 private:
  std::reference_wrapper<std::unique_ptr<File>> file_;  // context
};

class OpenCommand : public Command {
 public:
  OpenCommand(std::unique_ptr<File>& file) : file_(file) {}
  void Run() override { file_.get() = File::Open(); }

 private:
  std::reference_wrapper<std::unique_ptr<File>> file_;  // context
};

class SaveCommand : public Command {
 public:
  SaveCommand(const std::unique_ptr<File>& file) : file_(file) {}
  void Run() override {
    if (!file_.get())
      return;
    file_.get()->Save();
  }

 private:
  std::reference_wrapper<const std::unique_ptr<File>> file_;  // context
};

class Client {
  // Receiver
 private:
  std::unique_ptr<File> file_;  // data storage

  // Indirection
 private:
  std::unordered_map<Action, std::unique_ptr<Command>> handlers_;

 public:
  Client() {
    handlers_.emplace(ClickNew, std::make_unique<NewCommand>(std::ref(file_)));
    handlers_.emplace(ClickOpen,
                      std::make_unique<OpenCommand>(std::ref(file_)));
    handlers_.emplace(ClickSave,
                      std::make_unique<SaveCommand>(std::cref(file_)));
  }

  // Sender
 public:
  void OnAction() const {
    Action action /* = ... */;
    handlers_.at(action)->Run();
  }
};

// Message Flow:
//           Run                         Save
// (Sender) -----> SaveCommand(Command) ------> File(Receiver)
