#include <gtk/gtk.h>
#include <string.h>
#include <ctype.h>
#include "logic.h"

GtkWidget ***entries = NULL;
int current_size = 0; 
Graph currentGraph;

void on_window_destroy(GtkWidget *widget, GtkBuilder *builder, gpointer data) {
    // avoid warnings
    (void)widget;
    (void)data;
    
    if (builder) {
        g_object_unref(builder);
        builder = NULL;
    }
    gtk_main_quit();
}

// callback function to validate the numeric entry (greater than 0)
void on_entry_insert_text_adjacency(GtkEditable *editable, gchar *new_text, gint new_text_length, 
                         gint *position, gpointer user_data) {
    (void)new_text_length;
    (void)position;
    (void)user_data;

    // only allow one character
    if (gtk_entry_get_text_length(GTK_ENTRY(editable)) >= 1) {
        g_signal_stop_emission_by_name(editable, "insert-text");
        return;
    }
    
    // validate that the number is 0 or 1
    if (!isdigit(new_text[0]) || (new_text[0] != '0' && new_text[0] != '1')) {
        g_signal_stop_emission_by_name(editable, "insert-text");
        return;
    }
}

// callback function to validate the numeric entry (greater than 0)
void on_entry_insert_text_relative(GtkEditable *editable, gchar *new_text, gint new_text_length, 
                         gint *position, gpointer user_data) {
    (void)new_text_length;
    (void)position;
    (void)user_data;

    // Verificar si el texto actual ya tiene 3 dígitos
    if (gtk_entry_get_text_length(GTK_ENTRY(editable)) >= 3) {
        g_signal_stop_emission_by_name(editable, "insert-text");
        return;
    }
    
    // Validar que solo se ingresen dígitos
    for (int i = 0; new_text[i] != '\0'; i++) {
        if (!isdigit(new_text[i])) {
            g_signal_stop_emission_by_name(editable, "insert-text");
            return;
        }
    }
    
    // Validar que el número sea mayor a 0
    if (new_text[0] == '0' && gtk_entry_get_text_length(GTK_ENTRY(editable)) == 0) {
        g_signal_stop_emission_by_name(editable, "insert-text");
        return;
    }
}

void on_entry_changed(GtkEditable *editable, gpointer user_data) {
    GtkBuilder *builder = (GtkBuilder *)user_data;

    // gets the graph type, 1 is directed, 0 undirected
    GtkComboBoxText *IDType = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "IDType"));
    int type = gtk_combo_box_get_active(GTK_COMBO_BOX(IDType));

    // gets the text from the entry in case the graph is undirected
    const gchar *new_text = gtk_entry_get_text(GTK_ENTRY(editable));

    // check if the user is trying to insert something on the diagonal (not allowed)
    for (int row = 0; row < current_size; row++) {
        for (int col = 0; col < current_size; col++) {
            if (entries[row][col] == GTK_WIDGET(editable)) {
                // check the diagonal and replace with 0 if so
                if (row == col && new_text[0] != '0') {
                    gtk_entry_set_text(GTK_ENTRY(editable), "0");
                    return;
                }
                
                // checks if the graph is marked as undirected (0) and makes the matrix symmetrical
                if (type == 0 && row != col) 
                    gtk_entry_set_text(GTK_ENTRY(entries[col][row]), new_text);
                return;
            }
        }
    }
}

// function to clear all the grid
void clear_grid(GtkBuilder *builder) {
    // if there aren't any entries, return
    if (!entries) 
        return;

    GtkWidget *grid = GTK_WIDGET(gtk_builder_get_object(builder, "IDGrid"));
    if (!grid || !GTK_IS_GRID(grid)) {
        g_free(entries);
        entries = NULL;
        current_size = 0;
        return;
    }

    // delete all widgets from the grid
    GList *children = gtk_container_get_children(GTK_CONTAINER(grid));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        if (iter->data) {
            gtk_widget_destroy(GTK_WIDGET(iter->data));
        }
    }
    g_list_free(children);

    // free memory from the matrix
    for (int i = 0; i < current_size; i++) {
        if (entries[i] != NULL) {
            // No necesitamos liberar los widgets individuales aquí, ya que
            // GTK se encarga de ellos cuando destruimos el contenedor padre
            g_free(entries[i]);
            entries[i] = NULL;
        }
    }

    g_free(entries);
    entries = NULL;
    current_size = 0;
}

