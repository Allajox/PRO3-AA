

// BIBLIOTECAS


#include <gtk/gtk.h>
#include <string.h>
#include <ctype.h>
#include "logic.h"
#include <time.h>


// VARIABLES GLOBALES


Graph currentGraph;         // Grafo actual (estructura definida en logic.h)
int current_size = 0;       // Tamaño actual del grafo (número de nodos)
gboolean isValidCoo = FALSE;// Indica si las coordenadas son válidas

GtkWidget ***entries = NULL;      // Matriz de GtkEntry (matriz de adyacencia)
GtkWidget ***coo_entries = NULL;  // Matriz de GtkEntry para coordenadas
gboolean *coo_row_valid = NULL;   // Indica si una fila de coordenadas es válida
gboolean *coo_row_dup = NULL;     // Indica si hay coordenadas duplicadas
GtkWidget *global_btn_save = NULL;
GtkWidget *global_btn_latex = NULL;


// FUNCIONES


// VALIDACIONES


// Validación de coordenadas
static void update_coo_states_and_ui(void) {
    if (!coo_entries || current_size <= 0) return;

    int n = current_size;
    int *xs = g_new0(int, n);
    int *ys = g_new0(int, n);

    for (int i = 0; i < n; i++) {
        const gchar *sx = gtk_entry_get_text(GTK_ENTRY(coo_entries[i][0]));
        const gchar *sy = gtk_entry_get_text(GTK_ENTRY(coo_entries[i][1]));
        char *endptr;
        if (!sx || !sy || sx[0] == '\0' || sy[0] == '\0') {
            coo_row_valid[i] = FALSE;
            continue;
        }
        long vx = strtol(sx, &endptr, 10);
        if (*endptr != '\0' || vx <= 0) {
            coo_row_valid[i] = FALSE;
            continue;
        }
        long vy = strtol(sy, &endptr, 10);
        if (*endptr != '\0' || vy <= 0) {
            coo_row_valid[i] = FALSE;
            continue;
        }
        coo_row_valid[i] = TRUE;
        xs[i] = (int)vx;
        ys[i] = (int)vy;
    }

    for (int i = 0; i < n; i++) coo_row_dup[i] = FALSE;

    for (int i = 0; i < n; i++) {
        if (!coo_row_valid[i]) continue;
        for (int j = i + 1; j < n; j++) {
            if (!coo_row_valid[j]) continue;
            if (xs[i] == xs[j] && ys[i] == ys[j]) {
                coo_row_dup[i] = TRUE;
                coo_row_dup[j] = TRUE;
            }
        }
    }

    for (int i = 0; i < n; i++) {
        GtkWidget *wx = coo_entries[i][0];
        GtkWidget *wy = coo_entries[i][1];
        GtkStyleContext *ctxx = gtk_widget_get_style_context(wx);
        GtkStyleContext *ctxy = gtk_widget_get_style_context(wy);

        if (!coo_row_valid[i]) {
            gtk_style_context_add_class(ctxx, "invalid");
            gtk_style_context_add_class(ctxy, "invalid");
            gtk_widget_set_tooltip_text(wx, "Debe ser un entero positivo (>0)");
            gtk_widget_set_tooltip_text(wy, "Debe ser un entero positivo (>0)");
        } else if (coo_row_dup[i]) {
            // find first partner to mention
            int partner = -1;
            for (int j = 0; j < n; j++) {
                if (j == i) continue;
                if (coo_row_valid[j] && xs[i] == xs[j] && ys[i] == ys[j]) { partner = j; break; }
            }
            gtk_style_context_add_class(ctxx, "invalid");
            gtk_style_context_add_class(ctxy, "invalid");
            if (partner >= 0) {
                char *msg = g_strdup_printf("Coordenada duplicada con nodo %d", partner + 1);
                gtk_widget_set_tooltip_text(wx, msg);
                gtk_widget_set_tooltip_text(wy, msg);
                g_free(msg);
            } else {
                gtk_widget_set_tooltip_text(wx, "Coordenada duplicada");
                gtk_widget_set_tooltip_text(wy, "Coordenada duplicada");
            }
        } else {
            // valid and not duplicate
            gtk_style_context_remove_class(ctxx, "invalid");
            gtk_style_context_remove_class(ctxy, "invalid");
            gtk_widget_set_tooltip_text(wx, NULL);
            gtk_widget_set_tooltip_text(wy, NULL);
        }
    }

    g_free(xs);
    g_free(ys);

    gboolean all_ok = TRUE;
    for (int i = 0; i < n; i++) {
        if (!coo_row_valid[i] || coo_row_dup[i]) { all_ok = FALSE; break; }
    }
    isValidCoo = all_ok;

    if (global_btn_save)
        gtk_widget_set_sensitive(global_btn_save, isValidCoo);
    if (global_btn_latex)
        gtk_widget_set_sensitive(global_btn_latex, isValidCoo);
}
// Validación de celdas de la matriz de adyacencia
void on_entry_insert_text(GtkEditable *editable, gchar *new_text, gint new_text_length, 
                         gint *position, gpointer user_data) {
    (void)new_text_length;
    (void)position;
    (void)user_data;
    
    // allow a single character '0' or '1'. If the entry already contains one
    // character and there's no selection, replace the existing character so
    // the user can type '1' directly (no need to select/delete first).
    if (gtk_entry_get_text_length(GTK_ENTRY(editable)) >= 1) {
        gint sel_start, sel_end;
        if (!gtk_editable_get_selection_bounds(editable, &sel_start, &sel_end) || sel_end - sel_start <= 0) {
            // no selection: perform replace-if-valid behaviour
            if (new_text && new_text[0] != '\0' && isdigit((unsigned char)new_text[0])) {
                if (new_text[0] == '0' || new_text[0] == '1') {
                    char buf[2] = { new_text[0], '\0' };
                    // set the entry text to the single new character
                    gtk_entry_set_text(GTK_ENTRY(editable), buf);
                }
            }
            // DOESN'T SEEM TO BE NECESSARY

            // stop default insertion (we already handled replacement or ignored it)
            //g_signal_stop_emission_by_name(editable, "insert-text");
            //return;
        }
        // if there is a selection, allow the insertion (it will replace selected text)
    }
}

