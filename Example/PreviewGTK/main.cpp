#include <gtk/gtk.h>
#include <iostream>
#include "mainwindow.h"

static void on_app_activate(GtkApplication* app, gpointer user_data) {
    std::cout << "[PreviewGTK] Activating Application Window..." << std::endl;
    // Instantiate modular MainWindow
    MainWindow* window = new MainWindow(app);
    
    // Store pointer in application to clean up on exit
    g_object_set_data_full(G_OBJECT(app), "main-window", window, [](gpointer data) {
        delete (MainWindow*)data;
    });
}

int main(int argc, char* argv[]) {
    std::cout << "[PreviewGTK] Starting modular GTK4 Playout App..." << std::endl;

    // Create GtkApplication with non-unique flags to bypass DBus registration issues
    GtkApplication* app = gtk_application_new("com.lml.playout.previewgtk", G_APPLICATION_NON_UNIQUE);
    g_signal_connect(app, "activate", G_CALLBACK(on_app_activate), nullptr);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    std::cout << "[PreviewGTK] Modular GTK4 Playout App stopped." << std::endl;
    return status;
}
