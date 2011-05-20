void pLineEdit::setEditable(bool editable) {
  SendMessage(hwnd, EM_SETREADONLY, editable == false, 0);
}

void pLineEdit::setText(const string &text) {
  locked = true;
  SetWindowText(hwnd, utf16_t(text));
  locked = false;
}

string pLineEdit::text() {
  unsigned length = GetWindowTextLength(hwnd);
  wchar_t text[length + 1];
  GetWindowText(hwnd, text, length + 1);
  text[length] = 0;
  return utf8_t(text);
}

void pLineEdit::constructor() {
  setParent(Window::None);
}

void pLineEdit::setParent(Window &parent) {
  if(hwnd) DestroyWindow(hwnd);
  hwnd = CreateWindowEx(
    WS_EX_CLIENTEDGE, L"EDIT", L"",
    WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
    0, 0, 0, 0, parent.p.hwnd, (HMENU)id, GetModuleHandle(0), 0
  );
  SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)&lineEdit);
  setDefaultFont();
  setEditable(lineEdit.state.editable);
  setText(lineEdit.state.text);
}