// Validación de coordenadas (solo dígitos)
void coo_entry_insert_text(GtkEditable *editable, gchar *new_text, gint new_text_length,
                          gint *position, gpointer user_data) {
    (void)new_text_length;
    (void)position;
    (void)user_data;

    for (int i = 0; new_text[i] != '\0'; i++) {
        if (!g_ascii_isdigit(new_text[i])) {
            g_signal_stop_emission_by_name(editable, "insert-text");
            return;
        }
    }
}
// Validación de coordenadas (solo dígitos)
void coo_entry_changed(GtkEditable *editable, gpointer user_data) {
    (void)user_data;
    (void)editable;
    update_coo_states_and_ui();
}


// CREACIÓN MATRIZ y COO DE TAMAÑO "ORDER"


// Manejo de selección de celdas
gboolean coo_entry_focus_in(GtkWidget *widget, GdkEvent *event, gpointer user_data) {
    (void)event;
    (void)user_data;
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(widget));
    if (text && *text != '\0') {
        char *dup = g_strdup(text);
        g_object_set_data_full(G_OBJECT(widget), "prev_text", dup, g_free);
    }
    return FALSE; // propagate
}

// Sincronización de la matriz de adyacencia
void on_entry_changed(GtkEditable *editable, gpointer user_data) {
    GtkBuilder *builder = (GtkBuilder *)user_data;

    // gets the graph type, 1 is directed, 0 undirected
    GtkComboBoxText *IDType = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "IDType"));
    int type = gtk_combo_box_get_active(GTK_COMBO_BOX(IDType));

    // gets the text from the entry in case the graph is undirected
    const gchar *new_text = gtk_entry_get_text(GTK_ENTRY(editable));

    // validate adjacency entry: if empty or not '0'/'1', reset to '0'
    if (!new_text || new_text[0] == '\0' || !isdigit(new_text[0]) || (new_text[0] != '0' && new_text[0] != '1')) {
        gtk_entry_set_text(GTK_ENTRY(editable), "0");
        return;
    }

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

// Limpieza de la interfaz
void clear_grid(GtkBuilder *builder) {
    // if there aren't any entries, return
    if (!entries) 
        return;

    GtkWidget *grid = GTK_WIDGET(gtk_builder_get_object(builder, "IDGrid"));
    if (!grid || !GTK_IS_GRID(grid)) return;

    // delete all widgets from the grid
    GList *children = gtk_container_get_children(GTK_CONTAINER(grid));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter))
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    
    g_list_free(children);

    // free memory from the matrix
    for (int i = 0; i < current_size; i++) 
        g_free(entries[i]);

    g_free(entries);
    entries = NULL;

    // clear and free coordinate grid (IDGridCoo)
    GtkWidget *coo_grid = GTK_WIDGET(gtk_builder_get_object(builder, "IDGridCoo"));
    if (coo_grid && GTK_IS_GRID(coo_grid)) {
        GList *cchildren = gtk_container_get_children(GTK_CONTAINER(coo_grid));
        for (GList *iter = cchildren; iter != NULL; iter = g_list_next(iter))
            gtk_widget_destroy(GTK_WIDGET(iter->data));
        g_list_free(cchildren);
    }

    if (coo_entries) {
        for (int i = 0; i < current_size; i++) {
            if (coo_entries[i]) {
                g_free(coo_entries[i]);
            }
        }
        g_free(coo_entries);
        coo_entries = NULL;
    }

    if (coo_row_valid) {
        g_free(coo_row_valid);
        coo_row_valid = NULL;
    }
    if (coo_row_dup) {
        g_free(coo_row_dup);
        coo_row_dup = NULL;
    }

    current_size = 0;
}