// function to create and configure entries on the main grid
void setup_grid(GtkBuilder *builder, int size, gboolean is_relative) {
    // clear the existing grid if one already exists
    if (entries)
        clear_grid(builder);

    // get the main grid
    GtkWidget *grid = GTK_WIDGET(gtk_builder_get_object(builder, "IDGrid"));
    if (!grid || !GTK_IS_GRID(grid)) {
        return;
    }

    current_size = size;
    
    // initialize the matrix with entries
    entries = g_malloc(size * sizeof(GtkWidget**));
    for (int i = 0; i < size; i++) {
        entries[i] = g_malloc(size * sizeof(GtkWidget*));
    }

    // create entries according to the specified size
    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            if (is_relative) {
                // Crear un grid para las coordenadas (X,Y)
                GtkWidget *coord_grid = gtk_grid_new();
                gtk_grid_set_column_homogeneous(GTK_GRID(coord_grid), TRUE);
                gtk_grid_set_row_homogeneous(GTK_GRID(coord_grid), TRUE);
                gtk_grid_set_column_spacing(GTK_GRID(coord_grid), 2);
                
                // Crear los campos de entrada para X e Y
                GtkWidget *entry_x = gtk_entry_new();
                GtkWidget *entry_y = gtk_entry_new();
                
                // Configurar los campos de entrada
                gtk_entry_set_text(GTK_ENTRY(entry_x), "0");
                gtk_entry_set_text(GTK_ENTRY(entry_y), "0");
                gtk_entry_set_max_length(GTK_ENTRY(entry_x), 3);
                gtk_entry_set_max_length(GTK_ENTRY(entry_y), 3);
                gtk_entry_set_width_chars(GTK_ENTRY(entry_x), 3);
                gtk_entry_set_width_chars(GTK_ENTRY(entry_y), 3);
                gtk_widget_set_size_request(entry_x, 30, 20);
                gtk_widget_set_size_request(entry_y, 30, 20);
                
                // Conectar señales de validación
                g_signal_connect(entry_x, "insert-text", 
                               G_CALLBACK(on_entry_insert_text_relative), NULL);
                g_signal_connect(entry_y, "insert-text", 
                               G_CALLBACK(on_entry_insert_text_relative), NULL);
                g_signal_connect(entry_x, "changed", 
                               G_CALLBACK(on_entry_changed), builder);
                g_signal_connect(entry_y, "changed", 
                               G_CALLBACK(on_entry_changed), builder);
                
                // Crear etiquetas X: e Y:
                GtkWidget *label_x = gtk_label_new("X:");
                GtkWidget *label_y = gtk_label_new("Y:");
                
                // Crear un grid para organizar las etiquetas y los campos
                GtkWidget *x_container = gtk_grid_new();
                GtkWidget *y_container = gtk_grid_new();
                
                // Configurar los contenedores
                gtk_grid_attach(GTK_GRID(x_container), label_x, 0, 0, 1, 1);
                gtk_grid_attach(GTK_GRID(x_container), entry_x, 1, 0, 1, 1);
                gtk_grid_attach(GTK_GRID(y_container), label_y, 0, 0, 1, 1);
                gtk_grid_attach(GTK_GRID(y_container), entry_y, 1, 0, 1, 1);
                
                // Añadir los contenedores al grid de coordenadas
                gtk_grid_attach(GTK_GRID(coord_grid), x_container, 0, 0, 1, 1);
                gtk_grid_attach(GTK_GRID(coord_grid), y_container, 1, 0, 1, 1);
                
                // Mostrar todo
                gtk_widget_show_all(coord_grid);
                
                // Crear un contenedor para el par de coordenadas
                GtkWidget *container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
                gtk_box_pack_start(GTK_BOX(container), coord_grid, TRUE, TRUE, 0);
                gtk_widget_show_all(container);
                
                // Insertar el contenedor en el grid principal
                gtk_grid_attach(GTK_GRID(grid), container, col, row, 1, 1);
                
                // No necesitamos guardar referencias a los widgets individuales
                // ya que GTK manejará su ciclo de vida a través del contenedor padre
                entries[row][col] = NULL;  // Marcamos como NULL ya que no necesitamos acceder a estos widgets después
            } else {
                // Código original para la matriz de adyacencia
                GtkWidget *entry = gtk_entry_new();
                gtk_entry_set_text(GTK_ENTRY(entry), "0");
                gtk_entry_set_max_length(GTK_ENTRY(entry), 1);
                gtk_entry_set_width_chars(GTK_ENTRY(entry), 1);
                gtk_widget_set_size_request(entry, 50, 50);
                gtk_entry_set_alignment(GTK_ENTRY(entry), 0.5);
                
                gtk_widget_set_hexpand(entry, TRUE);
                gtk_widget_set_vexpand(entry, TRUE);
                gtk_widget_set_halign(entry, GTK_ALIGN_FILL);
                gtk_widget_set_valign(entry, GTK_ALIGN_FILL);
                
                g_signal_connect(entry, "insert-text", G_CALLBACK(on_entry_insert_text_adjacency), NULL);
                g_signal_connect(entry, "changed", G_CALLBACK(on_entry_changed), builder);
                
                gtk_grid_attach(GTK_GRID(grid), entry, col, row, 1, 1);
                entries[row][col] = entry;
            }
        }
    }
    
    // Mostrar todo el grid
    gtk_widget_show_all(grid);
}

