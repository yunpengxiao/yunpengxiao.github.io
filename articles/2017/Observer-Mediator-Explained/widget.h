/*
    Widget Library
*/

#ifndef WIDGET
#define WIDGET

template<typename Item>
class TextBox {
    Item _item;

public:
    TextBox (const Item &item) : _item (item) {}

    void SetText (const Item &item) { _item = item; }
    const Item &GetText () const { return _item; }

    // Interface for Invoker
    virtual void OnInput () = 0;
};

template<typename Item>
class ListBox {
    std::vector<Item> _items;
    unsigned _index;

public:
    ListBox (const std::vector<Item> &items,
             unsigned index = 0)
        : _items (items), _index (index) {}

    void SetSelection (const Item &item) {
        for (size_t i = 0; i < _items.size (); i++)
            if (_items[i] == item) {
                _index = i;
                break;
            }
    }
    const Item &GetSelection () const { return _items.at (_index); }

    // Interface for Invoker
    virtual void OnChange () = 0;
};

#endif // !WIDGET