// Creación de la matriz y coordenadas
void setup_grid(GtkBuilder *builder, int size) {
    // clear the existing grid if one already exists
    if (entries)
        clear_grid(builder);

    // get the main grid
    GtkWidget *grid = GTK_WIDGET(gtk_builder_get_object(builder, "IDGrid"));
    if (!grid || !GTK_IS_GRID(grid)) {
        return;
    }

    currentGraph.order = size;
    fprintf(stdout, "Setting up graph matrix of size %d x %d\n", size, size);
    current_size = size;
    
    // initialize the matrix with entries
    entries = g_malloc(size * sizeof(GtkWidget**));
    for (int i = 0; i < size; i++) {
        entries[i] = g_malloc(size * sizeof(GtkWidget*));
    }

    // initialize coordinate entries matrix (size rows x 2 columns)
    coo_entries = g_malloc(size * sizeof(GtkWidget**));
    for (int i = 0; i < size; i++) {
        coo_entries[i] = g_malloc(2 * sizeof(GtkWidget*));
        coo_entries[i][0] = NULL; // X
        coo_entries[i][1] = NULL; // Y
    }

    // allocate per-row state arrays and initialize to FALSE
    if (coo_row_valid) g_free(coo_row_valid);
    if (coo_row_dup) g_free(coo_row_dup);
    coo_row_valid = g_malloc0(size * sizeof(gboolean));
    coo_row_dup = g_malloc0(size * sizeof(gboolean));

    // Determine per-cell size from IDGridCoo's size_request if available
    GtkWidget *coo_grid = GTK_WIDGET(gtk_builder_get_object(builder, "IDGridCoo"));
    gint coo_w_req = 0, coo_h_req = 0;
    if (coo_grid)
        gtk_widget_get_size_request(coo_grid, &coo_w_req, &coo_h_req);
    const gint DEFAULT_CELL_H = 30;
    gint coo_cell_h = DEFAULT_CELL_H;
    if (coo_h_req > 0)
        coo_cell_h = coo_h_req / (size > 0 ? size : 1);

    // add CSS provider for invalid entries (only once)
    static gboolean css_added = FALSE;
    if (!css_added) {
        GtkCssProvider *provider = gtk_css_provider_new();
    const gchar *css = ".invalid entry, .invalid { background-color: #722F37; color: #ffffff; }";
        GError *err = NULL;
        gtk_css_provider_load_from_data(provider, css, -1, &err);
        if (!err) {
            GdkScreen *screen = gdk_screen_get_default();
            gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        } else {
            g_error_free(err);
        }
        g_object_unref(provider);
        css_added = TRUE;
    }

    // populate the coordinate grid (size rows, 2 columns)
    if (coo_grid && GTK_IS_GRID(coo_grid)) {
        for (int row = 0; row < size; row++) {
            // X entry (column 0)
            GtkWidget *entryX = gtk_entry_new();
            gtk_entry_set_text(GTK_ENTRY(entryX), "0");
            gtk_entry_set_width_chars(GTK_ENTRY(entryX), 4);
            // center text horizontally
            gtk_entry_set_alignment(GTK_ENTRY(entryX), 0.5);
            // only set height minimum so widgets can shrink horizontally when many nodes
            gtk_widget_set_size_request(entryX, -1, coo_cell_h);
            // connect validators
            g_signal_connect(entryX, "insert-text", G_CALLBACK(coo_entry_insert_text), NULL);
            g_signal_connect(entryX, "changed", G_CALLBACK(coo_entry_changed), NULL);
            g_signal_connect(entryX, "focus-in-event", G_CALLBACK(coo_entry_focus_in), NULL);
            gtk_grid_attach(GTK_GRID(coo_grid), entryX, 0, row, 1, 1);
            coo_entries[row][0] = entryX;
            // initially mark as invalid (red) and set tooltip
            GtkStyleContext *ctxx_init = gtk_widget_get_style_context(entryX);
            gtk_style_context_add_class(ctxx_init, "invalid");
            gtk_widget_set_tooltip_text(entryX, "Coordenada no asignada");

            // Y entry (column 1)
            GtkWidget *entryY = gtk_entry_new();
            gtk_entry_set_text(GTK_ENTRY(entryY), "0");
            gtk_entry_set_width_chars(GTK_ENTRY(entryY), 4);
            // center text horizontally
            gtk_entry_set_alignment(GTK_ENTRY(entryY), 0.5);
            gtk_widget_set_size_request(entryY, -1, coo_cell_h);
            // connect validators
            g_signal_connect(entryY, "insert-text", G_CALLBACK(coo_entry_insert_text), NULL);
            g_signal_connect(entryY, "changed", G_CALLBACK(coo_entry_changed), NULL);
            g_signal_connect(entryY, "focus-in-event", G_CALLBACK(coo_entry_focus_in), NULL);
            gtk_grid_attach(GTK_GRID(coo_grid), entryY, 1, row, 1, 1);
            coo_entries[row][1] = entryY;
            // initially mark as invalid (red) and set tooltip
            GtkStyleContext *ctxy_init = gtk_widget_get_style_context(entryY);
            gtk_style_context_add_class(ctxy_init, "invalid");
            gtk_widget_set_tooltip_text(entryY, "Coordenada no asignada");
        }
        gtk_widget_show_all(coo_grid);
        // initialize validity/duplicate UI state
        update_coo_states_and_ui();
    }


    // create entries according to the specified size
    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            // create new entry
            GtkWidget *entry = gtk_entry_new();
            // show a 0 on the entry
            gtk_entry_set_text(GTK_ENTRY(entry), "0");

            // configure the entry properties
            gtk_entry_set_max_length(GTK_ENTRY(entry), 1);
            gtk_entry_set_width_chars(GTK_ENTRY(entry), 1);
            // set size request; match size with coordinate grid entries and center text
            // allow horizontal shrinking by not forcing a minimum width
            gtk_widget_set_size_request(entry, -1, coo_cell_h);
            gtk_entry_set_alignment(GTK_ENTRY(entry), 0.5);
            
            // configure expansion
            gtk_widget_set_hexpand(entry, TRUE);
            gtk_widget_set_vexpand(entry, TRUE);
            gtk_widget_set_halign(entry, GTK_ALIGN_FILL);
            gtk_widget_set_valign(entry, GTK_ALIGN_FILL);
            
            // connect validation signals
            g_signal_connect(entry, "insert-text", G_CALLBACK(on_entry_insert_text), NULL);
            g_signal_connect(entry, "changed", G_CALLBACK(on_entry_changed), builder);
            
            // insert entry to the main grid
            gtk_grid_attach(GTK_GRID(grid), entry, col, row, 1, 1);
            
            // save entry reference
            entries[row][col] = entry;
        }
    }
    // show the updated grid
    gtk_widget_show_all(grid);
}

