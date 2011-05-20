struct Font::State {
  bool bold;
  string family;
  bool italic;
  unsigned size;
  bool underline;

  State() {
    bold = false;
    italic = false;
    size = 8;
    underline = false;
  }
};

struct Window::State {
  bool backgroundColor;
  unsigned backgroundColorRed, backgroundColorGreen, backgroundColorBlue;
  bool fullScreen;
  Geometry geometry;
  reference_array<Layout&> layout;
  reference_array<Menu&> menu;
  Font *menuFont;
  bool menuVisible;
  bool resizable;
  Font *statusFont;
  string statusText;
  bool statusVisible;
  string title;
  bool visible;
  reference_array<Widget&> widget;
  Font *widgetFont;

  State() {
    backgroundColor = false;
    backgroundColorRed = 0;
    backgroundColorGreen = 0;
    backgroundColorBlue = 0;
    fullScreen = false;
    geometry = { 128, 128, 256, 256 };
    menuFont = 0;
    menuVisible = false;
    resizable = true;
    statusVisible = false;
    visible = false;
    widgetFont = 0;
  }
};

struct Action::State {
  bool enabled;
  Window *parent;
  bool visible;

  State() {
    enabled = true;
    parent = 0;
    visible = true;
  }
};

struct Menu::State {
  reference_array<Action&> action;
  string text;
};

struct Item::State {
  string text;
};

struct CheckItem::State {
  bool checked;
  string text;

  State() {
    checked = false;
  }
};

struct RadioItem::State {
  bool checked;
  reference_array<RadioItem&> group;
  string text;

  State() {
    checked = true;
  }
};

struct Widget::State {
  bool abstract;
  bool enabled;
  Font *font;
  Geometry geometry;
  bool visible;

  State() {
    abstract = false;
    enabled = true;
    font = 0;
    geometry = { 0, 0, 0, 0 };
    visible = true;
  }
};

struct Button::State {
  string text;

  State() {
  }
};

struct CheckBox::State {
  bool checked;
  string text;

  State() {
    checked = false;
  }
};

struct ComboBox::State {
  unsigned selection;
  linear_vector<string> text;

  State() {
    selection = 0;
  }
};

struct HexEdit::State {
  unsigned columns;
  unsigned length;
  unsigned offset;
  unsigned rows;

  State() {
    columns = 16;
    length = 0;
    offset = 0;
    rows = 16;
  }
};

struct HorizontalSlider::State {
  unsigned length;
  unsigned position;

  State() {
    length = 101;
    position = 0;
  }
};

struct Label::State {
  string text;
};

struct LineEdit::State {
  bool editable;
  string text;

  State() {
    editable = true;
  }
};

struct ListView::State {
  bool checkable;
  array<bool> checked;
  lstring headerText;
  bool headerVisible;
  bool selected;
  unsigned selection;
  linear_vector<lstring> text;

  State() {
    checkable = false;
    headerVisible = false;
    selected = false;
    selection = 0;
  }
};

struct ProgressBar::State {
  unsigned position;

  State() {
    position = 0;
  }
};

struct RadioBox::State {
  bool checked;
  reference_array<RadioBox&> group;
  string text;

  State() {
    checked = true;
  }
};

struct TextEdit::State {
  unsigned cursorPosition;
  bool editable;
  string text;
  bool wordWrap;

  State() {
    cursorPosition = 0;
    editable = true;
    wordWrap = false;
  }
};

struct VerticalSlider::State {
  unsigned length;
  unsigned position;

  State() {
    length = 101;
    position = 0;
  }
};
