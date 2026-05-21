#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <gtk/gtk.h>
#include "LFile.h"
#include "LVideoWidgetGTK.h"
#include <string>

class MainWindow {
public:
    MainWindow(GtkApplication* app);
    ~MainWindow();

    GtkWidget* getWindow() { return m_window; }

private:
    void createUI(GtkApplication* app);
    void applyCSS();

    // Signal handlers (GTK callbacks must be static or C-compatible functions)
    static void onSelectFileClicked(GtkWidget* button, gpointer data);
    static void onFileChooserResponse(GtkNativeDialog* dialog, int response_id, gpointer data);
    static void onPlayClicked(GtkWidget* button, gpointer data);
    static void onPauseClicked(GtkWidget* button, gpointer data);
    static void onStopClicked(GtkWidget* button, gpointer data);
    static void onAspectRatioToggled(GtkSwitch* self, gboolean state, gpointer data);
    static void onVUMeterToggled(GtkSwitch* self, gboolean state, gpointer data);
    static void onTimecodeToggled(GtkSwitch* self, gboolean state, gpointer data);
    static gboolean updateUI(gpointer data);

    // GTK4 Widgets
    GtkWidget* m_window = nullptr;
    GtkWidget* m_playBtn = nullptr;
    GtkWidget* m_pauseBtn = nullptr;
    GtkWidget* m_stopBtn = nullptr;
    GtkWidget* m_timecodeLabel = nullptr;
    GtkWidget* m_progressBar = nullptr;
    GtkWidget* m_statusBadge = nullptr;
    GtkWidget* m_fileLabel = nullptr;
    GtkWidget* m_infoLabel = nullptr;

    // SDK Components
    LFile m_lFile;
    LVideoWidgetGTK m_videoWidget;
};

#endif // MAINWINDOW_H