// Limpieza de la Matriz
void clear_graph_matrix(int size) {
    //GtkBuilder *builder = (GtkBuilder *)user_data;
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            currentGraph.graph[i][j] = 0;
            if (entries && entries[i] && entries[i][j]) {
                gtk_entry_set_text(GTK_ENTRY(entries[i][j]), "");
            }
        }
    }

    //if (entries)
    //    clear_grid(builder);
}

// LATEX //

// Cargar Booleanos en Struct
void load_booleans_graph(Graph *g) {
    int path[SIZE];
    for (int i = 0; i < SIZE; i++) path[i] = -1;

    g->isConnected = isConnected(g->graph, g->order);

    if (g->isDirected) {
        // Directed graph: use directed Eulerian functions
        g->isEulerian = eulerianCycleDirected(g->graph, g->order);
        g->isSemiEulerian = eulerianPathDirected(g->graph, g->order);
    } else {
        // Undirected graph
        g->isEulerian = eulerianCycle(g->graph, g->order);
        g->isSemiEulerian = eulerianPath(g->graph, g->order);
    }

    // Hamiltonian cycle check
    g->hasHamiltonCycle = hamiltonian(g->graph, path, g->order, 1, 0);
    g->hasHamiltonPath = hamiltonian(g->graph, path, g->order, 0, 1);
}

