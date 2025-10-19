#include <gtk/gtk.h>
#include <string.h>
#include <ctype.h>
#include "logic.h"

GtkWidget ***entries = NULL;
Graph currentGraph;

void on_window_destroy(GtkWidget *widget, gpointer data) {
    // Marcar parámetros como no utilizados para evitar warnings
    (void)widget;
    (void)data;
    gtk_main_quit();
}

// callback function to validate the numeric entry (0 or 1)
void on_entry_insert_text(GtkEditable *editable, gchar *new_text, gint new_text_length, 
                         gint *position, gpointer user_data) {
    // avoid warnings
    (void)user_data;
    (void)position;
    
    // only allow one character
    if (gtk_entry_get_text_length(GTK_ENTRY(editable)) >= 1) {
        g_signal_stop_emission_by_name(editable, "insert-text");
        return;
    }
    
    // validate that the number is 0 or 1
    for (int i = 0; i < new_text_length; i++) {
        if (!isdigit(new_text[i]) || (new_text[i] != '0' && new_text[i] != '1')) {
            g_signal_stop_emission_by_name(editable, "insert-text");
            return;
        }
    }
}

// function to load a graph into a binary file
void on_load_button_clicked(GtkButton *button) {
    (void)button;
    
    // create dialog to load file
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Load Graph",
                                                   GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button))),
                                                   GTK_FILE_CHOOSER_ACTION_OPEN,
                                                   "Cancel", GTK_RESPONSE_CANCEL,
                                                   "Load", GTK_RESPONSE_ACCEPT,
                                                   NULL);
    
    // filter by binary files
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Graph files (*.bin)");
    gtk_file_filter_add_pattern(filter, "*.bin");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    // show dialog
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        if (loadGraph(filename, &currentGraph)) {
            // update the interface with loaded graph
            for (int row = 0; row < SIZE; row++) {
                for (int col = 0; col < SIZE; col++) {
                    if (currentGraph.graph[row][col] != 0) {
                        char text[2];
                        snprintf(text, sizeof(text), "%d", currentGraph.graph[row][col]);
                        gtk_entry_set_text(GTK_ENTRY(entries[row][col]), text);
                    } else {
                        gtk_entry_set_text(GTK_ENTRY(entries[row][col]), "");
                    }
                }
            }
        } else {
            // show error message
            GtkWidget *error_dialog = gtk_message_dialog_new(
                GTK_WINDOW(dialog),
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "Error opening the file: %s", filename);
            gtk_widget_destroy(error_dialog);
        }
        
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

// function to save a graph into a binary file
void on_save_button_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    (void)user_data;
    
    // get graph from interface
    for (int row = 0; row < SIZE; row++) {
        for (int col = 0; col < SIZE; col++) {
            const char *text = gtk_entry_get_text(GTK_ENTRY(entries[row][col]));
            if (text && strlen(text) > 0 && isdigit(text[0])) {
                currentGraph.graph[row][col] = atoi(text);
            } else {
                currentGraph.graph[row][col] = 0;
            }
        }
    }
    
    // create dialog to save file
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Save Graph",
                                                   GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button))),
                                                   GTK_FILE_CHOOSER_ACTION_SAVE,
                                                   "Cancel", GTK_RESPONSE_CANCEL,
                                                   "Save", GTK_RESPONSE_ACCEPT,
                                                   NULL);
    
    // default name suggestion
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "graph.bin");
    
    // filter by binary files
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Graph files (*.bin)");
    gtk_file_filter_add_pattern(filter, "*.bin");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    // show dialog
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (result == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        // always .bin extension
        if (!g_str_has_suffix(filename, ".bin")) {
            char *new_filename = g_strdup_printf("%s.bin", filename);
            g_free(filename);
            filename = new_filename;
        }
        
        if (!saveGraph(filename, &currentGraph)) {
        // show error message
        GtkWidget *error_dialog = gtk_message_dialog_new(
            GTK_WINDOW(dialog),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Error saving file: %s", filename);
        gtk_dialog_run(GTK_DIALOG(error_dialog));
        gtk_widget_destroy(error_dialog);
        }
        
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

// function to create and configure entries on the main grid
void setup_grid(GtkBuilder *builder) {

    // get the main grid
    GtkWidget *main_grid = GTK_WIDGET(gtk_builder_get_object(builder, "IDGrid"));
    if (!main_grid || !GTK_IS_GRID(main_grid)) {
        return;
    }
    
    // initialize the matrix with entries
    entries = g_malloc(5 * sizeof(GtkWidget**));
    for (int i = 0; i < 5; i++) {
        entries[i] = g_malloc(5 * sizeof(GtkWidget*));
    }

    // create x amount of entries
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            // create new entry
            GtkWidget *entry = gtk_entry_new();
            
            // configure the entrys properties
            gtk_entry_set_max_length(GTK_ENTRY(entry), 1);
            gtk_entry_set_width_chars(GTK_ENTRY(entry), 1);
            gtk_widget_set_size_request(entry, 50, 50);
            gtk_entry_set_alignment(GTK_ENTRY(entry), 0.5);
            
            // configure expansion
            gtk_widget_set_hexpand(entry, TRUE);
            gtk_widget_set_vexpand(entry, TRUE);
            gtk_widget_set_halign(entry, GTK_ALIGN_FILL);
            gtk_widget_set_valign(entry, GTK_ALIGN_FILL);
            
            // connect validation signal
            g_signal_connect(entry, "insert-text", G_CALLBACK(on_entry_insert_text), NULL);
            
            // insert entry to the main grid
            gtk_grid_attach(GTK_GRID(main_grid), entry, col, row, 1, 1);
            
            // save entry reference
            entries[row][col] = entry;
        }
    }
}

int main(int argc, char *argv[]) {
    GtkBuilder *builder;
    GtkWidget *window;
    gtk_init(&argc, &argv);

    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "ui/hamilton.glade", NULL);

    window = GTK_WIDGET(gtk_builder_get_object(builder, "IDWindow"));

    
    // GtkWidget *IDLatex = GTK_WIDGET(gtk_builder_get_object(builder, "IDLatex"));
    GtkWidget *IDSave = GTK_WIDGET(gtk_builder_get_object(builder, "IDSave"));
    GtkWidget *IDLoad = GTK_WIDGET(gtk_builder_get_object(builder, "IDLoad"));
    
    if (IDSave)
        g_signal_connect(IDSave, "clicked", G_CALLBACK(on_save_button_clicked), NULL);
    if (IDLoad)
        g_signal_connect(IDLoad, "clicked", G_CALLBACK(on_load_button_clicked), NULL);

    setup_grid(builder);
    
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), NULL);
    
    gtk_builder_connect_signals(builder, NULL);

    g_object_unref(builder);
    gtk_widget_show_all(window);
    gtk_main();
    
    return 0;
}