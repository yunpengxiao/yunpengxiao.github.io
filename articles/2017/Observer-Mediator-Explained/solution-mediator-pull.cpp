//clang++ solution-mediator-pull.cpp -std=c++14 -o test && ./test && rm test

#include <memory>
#include <string>
#include <vector>
#include <iostream>

#include "observable-widget-pull.h"

/*
    Mediator-Pull (override Library)
*/

//using MyItem = unsigned;
using MyItem = std::string;

using MyTextBox = ObservableTextBox<MyItem>;
using MyListBox = ObservableListBox<MyItem>;

class Mediator :
    public MyTextBox::Observer,
    public MyListBox::Observer
{
    std::shared_ptr<MyTextBox> _textbox;
    std::shared_ptr<MyListBox> _listbox;

public:
    Mediator (std::shared_ptr<MyTextBox> &textbox,
              std::shared_ptr<MyListBox> &listbox)
        : _textbox (textbox), _listbox (listbox) {}

    void TextUpdated () override {
        _listbox->SetSelection (_textbox->GetText ());
    }

    void SelectionChanged () override {
        _textbox->SetText (_listbox->GetSelection ());
    }
};

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
    auto mediator = std::make_shared<Mediator> (textbox, listbox);

    textbox->SetObserver (mediator);
    listbox->SetObserver (mediator);

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