// Latex Builder
void latex_builder(const char *filename, const Graph *g) {

    FILE *file = fopen(filename, "w");
    
    if (!file) {
        g_warning("Failed to open LaTeX file for writing");
        return;
    }
    
    fprintf(file,
        "\\documentclass[12pt]{article}\n\n"
        "%% ==== PACKAGES NECESARIOS ====\n"
        "\\usepackage{fancyhdr}\n"
        "\\usepackage{lastpage}\n"
        "\\usepackage{setspace}\n"
        "\\usepackage[T1]{fontenc}\n"
        "\\usepackage{tikz}\n"
        "\\usepackage[font=small, labelfont=bf]{caption}\n\n"

        "%% ==== CONFIGURACIÓN ====\n"
        "\\newcommand{\\HRule}[1]{\\rule{\\linewidth}{#1}}\n"
        "\\onehalfspacing\n"
        "\\setcounter{tocdepth}{5}\n"
        "\\setcounter{secnumdepth}{5}\n\n"

        "\\pagestyle{fancy}\n"
        "\\fancyhf{}\n"
        "\\setlength\\headheight{15pt}\n"
        "\\fancyhead[R]{Josu\\'e Hidalgo \\& Allan Jim\\'enez}\n"
        "\\fancyfoot[R]{Page \\thepage\\ of \\pageref{LastPage}}\n\n"

        "\\begin{document}\n\n"

        "\\title{\n"
        "\t\\HRule{0.5pt} \\\\\n"
        "\t\\LARGE \\textbf{\\uppercase{ALGORITHM ANALYSIS PROGRAMMED PROJECT 03}}\\\\\n"
        "\t\\HRule{2pt} \\\\ [0.5cm]\n"
        "\t\\normalsize \\vspace*{2\\baselineskip}}\n\n"

        "\\date{}\n\n"

        "\\author{\n"
        "\tJosu\\'e Hidalgo \\& Allan Jim\\'enez \\\\\n"
        "\tStudent ID: 2024800128 \\& 2024154925 \\\\\\\\\n"
        "\tInstituto Tecnol\\'ogico de Costa Rica \\\\\n"
        "\tComputer Engineering Department \\\\\\\\\n"
        "\tProfessor Francisco Torres \\\\\n"
        "\t2nd semester, 2025}\n\n"

        "\\maketitle\n"
        "\\thispagestyle{empty}\n"
        "\\clearpage\n"
        "\\pagenumbering{arabic}\n"
        "\\setcounter{page}{1}\n"
        "\\newpage\n\n"

        "\\section{William Rowan Hamilton}\n"
        "\\begin{itemize}\n"
        "    \\item He was an Irish-British physicist, astronomer, and mathematician, considered the second most important mathematician in the United Kingdom after Isaac Newton. From an early age, he showed exceptional talent for mathematics and languages, being recognized as a child prodigy.\n"
        "    \\item Hamilton is known for creating the ``Icosian Game,'' a mathematical game based on traversing the vertices of a dodecahedron without repeating any, considered a precursor to modern graph theory problems.\n"
        "    \\item He is also famous for his discovery of quaternions, an extension of complex numbers that revolutionized the field of algebra.\n"
        "    \\item According to legend, Hamilton was inspired to come up with this idea while walking across Brougham Bridge in Dublin, and in a moment of inspiration, he wrote the fundamental formula for quaternions on one of the bridge's stones. In honor of this event, a commemorative plaque is on the bridge that recalls this historic moment in science and mathematics.\n"
        "\\end{itemize}\n\n"

        "\\newpage\n\n"

        "\\section{What are Hamiltonian paths and cycles?}\n"
        "\\subsection{Hamiltonian paths}\n"
        "\\begin{description}\n"
        "    \\item[Def:] A simple path that visits every vertex in the graph.\n"
        "    \\item[Def:] A path with no repeated vertices that visits all vertices of the graph.\n"
        "\\end{description}\n"
        "\\subsection{Hamiltonian cycles}\n"
        "\\begin{description}\n"
        "    \\item[Def:] A simple cycle that visits every vertex in the graph.\n"
        "    \\item[Def:] A path with no repeated vertices in which the first and last vertices are the same, visiting all vertices of the graph.\n"
        "\\end{description}\n\n"

        "\\newpage\n\n"

        "\\section{Leonhard Euler}\n"
        "\\begin{itemize}\n"
        "    \\item He was a Swiss mathematician, physicist, and astronomer widely recognized as one of the most prolific and brilliant scientists in history. His contributions span virtually every branch of mathematics, from calculus and number theory to geometry, mechanics, and optics.\n"
        "    \\item Euler was a disciple of Johann Bernoulli and, thanks to his genius, revolutionized mathematical notation by introducing symbols that are still used today, such as $e$ for the base of natural logarithms, $i$ for the imaginary unit, and $f(x)$ for functions.\n"
        "    \\item Among his many achievements are Euler's formula for polyhedra $(V - E + F = 2)$, his graph theory (based on the famous K\\\"onigsberg bridge problem), and Euler's identity, considered one of the most beautiful expressions in mathematics.\n"
        "    \\item Despite losing his sight in his later years, Euler continued to work with a prodigious memory, dictating thousands of pages of research that cemented his place as one of the most influential mathematicians of all time.\n"
        "\\end{itemize}\n\n"

        "\\newpage\n\n"

        "\\section{What are Eulerian paths and cycles?}\n\n"
        "\\subsection{Eulerian paths}\n"
        "\\begin{description}\n"
        "    \\item[Def:] Path that visits every edge exactly once (vertices can repeat).\n"
        "\\end{description}\n\n"
        "\\subsection{Eulerian cycles}\n"
        "\\begin{description}\n"
        "    \\item[Def:] Cycle that visits every edge exactly once and ends in the same vertex that it started at.\n"
        "\\end{description}\n\n"
        "The way to determine an Eulerian cycle or path depends if the graph is directed or undirected. For directed graphs, every vertex needs to have an equal indegree and outdegree for it to have an Eulerian cycle; for an Eulerian path, at most one vertex has $(outdegree - indegree) = 1$ and at most one vertex has $(indegree - outdegree) = 1.$\n\n"
        "For undirected graphs, every vertex needs to have an even degree for them to have an Eulerian cycle; for an Eulerian path, exactly two vertices need to have an odd degree.\n\n"

        "\\newpage\n\n"
    );

    fprintf(file,
        "\\section{Generated graph}\n"
        "\\begin{center}\n"
        "\\begin{tikzpicture}[\n"
        "NotDirectedEven/.style={circle, draw=black, fill=white, very thick, minimum size=8mm},\n"
        "NotDirectedOdd/.style={circle, draw=black, fill=black, very thick, minimum size=8mm, text=white},\n"
        "DirectedEvenInEvenOut/.style={circle, draw=red!80!black, fill=red!30, very thick, minimum size=8mm},\n"
        "DirectedEvenInOddOut/.style={circle, draw=violet!80!black, fill=violet!40, very thick, minimum size=8mm},\n"
        "DirectedOddInEvenOut/.style={circle, draw=purple!80!black, fill=purple!40, very thick, minimum size=8mm, text=white},\n"
        "DirectedOddInOddOut/.style={circle, draw=blue!80!black, fill=blue!40, very thick, minimum size=8mm, text=white}\n"
        "]\n"
    );

    fprintf(file,"%% === NODOS === \n");

    for (int i = 0; i < currentGraph.order; i++) {
        int x = currentGraph.coords[i].x;
        int y = currentGraph.coords[i].y;

        if (g->isDirected) {
            int indeg = 0;
            int outdeg = 0;
            for (int j = 0; j < currentGraph.order; j++) {
                if (currentGraph.graph[j][i] == 1) indeg++;
                if (currentGraph.graph[i][j] == 1) outdeg++;
            }

            if (indeg % 2 == 0 && outdeg % 2 == 0) {
                fprintf(file, "\\node[DirectedEvenInEvenOut] (N%d) at (%d,%d) {%d};\n", i+1, x, y, i+1);
            } else if (indeg % 2 == 0 && outdeg % 2 != 0) {
                fprintf(file, "\\node[DirectedEvenInOddOut] (N%d) at (%d,%d) {%d};\n", i+1, x, y, i+1);
            } else if (indeg % 2 != 0 && outdeg % 2 == 0) {
                fprintf(file, "\\node[DirectedOddInEvenOut] (N%d) at (%d,%d) {%d};\n", i+1, x, y, i+1);
            } else {
                fprintf(file, "\\node[DirectedOddInOddOut] (N%d) at (%d,%d) {%d};\n", i+1, x, y, i+1);
            }
        } else {
            int deg = 0;
            for (int j = 0; j < currentGraph.order; j++) {
                if (currentGraph.graph[i][j] == 1) deg++;
            }

            if (deg % 2 == 0) {
                fprintf(file, "\\node[NotDirectedEven] (N%d) at (%d,%d) {%d};\n", i+1, x, y, i+1);
            } else {
                fprintf(file, "\\node[NotDirectedOdd] (N%d) at (%d,%d) {%d};\n", i+1, x, y, i+1);
            }
        }
    }

    fprintf(file,"%% === ARISTAS === \n");
    
    for (int i = 0; i < currentGraph.order; i++) {
        if (g->isDirected) {
            for (int j = 0; j < currentGraph.order; j++) {
                if (currentGraph.graph[i][j] == 1) {
                    fprintf(file, "\\draw[->, thick] (N%d) -- (N%d);\n", i+1, j+1);
                }
            }
        } else {
            for (int j = i; j < currentGraph.order; j++) {
                if (currentGraph.graph[i][j] == 1) {
                    fprintf(file, "\\draw[<->, thick] (N%d) -- (N%d);\n", i+1, j+1);
                }
            }
        }
    }

    fprintf(file,
        "\\end{tikzpicture}\n"
        "};\n"
        "\\end{tikzpicture}\n"
        "\\end{center}\n\n"

        "\\newpage\n"
        "\\section{Graph properties}\n"
    );

    // ===========================================================================================
    // all this section is to write the graph's properties

    if (g->hasHamiltonCycle)
        fprintf(file, "It has a hamiltonian cycle because there's a path from a starting vertex that visits all the others once and ends up in the starting one.\\\\\n");
    else 
        fprintf(file, "It doesn't have a hamiltonian cycle because there's no way to visit every vertex once and end in the starting one.\\\\\n");

    if (g->hasHamiltonPath)
        fprintf(file, "It has a hamiltonian path because there's a way to visit every vertex without repetition.\\\\\n");
    else 
        fprintf(file, "It doesn't have a hamiltonian path because there's no way to visit every vertex without repeting at least one.\\\\\n");
        
    if (g->isDirected) {
        if (g->isEulerian && g->isConnected)
            fprintf(file, "It's Eulerian because it's connected and all nodes' out degree is the same as its in degree.\\\\\n");
        else if (!g->isEulerian && g->isConnected)
            fprintf(file, "It's not Eulerian because at least one node's out degree is different from its in degree.\\\\\n");
        else if (g->isEulerian && !g->isConnected)
            fprintf(file, "It's not Eulerian because the graph is not connected.\\\\\n");

        if (g->isSemiEulerian)
            fprintf(file, "It's semi-Eulerian because all nodes' out degree is the same as its in degree, excluding 2 nodes.\\\\\n");
        else 
            fprintf(file, "It's not semi-Eulerian because there's more than 2 nodes with different out and in degrees.\\\\\n");
    }
    // for undirected graphs
    else {
        if (g->isEulerian && g->isConnected)
            fprintf(file, "It's Eulerian because it's connected and all nodes have even degree.\\\\\n");
        else if (!g->isEulerian && g->isConnected)
            fprintf(file, "It's not Eulerian because at least one node has an odd degree.\\\\\n");
        else if (g->isEulerian && !g->isConnected)
            fprintf(file, "It's not Eulerian because the graph is not connected.\\\\\n");

        if (g->isSemiEulerian)
            fprintf(file, "It's semi-Eulerian because it has exactly 2 odd degree nodes.\\\\\n");
        else 
            fprintf(file, "It's not semi-Eulerian because it doesn't have exactly 2 odd degree nodes.\\\\\n");
    }

    fprintf(file, "\\end{document}\n");

    fclose(file);
}

