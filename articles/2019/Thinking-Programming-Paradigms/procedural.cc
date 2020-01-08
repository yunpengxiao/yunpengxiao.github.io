
enum Action {
  ClickNew,
  ClickOpen,
  ClickSave,
};

struct File {
  // data
};

bool NewFile(File** file);        // opreation
bool OpenFile(File** file);       // opreation
bool SaveFile(const File* file);  // opreation

// Receiver

File* g_current_file;  // data storage

// Sender

void OnAction() {
  Action action /* = ... */;
  if (action == ClickNew) {
    NewFile(&g_current_file);
  } else if (action == ClickOpen) {
    OpenFile(&g_current_file);
  } else if (action == ClickSave) {
    SaveFile(g_current_file);
  }
}
