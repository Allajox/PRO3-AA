#include <gtk/gtk.h>

void on_window_destroy(GtkWidget *widget, gpointer data) {
    // Marcar parámetros como no utilizados para evitar warnings
    (void)widget;
    (void)data;
    gtk_main_quit();
}

int main(int argc, char *argv[]) {
    GtkBuilder *builder;
    GtkWidget *window;
    gtk_init(&argc, &argv);

    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "ui/hamilton.glade", NULL);

    window = GTK_WIDGET(gtk_builder_get_object(builder, "IDWindow"));

    /*
    GtkWidget *IDLatex = GTK_WIDGET(gtk_builder_get_object(builder, "IDLatex"));
    GtkWidget *IDSave = GTK_WIDGET(gtk_builder_get_object(builder, "IDSave"));
    GtkWidget *IDLoad = GTK_WIDGET(gtk_builder_get_object(builder, "IDLoad"));
    */
    
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), NULL);
    
    gtk_builder_connect_signals(builder, NULL);

    g_object_unref(builder);
    gtk_widget_show_all(window);
    gtk_main();
    
    return 0;
}