void pTextEdit::setCursorPosition(unsigned position) {
  Edit_SetSel(hwnd, position, position);
}

void pTextEdit::setEditable(bool editable) {
  SendMessage(hwnd, EM_SETREADONLY, editable == false, (LPARAM)0);
}

void pTextEdit::setText(const string &text) {
  locked = true;
  string output = text;
  output.replace("\r", "");
  output.replace("\n", "\r\n");
  SetWindowText(hwnd, utf16_t(output));
  locked = false;
}

void pTextEdit::setWordWrap(bool wordWrap) {
  //ES_AUTOHSCROLL cannot be changed after widget creation.
  //As a result, we must destroy and re-create widget to change this setting.
  HWND hwndParent = GetParent(hwnd);
  Object *object = (Object*)GetWindowLongPtr(hwndParent, GWLP_USERDATA);
  if(object == 0) return;
  if(dynamic_cast<Window*>(object)) setParent(((Window&)*object));
}

string pTextEdit::text() {
  unsigned length = GetWindowTextLength(hwnd);
  wchar_t buffer[length + 1];
  GetWindowText(hwnd, buffer, length + 1);
  buffer[length] = 0;
  string text = (const char*)utf8_t(buffer);
  text.replace("\r", "");
  return text;
}

void pTextEdit::constructor() {
  setParent(Window::None);
}

void pTextEdit::setParent(Window &parent) {
  if(hwnd) DestroyWindow(hwnd);
  hwnd = CreateWindowEx(
    WS_EX_CLIENTEDGE, L"EDIT", L"",
    WS_CHILD | WS_VISIBLE | ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN | (textEdit.state.wordWrap == false ? ES_AUTOHSCROLL : 0),
    0, 0, 0, 0, parent.p.hwnd, (HMENU)id, GetModuleHandle(0), 0
  );
  SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)&textEdit);
  setDefaultFont();
  setCursorPosition(textEdit.state.cursorPosition);
  setEditable(textEdit.state.editable);
  setText(textEdit.state.text);
}
