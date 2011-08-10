void pMenu::append(Action &action) {
  if(parentWindow) parentWindow->p.updateMenu();
}

void pMenu::setText(const string &text) {
  if(parentWindow) parentWindow->p.updateMenu();
}

void pMenu::constructor() {
  hmenu = 0;
}

//Windows actions lack the ability to toggle visibility.
//To support this, menus must be destroyed and recreated when toggling any action's visibility.
void pMenu::update(Window &parentWindow, HMENU parentMenu) {
  this->parentWindow = &parentWindow;
  this->parentMenu = parentMenu;
  if(hmenu) DestroyMenu(hmenu);
  hmenu = CreatePopupMenu();

  foreach(action, menu.state.action) {
    action.p.parentWindow = &parentWindow;
    action.p.parentMenu = hmenu;

    unsigned enabled = action.state.enabled ? 0 : MF_GRAYED;
    if(dynamic_cast<Menu*>(&action)) {
      Menu &item = (Menu&)action;
      item.p.update(parentWindow, hmenu);
      AppendMenu(hmenu, MF_STRING | MF_POPUP | enabled, (UINT_PTR)item.p.hmenu, utf16_t(item.state.text));
    } else if(dynamic_cast<Separator*>(&action)) {
      Separator &item = (Separator&)action;
      if(action.state.visible) AppendMenu(hmenu, MF_SEPARATOR | enabled, item.p.id, L"");
    } else if(dynamic_cast<Item*>(&action)) {
      Item &item = (Item&)action;
      if(action.state.visible) AppendMenu(hmenu, MF_STRING | enabled, item.p.id, utf16_t(item.state.text));
    } else if(dynamic_cast<CheckItem*>(&action)) {
      CheckItem &item = (CheckItem&)action;
      if(action.state.visible) AppendMenu(hmenu, MF_STRING | enabled, item.p.id, utf16_t(item.state.text));
      if(item.state.checked) item.setChecked();
    } else if(dynamic_cast<RadioItem*>(&action)) {
      RadioItem &item = (RadioItem&)action;
      if(action.state.visible) AppendMenu(hmenu, MF_STRING | enabled, item.p.id, utf16_t(item.state.text));
      if(item.state.checked) item.setChecked();
    }
  }
}
