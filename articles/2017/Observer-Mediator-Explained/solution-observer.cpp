//clang++ solution-observer.cpp -std=c++14 -o test && ./test && rm test

#include <memory>
#include <string>
#include <vector>
#include <iostream>

#include "observable-widget.h"

/*
    My Observable Widget (override Library)
*/

//using MyItem = unsigned;
using MyItem = std::string;

class MyTextBox :
    public ObservableTextBox<MyItem>,
    public ObservableListBox<MyItem>::Observer
{
public:
    MyTextBox (const MyItem &item) : ObservableTextBox (item) {}

    void SelectionChanged (const MyItem &item) override {
        this->SetText (item);
    }
};

class MyListBox :
    public ObservableListBox<MyItem>,
    public ObservableTextBox<MyItem>::Observer
{
public:
    MyListBox (const std::vector<MyItem> &items,
               unsigned index = 0)
        : ObservableListBox (items, index) {}

    void TextUpdated (const MyItem &item) override {
        this->SetSelection (item);
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

    textbox->SetObserver (listbox);
    listbox->SetObserver (textbox);

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
