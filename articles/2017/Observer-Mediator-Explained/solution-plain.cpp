//clang++ solution-plain.cpp -std=c++14 -o test && ./test && rm test

#include <memory>
#include <string>
#include <vector>
#include <iostream>

#include "widget.h"

/*
    My Widget (override Library)
*/

//using MyItem = unsigned;
using MyItem = std::string;

class MyTextBox;
class MyListBox;

class MyTextBox : public TextBox<MyItem> {
    std::weak_ptr<MyListBox> _listbox;

public:
    MyTextBox (const MyItem &item) : TextBox (item) {}

    void SetListBox (std::weak_ptr<MyListBox> &&p) { _listbox = p; }
    void OnInput () override;
};

class MyListBox : public ListBox<MyItem> {
    std::weak_ptr<MyTextBox> _textbox;

public:
    MyListBox (const std::vector<MyItem> &items,
               unsigned index = 0)
        : ListBox (items, index) {}

    void SetTextBox (std::weak_ptr<MyTextBox> &&p) { _textbox = p; }
    void OnChange () override;
};

void MyTextBox::OnInput () {
    if (auto p = _listbox.lock ())
        p->SetSelection (this->GetText ());
}

void MyListBox::OnChange () {
    if (auto p = _textbox.lock ())
        p->SetText (this->GetSelection ());
}

int main () {
    /*
        Client
    */

    //std::vector<MyItem> items { 10, 11, 12 };
    std::vector<MyItem> items {
        MyItem { "My Item 0" },
        MyItem { "My Item 1" },
        MyItem { "My Item 2" },
    };

    auto textbox = std::make_shared<MyTextBox> (items[0]);
    auto listbox = std::make_shared<MyListBox> (items);

    textbox->SetListBox (listbox);
    listbox->SetTextBox (textbox);

    /*
        Invoker (async with Client)
    */

    auto check = [&] {
        const auto &text = textbox->GetText ();
        const auto &selection = listbox->GetSelection ();
        if (text == selection)
            std::cout << "Sync: " << text << std::endl;
        else
            std::cout << "Sync Failed\n";
    };

    textbox->SetText (items[1]);
    textbox->OnInput ();
    check ();

    listbox->SetSelection (items[2]);
    listbox->OnChange ();
    check ();

    return 0;
}
