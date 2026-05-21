#include "mainwindow.h"
#include <iostream>

const char* MAIN_CSS = R"(
    window {
        background-color: #0d0e12;
        color: #e2e8f0;
        font-family: 'Inter', 'Segoe UI', sans-serif;
    }
    .main-box {
        padding: 16px;
    }
    .video-frame {
        background-color: #000000;
        border: 2px solid #1f2937;
        border-radius: 12px;
        box-shadow: 0 10px 25px rgba(0, 0, 0, 0.7);
    }
    .sidebar {
        background-color: #111827;
        border-radius: 12px;
        padding: 20px;
        border: 1px solid #1f2937;
    }
    .title-label {
        font-size: 22px;
        font-weight: 800;
        color: #3b82f6;
        margin-bottom: 4px;
        text-shadow: 0 0 10px rgba(59, 130, 246, 0.3);
    }
    .subtitle-label {
        font-size: 12px;
        color: #9ca3af;
        margin-bottom: 24px;
    }
    .btn-select {
        background: linear-gradient(135deg, #3b82f6, #1d4ed8);
        border: none;
        color: white;
        border-radius: 8px;
        padding: 12px 20px;
        font-weight: bold;
        font-size: 14px;
        box-shadow: 0 4px 15px rgba(29, 78, 216, 0.4);
    }
    .btn-select:hover {
        background: linear-gradient(135deg, #60a5fa, #2563eb);
        box-shadow: 0 6px 20px rgba(37, 99, 235, 0.6);
    }
    .btn-ctrl {
        background: #1f2937;
        border: 1px solid #374151;
        color: #e5e7eb;
        border-radius: 8px;
        padding: 10px 16px;
        font-weight: bold;
        font-size: 13px;
        transition: all 0.2s ease;
    }
    .btn-ctrl:hover {
        background: #374151;
        color: white;
        box-shadow: 0 0 10px rgba(255, 255, 255, 0.1);
    }
    .btn-ctrl:disabled {
        opacity: 0.3;
        border-color: #1f2937;
        color: #4b5563;
    }
    .badge {
        font-size: 11px;
        font-weight: bold;
        padding: 4px 10px;
        border-radius: 6px;
        text-transform: uppercase;
        border: 1px solid transparent;
    }
    .badge-idle {
        background-color: #374151;
        color: #9ca3af;
        border-color: #4b5563;
    }
    .badge-playing {
        background-color: rgba(16, 185, 129, 0.2);
        color: #10b981;
        border-color: rgba(16, 185, 129, 0.4);
    }
    .badge-paused {
        background-color: rgba(245, 158, 11, 0.2);
        color: #f59e0b;
        border-color: rgba(245, 158, 11, 0.4);
    }
    .timecode {
        font-family: 'Courier New', monospace;
        font-size: 20px;
        font-weight: bold;
        color: #60a5fa;
        text-shadow: 0 0 8px rgba(96, 165, 250, 0.3);
    }
    progressbar trough {
        background-color: #1f2937;
        border-radius: 4px;
        min-height: 8px;
    }
    progressbar progress {
        background: linear-gradient(to right, #3b82f6, #60a5fa);
        border-radius: 4px;
    }
    switch {
        border-radius: 12px;
    }
    switch:checked {
        background-color: #3b82f6;
    }
)";

MainWindow::MainWindow(GtkApplication* app) {
    // Bind Source
    m_videoWidget.setSource(&m_lFile);

    applyCSS();
    createUI(app);
}

MainWindow::~MainWindow() {
    m_lFile.stop();
}

void MainWindow::applyCSS() {
    GtkCssProvider* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, MAIN_CSS, -1);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(provider);
}

void MainWindow::createUI(GtkApplication* app) {
    // Create Main Window
    m_window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(m_window), "LinuxMediaLibrary - Premium GTK4 Preview");
    gtk_window_set_default_size(GTK_WINDOW(m_window), 1050, 640);

    // Main layout container (Horizontal)
    GtkWidget* mainBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    gtk_widget_set_margin_start(mainBox, 16);
    gtk_widget_set_margin_end(mainBox, 16);
    gtk_widget_set_margin_top(mainBox, 16);
    gtk_widget_set_margin_bottom(mainBox, 16);
    gtk_widget_add_css_class(mainBox, "main-box");
    gtk_window_set_child(GTK_WINDOW(m_window), mainBox);

    // LEFT PANEL: Video Container
    GtkWidget* videoBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_hexpand(videoBox, TRUE);
    gtk_widget_set_vexpand(videoBox, TRUE);
    gtk_box_append(GTK_BOX(mainBox), videoBox);

    GtkWidget* videoFrame = gtk_aspect_frame_new(0.5, 0.5, 16.0/9.0, FALSE);
    gtk_widget_add_css_class(videoFrame, "video-frame");
    gtk_widget_set_hexpand(videoFrame, TRUE);
    gtk_widget_set_vexpand(videoFrame, TRUE);
    gtk_box_append(GTK_BOX(videoBox), videoFrame);

    // Get GTK Video Widget and add to container
    GtkWidget* videoWidget = m_videoWidget.getWidget();
    gtk_widget_set_hexpand(videoWidget, TRUE);
    gtk_widget_set_vexpand(videoWidget, TRUE);
    gtk_aspect_frame_set_child(GTK_ASPECT_FRAME(videoFrame), videoWidget);

    // Progress Bar (below video)
    m_progressBar = gtk_progress_bar_new();
    gtk_widget_set_hexpand(m_progressBar, TRUE);
    gtk_box_append(GTK_BOX(videoBox), m_progressBar);

    // RIGHT PANEL: Sidebar / Controls
    GtkWidget* sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_size_request(sidebar, 320, -1);
    gtk_widget_add_css_class(sidebar, "sidebar");
    gtk_box_append(GTK_BOX(mainBox), sidebar);

    // Header inside sidebar
    GtkWidget* title = gtk_label_new("Playout Preview");
    gtk_widget_add_css_class(title, "title-label");
    gtk_widget_set_halign(title, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(sidebar), title);

    GtkWidget* subtitle = gtk_label_new("GTK4 GPU Accelerated Engine");
    gtk_widget_add_css_class(subtitle, "subtitle-label");
    gtk_widget_set_halign(subtitle, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(sidebar), subtitle);

    // File Selector section
    GtkWidget* selectBtn = gtk_button_new_with_label("📁 SELECT VIDEO FILE");
    gtk_widget_add_css_class(selectBtn, "btn-select");
    g_signal_connect(selectBtn, "clicked", G_CALLBACK(onSelectFileClicked), this);
    gtk_box_append(GTK_BOX(sidebar), selectBtn);

    // Active File Label
    m_fileLabel = gtk_label_new("No file loaded");
    gtk_widget_set_margin_top(m_fileLabel, 10);
    gtk_widget_set_margin_bottom(m_fileLabel, 20);
    gtk_label_set_ellipsize(GTK_LABEL(m_fileLabel), PANGO_ELLIPSIZE_MIDDLE);
    gtk_widget_set_halign(m_fileLabel, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(sidebar), m_fileLabel);

    // Divider Line
    GtkWidget* sep1 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_margin_bottom(sep1, 20);
    gtk_box_append(GTK_BOX(sidebar), sep1);

    // Playback Controls Row
    GtkWidget* ctrlBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_box_append(GTK_BOX(sidebar), ctrlBox);

    m_playBtn = gtk_button_new_with_label("▶ PLAY");
    gtk_widget_add_css_class(m_playBtn, "btn-ctrl");
    gtk_widget_set_hexpand(m_playBtn, TRUE);
    g_signal_connect(m_playBtn, "clicked", G_CALLBACK(onPlayClicked), this);
    gtk_widget_set_sensitive(m_playBtn, FALSE);
    gtk_box_append(GTK_BOX(ctrlBox), m_playBtn);

    m_pauseBtn = gtk_button_new_with_label("⏸ PAUSE");
    gtk_widget_add_css_class(m_pauseBtn, "btn-ctrl");
    gtk_widget_set_hexpand(m_pauseBtn, TRUE);
    g_signal_connect(m_pauseBtn, "clicked", G_CALLBACK(onPauseClicked), this);
    gtk_widget_set_sensitive(m_pauseBtn, FALSE);
    gtk_box_append(GTK_BOX(ctrlBox), m_pauseBtn);

    m_stopBtn = gtk_button_new_with_label("⬛ STOP");
    gtk_widget_add_css_class(m_stopBtn, "btn-ctrl");
    gtk_widget_set_hexpand(m_stopBtn, TRUE);
    g_signal_connect(m_stopBtn, "clicked", G_CALLBACK(onStopClicked), this);
    gtk_widget_set_sensitive(m_stopBtn, FALSE);
    gtk_box_append(GTK_BOX(ctrlBox), m_stopBtn);

    // Status / Timecode Display Card
    GtkWidget* statusCard = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(statusCard, 24);
    gtk_widget_set_margin_bottom(statusCard, 24);
    gtk_box_append(GTK_BOX(sidebar), statusCard);

    // Status Row (Status + Badge)
    GtkWidget* statusRow = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_box_append(GTK_BOX(statusCard), statusRow);

    GtkWidget* statusTitle = gtk_label_new("Engine Status:");
    gtk_widget_set_halign(statusTitle, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(statusRow), statusTitle);

    m_statusBadge = gtk_label_new("Idle");
    gtk_widget_add_css_class(m_statusBadge, "badge");
    gtk_widget_add_css_class(m_statusBadge, "badge-idle");
    gtk_box_append(GTK_BOX(statusRow), m_statusBadge);

    // Timecode Label
    m_timecodeLabel = gtk_label_new("00:00:00.000 / 00:00:00.000");
    gtk_widget_add_css_class(m_timecodeLabel, "timecode");
    gtk_widget_set_halign(m_timecodeLabel, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(statusCard), m_timecodeLabel);

    // Info/Resolution Label
    m_infoLabel = gtk_label_new("Format: --");
    gtk_widget_set_halign(m_infoLabel, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(statusCard), m_infoLabel);

    // Divider Line
    GtkWidget* sep2 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_margin_bottom(sep2, 20);
    gtk_box_append(GTK_BOX(sidebar), sep2);

    // Settings Grid
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 12);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_box_append(GTK_BOX(sidebar), grid);

    // Row 1: Aspect Ratio
    GtkWidget* aspectLabel = gtk_label_new("Maintain Aspect Ratio");
    gtk_widget_set_halign(aspectLabel, GTK_ALIGN_START);
    GtkWidget* aspectSwitch = gtk_switch_new();
    gtk_switch_set_active(GTK_SWITCH(aspectSwitch), FALSE);
    g_signal_connect(aspectSwitch, "state-set", G_CALLBACK(onAspectRatioToggled), this);
    gtk_grid_attach(GTK_GRID(grid), aspectLabel, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), aspectSwitch, 1, 0, 1, 1);

    // Row 2: VU Meters
    GtkWidget* vuLabel = gtk_label_new("Enable Audio Meters");
    gtk_widget_set_halign(vuLabel, GTK_ALIGN_START);
    GtkWidget* vuSwitch = gtk_switch_new();
    gtk_switch_set_active(GTK_SWITCH(vuSwitch), FALSE);
    g_signal_connect(vuSwitch, "state-set", G_CALLBACK(onVUMeterToggled), this);
    gtk_grid_attach(GTK_GRID(grid), vuLabel, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), vuSwitch, 1, 1, 1, 1);

    // Row 3: Overlay Timecode
    GtkWidget* overlayTcLabel = gtk_label_new("Show Overlay Timecode");
    gtk_widget_set_halign(overlayTcLabel, GTK_ALIGN_START);
    GtkWidget* tcSwitch = gtk_switch_new();
    gtk_switch_set_active(GTK_SWITCH(tcSwitch), FALSE);
    g_signal_connect(tcSwitch, "state-set", G_CALLBACK(onTimecodeToggled), this);
    gtk_grid_attach(GTK_GRID(grid), overlayTcLabel, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), tcSwitch, 1, 2, 1, 1);

    // Show window and trigger update timeout
    gtk_window_present(GTK_WINDOW(m_window));
    g_timeout_add(33, updateUI, this);
}

