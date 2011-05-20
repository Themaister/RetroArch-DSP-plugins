#include "gtk.hpp"

#include "settings.cpp"
#include "font.cpp"
#include "message-window.cpp"
#include "window.cpp"

#include "action/action.cpp"
#include "action/menu.cpp"
#include "action/separator.cpp"
#include "action/item.cpp"
#include "action/check-item.cpp"
#include "action/radio-item.cpp"

#include "widget/widget.cpp"
#include "widget/button.cpp"
#include "widget/check-box.cpp"
#include "widget/combo-box.cpp"
#include "widget/hex-edit.cpp"
#include "widget/horizontal-slider.cpp"
#include "widget/label.cpp"
#include "widget/line-edit.cpp"
#include "widget/list-view.cpp"
#include "widget/progress-bar.cpp"
#include "widget/radio-box.cpp"
#include "widget/text-edit.cpp"
#include "widget/vertical-slider.cpp"
#include "widget/viewport.cpp"

Geometry pOS::availableGeometry() {
  //TODO: is there a GTK+ function for this?
  //should return desktopGeometry() sans panels, toolbars, docks, etc.
  Geometry geometry = desktopGeometry();
  return { geometry.x + 64, geometry.y + 64, geometry.width - 128, geometry.height - 128 };
}

Geometry pOS::desktopGeometry() {
  return {
    0, 0,
    gdk_screen_get_width(gdk_screen_get_default()),
    gdk_screen_get_height(gdk_screen_get_default())
  };
}

static string pOS_fileDialog(bool save, Window &parent, const string &path, const lstring &filter) {
  string name;

  GtkWidget *dialog = gtk_file_chooser_dialog_new(
    save == 0 ? "Load File" : "Save File",
    &parent != &Window::None ? GTK_WINDOW(parent.p.widget) : (GtkWindow*)0,
    save == 0 ? GTK_FILE_CHOOSER_ACTION_OPEN : GTK_FILE_CHOOSER_ACTION_SAVE,
    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
    (const gchar*)0
  );

  if(path) gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), path);

  foreach(filterItem, filter) {
    GtkFileFilter *gtkFilter = gtk_file_filter_new();
    gtk_file_filter_set_name(gtkFilter, filterItem);
    lstring part;
    part.split("(", filterItem);
    part[1].rtrim<1>(")");
    lstring list;
    list.split(",", part[1]);
    foreach(pattern, list) gtk_file_filter_add_pattern(gtkFilter, pattern);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), gtkFilter);
  }

  if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    char *temp = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    name = temp;
    g_free(temp);
  }

  gtk_widget_destroy(dialog);
  return name;
}

string pOS::fileLoad(Window &parent, const string &path, const lstring &filter) {
  return pOS_fileDialog(0, parent, path, filter);
}

string pOS::fileSave(Window &parent, const string &path, const lstring &filter) {
  return pOS_fileDialog(1, parent, path, filter);
}

string pOS::folderSelect(Window &parent, const string &path) {
  string name;

  GtkWidget *dialog = gtk_file_chooser_dialog_new(
    "Select Folder",
    &parent != &Window::None ? GTK_WINDOW(parent.p.widget) : (GtkWindow*)0,
    GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
    (const gchar*)0
  );

  if(path) gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), path);

  if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    char *temp = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    name = temp;
    g_free(temp);
  }

  gtk_widget_destroy(dialog);
  if(name == "") return "";
  if(name.endswith("/") == false) name.append("/");
  return name;
}

void pOS::main() {
  gtk_main();
}

bool pOS::pendingEvents() {
  return gtk_events_pending();
}

void pOS::processEvents() {
  while(pendingEvents()) gtk_main_iteration_do(false);
}

void pOS::quit() {
  settings.save();
  gtk_main_quit();
}

void pOS::initialize() {
  settings.load();

  int argc = 1;
  char *argv[2];
  argv[0] = new char[8];
  argv[1] = 0;
  strcpy(argv[0], "phoenix");
  char **argvp = argv;
  gtk_init(&argc, &argvp);

  gtk_rc_parse_string(
    "style \"phoenix-gtk\"\n"
    "{\n"
    "  GtkComboBox::appears-as-list = 1\n"
    "  GtkTreeView::vertical-separator = 0\n"
    "}\n"
    "class \"GtkComboBox\" style \"phoenix-gtk\"\n"
    "class \"GtkTreeView\" style \"phoenix-gtk\"\n"
  );
}