// function to create a matrix
void on_latex_button_clicked(GtkButton *button, gpointer user_data) {
    (void)button;

    GtkBuilder *builder = (GtkBuilder *)user_data;
    
    // Get current size from spin button
    GtkSpinButton *IDSpin = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "IDSpin"));
    int current_order = gtk_spin_button_get_value_as_int(IDSpin);
    
    // Update currentGraph with the interface data
    currentGraph.order = current_order;
    
    for (int row = 0; row < current_order; row++) {
        for (int col = 0; col < current_order; col++) {
            const char *text = gtk_entry_get_text(GTK_ENTRY(entries[row][col]));
            if (text && strcmp(text, "1") == 0)
                currentGraph.graph[row][col] = 1;
            else
                currentGraph.graph[row][col] = 0;
        }
    }

    int path[current_order];
    path[0] = 0;
    printf("Graph matrix:\n");
    printGraph(currentGraph.graph, current_order);

    if (hamiltonian(currentGraph.graph, path, current_order, 1)) {
        printf("Hamiltonian cycle found: ");
        // prints the cycle
        for (int i = 0; i < current_order; i++)
            printf("%d ", path[i]);

        printf("%d\n", path[0]); // completes the cycle with the first node
    } else
        printf("No solution found\n");
}

// function to load a graph into a binary file
void on_load_button_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    
    GtkBuilder *builder = (GtkBuilder *)user_data;

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

            // update spin button with the loaded order
            GtkSpinButton *IDSpin = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "IDSpin"));
            
            GtkComboBoxText *IDType = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "IDType"));

            if (IDType)
                gtk_combo_box_set_active(GTK_COMBO_BOX(IDType), currentGraph.type);
            
            if (IDSpin)
                gtk_spin_button_set_value(IDSpin, currentGraph.order);
            
            // update matrix on the interface
            setup_grid(builder, currentGraph.order, FALSE);

            // update entries
            for (int row = 0; row < currentGraph.order; row++) {
                for (int col = 0; col < currentGraph.order; col++) {
                    if (currentGraph.graph[row][col] == 1)
                        gtk_entry_set_text(GTK_ENTRY(entries[row][col]), "1");
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

    GtkBuilder *builder = (GtkBuilder *)user_data;
    
    // get current order from spin button
    GtkSpinButton *IDSpin = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "IDSpin"));
    int current_order = 0;
    
    GtkComboBoxText *IDType = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "IDType"));
    int current_type = 0;

    if (IDType)
        current_type = gtk_combo_box_get_active(GTK_COMBO_BOX(IDType));

    if (IDSpin)
        current_order = gtk_spin_button_get_value_as_int(IDSpin);

    currentGraph.type = current_type;
    currentGraph.order = current_order;
    
    // get graph from interface
    for (int row = 0; row < current_order; row++) {
        for (int col = 0; col < current_order; col++) {
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

void on_spin_order_changed(GtkSpinButton *spin_button, GtkBuilder *builder) {
    int new_size = gtk_spin_button_get_value_as_int(spin_button);
    // Determinar si estamos en modo relativo o adyacencia
    GtkWidget *relativeButton = GTK_WIDGET(gtk_builder_get_object(builder, "IDRelativeMatrix"));
    gboolean is_relative = relativeButton && !gtk_widget_get_sensitive(relativeButton);
    
    // update grid with new size
    setup_grid(builder, new_size, is_relative);
}

// Función manejadora para el botón de matriz de adyacencia
void on_adjacency_matrix_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    GtkBuilder *builder = (GtkBuilder *)user_data;
    
    // Obtener referencias a los botones
    GtkWidget *relativeButton = GTK_WIDGET(gtk_builder_get_object(builder, "IDRelativeMatrix"));
    GtkWidget *adjacencyButton = GTK_WIDGET(gtk_builder_get_object(builder, "IDAdjacencyMatrix"));
    
    // Obtener el tamaño actual del grid
    GtkSpinButton *IDSpin = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "IDSpin"));
    int size = gtk_spin_button_get_value_as_int(IDSpin);
    
    // Actualizar la interfaz
    if (relativeButton) {
        gtk_widget_set_sensitive(relativeButton, TRUE);
    }
    if (adjacencyButton) {
        gtk_widget_set_sensitive(adjacencyButton, FALSE);
    }
    
    // Recrear el grid en modo adyacencia
    setup_grid(builder, size, FALSE);
}