// Nombre Diferente
void generate_datetime_filename_prefix(char *buffer, size_t length) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t) {
        strftime(buffer, length, "%Y%m%d_%H%M%S", t);
    } else {
        // fallback
        snprintf(buffer, length, "output");
    }
}

// WIDGETS //

// ComboBox "Type"
void on_type_changed(GtkComboBox *combo_box, gpointer user_data) {
    (void)user_data;

    int new_type = gtk_combo_box_get_active(combo_box);
    if (new_type != currentGraph.isDirected) {
        currentGraph.isDirected = new_type;
        clear_graph_matrix(currentGraph.order);
    }
}

// Botón "LaTeX" / Verificar Hamiltoniano
void on_latex_button_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    (void)user_data;

    // Llama Función mágica
    load_booleans_graph(&currentGraph);

    // Generar un Nombre Diferente
    char filename_prefix[32];
    generate_datetime_filename_prefix(filename_prefix, sizeof(filename_prefix));

    // Build LaTeX filename
    char tex_filename[64];
    snprintf(tex_filename, sizeof(tex_filename), "Files_PDF/%s.tex", filename_prefix);

    // Asegura que exista el directorio Files_PDF
    g_mkdir_with_parents("Files_PDF", 0755);

    // Generar archivo LaTeX
    latex_builder(tex_filename, &currentGraph);

    // Ejecutar pdflatex una sola vez y abrir PDF con evince
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
        "bash -c 'pdflatex -interaction=nonstopmode -output-directory=Files_PDF \"%s\" "
        "&& evince \"Files_PDF/%s.pdf\" >/dev/null 2>&1 "
        "&& rm Files_PDF/*.aux Files_PDF/*.log'",
        tex_filename, filename_prefix);

    GError *gerr = NULL;
    if (!g_spawn_command_line_async(cmd, &gerr)) {
        g_warning("No se pudo lanzar la compilación/visor: %s", gerr ? gerr->message : "unknown");
        if (gerr) g_error_free(gerr);
    }
}