gboolean MainWindow::updateUI(gpointer data) {
    MainWindow* self = (MainWindow*)data;
    if (!self->m_lFile.getWidth()) return TRUE;

    // 1. Position and Duration
    double pos = self->m_lFile.PosGet();
    double dur = self->m_lFile.getDurationMs();
    
    // 2. Format Timecode
    int posSec = (int)(pos / 1000.0);
    int posMs = (int)(pos) % 1000;
    int posMin = posSec / 60;
    int posHour = posMin / 60;
    posSec %= 60;
    posMin %= 60;
    
    int durSec = (int)(dur / 1000.0);
    int durMin = durSec / 60;
    int durHour = durMin / 60;
    durSec %= 60;
    durMin %= 60;
    
    char tcBuf[256];
    snprintf(tcBuf, sizeof(tcBuf), "%02d:%02d:%02d.%03d / %02d:%02d:%02d.%03d",
             posHour, posMin, posSec, posMs,
             durHour, durMin, durSec, (int)dur % 1000);
    gtk_label_set_text(GTK_LABEL(self->m_timecodeLabel), tcBuf);
    
    // 3. Progress Bar
    if (dur > 0) {
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(self->m_progressBar), pos / dur);
    } else {
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(self->m_progressBar), 0.0);
    }
    
    // 4. Status badge and overlay state
    int status = 0;
    self->m_lFile.statusGet(status);
    
    GtkStyleContext* ctx = gtk_widget_get_style_context(self->m_statusBadge);
    gtk_style_context_remove_class(ctx, "badge-idle");
    gtk_style_context_remove_class(ctx, "badge-playing");
    gtk_style_context_remove_class(ctx, "badge-paused");
    
    if (status == 1) { // Playing
        gtk_label_set_text(GTK_LABEL(self->m_statusBadge), "Playing");
        gtk_style_context_add_class(ctx, "badge-playing");
        self->m_videoWidget.setStatus("program");
    } else if (status == 2) { // Paused
        gtk_label_set_text(GTK_LABEL(self->m_statusBadge), "Paused");
        gtk_style_context_add_class(ctx, "badge-paused");
        self->m_videoWidget.setStatus("preview");
    } else { // Stopped / Idle
        gtk_label_set_text(GTK_LABEL(self->m_statusBadge), "Idle");
        gtk_style_context_add_class(ctx, "badge-idle");
        self->m_videoWidget.setStatus("none");
    }
    
    // 5. Information labels
    int w = self->m_lFile.getWidth();
    int h = self->m_lFile.getHeight();
    double fps = self->m_lFile.getFPS();
    if (w > 0 && h > 0) {
        char infoBuf[128];
        snprintf(infoBuf, sizeof(infoBuf), "%d x %d @ %.2f FPS", w, h, fps);
        gtk_label_set_text(GTK_LABEL(self->m_infoLabel), infoBuf);
    } else {
        gtk_label_set_text(GTK_LABEL(self->m_infoLabel), "Format: --");
    }
    
    return TRUE;
}