// Función manejadora para el botón de matriz de adyacencia relativa
void on_relative_matrix_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    GtkBuilder *builder = (GtkBuilder *)user_data;
    
    // Obtener referencias a los botones
    GtkWidget *relativeButton = GTK_WIDGET(gtk_builder_get_object(builder, "IDRelativeMatrix"));
    GtkWidget *adjacencyButton = GTK_WIDGET(gtk_builder_get_object(builder, "IDAdjacencyMatrix"));
    
    // Obtener el tamaño actual del grid
    GtkSpinButton *IDSpin = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "IDSpin"));
    int size = gtk_spin_button_get_value_as_int(IDSpin);
    
    // Actualizar la interfaz
    if (relativeButton) {
        gtk_widget_set_sensitive(relativeButton, FALSE);
    }
    if (adjacencyButton) {
        gtk_widget_set_sensitive(adjacencyButton, TRUE);
    }
    
    // Recrear el grid en modo relativo
    setup_grid(builder, size, TRUE);
}

int main(int argc, char *argv[]) {
    GtkBuilder *builder;
    GtkWidget *window;

    gtk_init(&argc, &argv);

    // Inicializar el builder
    GError *error = NULL;
    builder = gtk_builder_new();
    
    // Cargar el archivo de interfaz
    if (!gtk_builder_add_from_file(builder, "ui/GraphUI.glade", &error)) {
        g_critical("Error al cargar el archivo Glade: %s", error->message);
        g_error_free(error);
        return 1;
    }

    // Obtener la ventana principal (usando el ID correcto del archivo Glade)
    window = GTK_WIDGET(gtk_builder_get_object(builder, "IDWindow"));
    if (!GTK_IS_WINDOW(window)) {
        g_critical("No se pudo cargar la ventana principal (IDWindow)");
        return 1;
    }

    // Obtener los botones de matriz
    GtkWidget *IDRelativeMatrix = GTK_WIDGET(gtk_builder_get_object(builder, "IDRelativeMatrix"));
    GtkWidget *IDAdjacencyMatrix = GTK_WIDGET(gtk_builder_get_object(builder, "IDAdjacencyMatrix"));

    gtk_widget_set_sensitive(IDAdjacencyMatrix, FALSE);

    // Conectar las señales de los botones
    if (!GTK_IS_BUTTON(IDRelativeMatrix)) {
        g_warning("No se encontró el widget IDRelativeMatrix");
    } else {
        g_signal_connect(IDRelativeMatrix, "clicked", G_CALLBACK(on_relative_matrix_clicked), builder);
    }

    if (!GTK_IS_BUTTON(IDAdjacencyMatrix)) {
        g_warning("No se encontró el widget IDAdjacencyMatrix");
    } else {
        g_signal_connect(IDAdjacencyMatrix, "clicked", G_CALLBACK(on_adjacency_matrix_clicked), builder);
    }

    // Obtener los widgets con los IDs correctos del archivo Glade
    GtkSpinButton *IDSpin = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "IDSpin"));
    GtkWidget *IDSave = GTK_WIDGET(gtk_builder_get_object(builder, "IDSave"));
    GtkWidget *IDLoad = GTK_WIDGET(gtk_builder_get_object(builder, "IDLoad"));
    GtkWidget *IDLatex = GTK_WIDGET(gtk_builder_get_object(builder, "IDLatex"));

    // Verificar que los widgets necesarios existen
    if (!GTK_IS_SPIN_BUTTON(IDSpin)) {
        g_warning("No se encontró el widget IDSpin");
    } else {
        g_signal_connect(IDSpin, "value-changed", G_CALLBACK(on_spin_order_changed), builder);
    }

    if (!GTK_IS_BUTTON(IDSave)) {
        g_warning("No se encontró el widget IDSave");
    } else {
        g_signal_connect(IDSave, "clicked", G_CALLBACK(on_save_button_clicked), builder);
    }

    if (!GTK_IS_BUTTON(IDLoad)) {
        g_warning("No se encontró el widget IDLoad");
    } else {
        g_signal_connect(IDLoad, "clicked", G_CALLBACK(on_load_button_clicked), builder);
    }

    if (!GTK_IS_BUTTON(IDLatex)) {
        g_warning("No se encontró el widget IDLatex");
    } else {
        g_signal_connect(IDLatex, "clicked", G_CALLBACK(on_latex_button_clicked), builder);
    }

    memset(&currentGraph, 0, sizeof(Graph));
    // show a default 5x5 grid
    currentGraph.order = 5;
    setup_grid(builder, 5, FALSE); 
    
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), NULL);
    
    gtk_builder_connect_signals(builder, NULL);

    gtk_widget_show_all(window);
    gtk_main();
    
    return 0;
}