// Botón "Cargar"
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
            
            // update combo box with the loaded order
            GtkComboBoxText *IDType = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "IDType"));

            if (IDType)
                gtk_combo_box_set_active(GTK_COMBO_BOX(IDType), currentGraph.isDirected);
            
            if (IDSpin)
                gtk_spin_button_set_value(IDSpin, currentGraph.order);
            
            // update matrix on the interface
            setup_grid(builder, currentGraph.order);

            // update entries
            for (int row = 0; row < currentGraph.order; row++) {
                for (int col = 0; col < currentGraph.order; col++) {
                    if (currentGraph.graph[row][col] == 1)
                        gtk_entry_set_text(GTK_ENTRY(entries[row][col]), "1");
                }
            }
            // update coordinate entries if present
            if (coo_entries) {
                for (int i = 0; i < currentGraph.order; i++) {
                    char buf[32];
                    snprintf(buf, sizeof(buf), "%d", currentGraph.coords[i].x);
                    gtk_entry_set_text(GTK_ENTRY(coo_entries[i][0]), buf);
                    snprintf(buf, sizeof(buf), "%d", currentGraph.coords[i].y);
                    gtk_entry_set_text(GTK_ENTRY(coo_entries[i][1]), buf);
                }
                update_coo_states_and_ui();
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

// Botón "Guardar"
void on_save_button_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    GtkBuilder *builder = (GtkBuilder *)user_data;
    
    // get current order from spin button
    GtkSpinButton *IDSpin = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "IDSpin"));
    int current_order = 0;
    
    // get current type from combo box
    GtkComboBoxText *IDType = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "IDType"));
    int current_type = 0;

    if (IDType)
        current_type = gtk_combo_box_get_active(GTK_COMBO_BOX(IDType));

    if (IDSpin)
        current_order = gtk_spin_button_get_value_as_int(IDSpin);
        
    currentGraph.isDirected = current_type;
    currentGraph.order = current_order;
        
    // Obtiene la Matriz desde el Grid
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

    // Obtiene la matriz de coordenadas desde el grid de coordenadas (si existe)
    if (coo_entries) {
        for (int i = 0; i < current_order; i++) {
            const char *sx = gtk_entry_get_text(GTK_ENTRY(coo_entries[i][0]));
            const char *sy = gtk_entry_get_text(GTK_ENTRY(coo_entries[i][1]));
            int vx = 0, vy = 0;
            if (sx && *sx != '\0') {
                char *endptr;
                long lx = strtol(sx, &endptr, 10);
                if (*endptr == '\0' && lx > 0) vx = (int)lx;
            }
            if (sy && *sy != '\0') {
                char *endptr;
                long ly = strtol(sy, &endptr, 10);
                if (*endptr == '\0' && ly > 0) vy = (int)ly;
            }
            currentGraph.coords[i].x = vx;
            currentGraph.coords[i].y = vy;
        }
    } else {
        for (int i = 0; i < current_order; i++) { currentGraph.coords[i].x = 0; currentGraph.coords[i].y = 0; }
    }
    
    // PopUp para Guardar el Archivo
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Save Graph",
                                                   GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button))),
                                                   GTK_FILE_CHOOSER_ACTION_SAVE,
                                                   "Cancel", GTK_RESPONSE_CANCEL,
                                                   "Save", GTK_RESPONSE_ACCEPT,
                                                   NULL);
    
    // Nombre Default
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "NameYourGraph.bin");
    
    // Filtro para que se vean únicamente los archivos .bin
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Graph files (*.bin)");
    gtk_file_filter_add_pattern(filter, "*.bin");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    // Mostrar PopUp
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));

    if (result == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        // Siempre extension .bin
        if (!g_str_has_suffix(filename, ".bin")) {
            char *new_filename = g_strdup_printf("%s.bin", filename);
            g_free(filename);
            filename = new_filename;
        }
        
        // ERROR
        if (!saveGraph(filename, &currentGraph)) {
            GtkWidget *error_dialog = gtk_message_dialog_new(
                GTK_WINDOW(dialog),
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "Error saving file: %s", filename);
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
        }

        // Cierra Archivo
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

// Cambio de tamaño (spin button)
void on_spin_order_changed(GtkSpinButton *spin_button, GtkBuilder *builder) {
    int new_size = gtk_spin_button_get_value_as_int(spin_button);
    setup_grid(builder, new_size);
}

// Cierre de ventana
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


// MAIN //


// Main
int main(int argc, char *argv[]) {

    // Builder
    GtkBuilder  *builder;
    GtkWidget   *window;
    gtk_init(&argc, &argv);

    // Inicializar el builder
    GError      *error = NULL;
    builder =   gtk_builder_new();
    
    // Cargar el archivo de interfaz
    if (!gtk_builder_add_from_file(builder, "ui/Grafos.glade", &error)) {
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

    // Obtener los widgets con los IDs correctos del archivo Glade
    GtkSpinButton   *IDSpin = GTK_SPIN_BUTTON(gtk_builder_get_object(builder,   "IDSpin"));
    GtkWidget       *IDSave = GTK_WIDGET(gtk_builder_get_object(builder,        "IDSave"));
    GtkWidget       *IDLoad = GTK_WIDGET(gtk_builder_get_object(builder,        "IDLoad"));
    GtkWidget       *IDLatex = GTK_WIDGET(gtk_builder_get_object(builder,       "IDLatex"));
    GtkComboBoxText *IDType = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "IDType"));
    // Poner como globales para cumplir restricciones
    global_btn_save = IDSave;
    global_btn_latex = IDLatex;

    // Revisa que existan y obtiene eventos
    if (!GTK_IS_SPIN_BUTTON(    IDSpin))
        g_warning("No se encontró el widget IDSpin");
    else
        g_signal_connect(       IDSpin, "value-changed", G_CALLBACK(    on_spin_order_changed   ), builder);
    
    if (!GTK_IS_BUTTON(         IDSave))
        g_warning("No se encontró el widget IDSave");
    else
        g_signal_connect(       IDSave, "clicked", G_CALLBACK(          on_save_button_clicked  ), builder);
    
    if (!GTK_IS_BUTTON(         IDLoad))
        g_warning("No se encontró el widget IDLoad");
    else
        g_signal_connect(       IDLoad, "clicked", G_CALLBACK(          on_load_button_clicked  ), builder);

    if (!GTK_IS_BUTTON(IDLatex)) {
        g_warning("No se encontró el widget IDLatex");
    } else {
        g_signal_connect(IDLatex, "clicked", G_CALLBACK(on_latex_button_clicked), builder);
    }

    if (!GTK_IS_COMBO_BOX_TEXT(IDType))
        g_warning("No se encontró el widget IDType");
    else
        g_signal_connect(IDType, "changed", G_CALLBACK(on_type_changed), builder);

    // Guarda espacio para el Grafo
    memset(&currentGraph, 0, sizeof(Graph));
    // Seteamos un orden inicial    
    currentGraph.order = 5;
    setup_grid(builder, 5);
    
    // Metodos para cerrar.
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), NULL);
    gtk_builder_connect_signals(builder, NULL);
    
    // Inicia el Loop
    gtk_widget_show_all(window);
    gtk_main();
    return 0;
}