void MainWindow::onSelectFileClicked(GtkWidget* button, gpointer data) {
    MainWindow* self = (MainWindow*)data;
    GtkFileChooserNative* chooser = gtk_file_chooser_native_new(
        "Select Video File",
        GTK_WINDOW(self->m_window),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Open",
        "_Cancel"
    );
    
    GtkFileFilter* filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Video Files");
    gtk_file_filter_add_mime_type(filter, "video/*");
    gtk_file_filter_add_pattern(filter, "*.mp4");
    gtk_file_filter_add_pattern(filter, "*.mkv");
    gtk_file_filter_add_pattern(filter, "*.avi");
    gtk_file_filter_add_pattern(filter, "*.mov");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);

    g_signal_connect(chooser, "response", G_CALLBACK(onFileChooserResponse), self);
    gtk_native_dialog_show(GTK_NATIVE_DIALOG(chooser));
}

void MainWindow::onFileChooserResponse(GtkNativeDialog* dialog, int response_id, gpointer data) {
    MainWindow* self = (MainWindow*)data;
    if (response_id == GTK_RESPONSE_ACCEPT) {
        GtkFileChooser* chooser = GTK_FILE_CHOOSER(dialog);
        GFile* file = gtk_file_chooser_get_file(chooser);
        if (file) {
            char* path = g_file_get_path(file);
            if (path) {
                std::cout << "[PreviewGTK] Loading File: " << path << std::endl;
                self->m_lFile.fileNameSet(path);
                
                std::string filename = path;
                size_t lastSlash = filename.find_last_of('/');
                if (lastSlash != std::string::npos) {
                    filename = filename.substr(lastSlash + 1);
                }
                self->m_videoWidget.setName(filename);
                gtk_label_set_text(GTK_LABEL(self->m_fileLabel), filename.c_str());
                
                // Play immediately
                self->m_lFile.play();
                
                // Enable controls
                gtk_widget_set_sensitive(self->m_playBtn, TRUE);
                gtk_widget_set_sensitive(self->m_pauseBtn, TRUE);
                gtk_widget_set_sensitive(self->m_stopBtn, TRUE);
                
                g_free(path);
            }
            g_object_unref(file);
        }
    }
    g_object_unref(dialog);
}

void MainWindow::onPlayClicked(GtkWidget* button, gpointer data) {
    MainWindow* self = (MainWindow*)data;
    self->m_lFile.play();
}

void MainWindow::onPauseClicked(GtkWidget* button, gpointer data) {
    MainWindow* self = (MainWindow*)data;
    self->m_lFile.pause();
}

void MainWindow::onStopClicked(GtkWidget* button, gpointer data) {
    MainWindow* self = (MainWindow*)data;
    self->m_lFile.stop();
}

void MainWindow::onAspectRatioToggled(GtkSwitch* self, gboolean state, gpointer data) {
    MainWindow* window = (MainWindow*)data;
    window->m_videoWidget.setMaintainAspectRatio(state);
}

void MainWindow::onVUMeterToggled(GtkSwitch* self, gboolean state, gpointer data) {
    MainWindow* window = (MainWindow*)data;
    window->m_videoWidget.enableMeter(state);
}

void MainWindow::onTimecodeToggled(GtkSwitch* self, gboolean state, gpointer data) {
    MainWindow* window = (MainWindow*)data;
    window->m_videoWidget.enableTimecode(state);
}
