#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static GtkWidget *output_view;

// Function to execute the command and display output
void execute_command(GtkWidget *widget, gpointer data) {
    const gchar *command = gtk_entry_get_text(GTK_ENTRY(data));
    char buffer[128];
    FILE *fp;
    GtkTextBuffer *buffer_widget = gtk_text_view_get_buffer(GTK_TEXT_VIEW(output_view));
    GtkTextIter iter;

    // Handle the "clear" command specifically
    if (strcmp(command, "clear") == 0) {
        gtk_text_buffer_set_text(buffer_widget, "", -1); // Clear the text view
        gtk_entry_set_text(GTK_ENTRY(data), ""); // Clear the input entry
        return;
    }

    gtk_text_buffer_get_end_iter(buffer_widget, &iter);

    // Insert blank line for spacing
    gtk_text_buffer_insert(buffer_widget, &iter, "\n", -1);

    // Insert command input in red
    gtk_text_buffer_insert_with_tags_by_name(buffer_widget, &iter, "Input -> ", -1, "bold", "red", NULL);
    gtk_text_buffer_insert_with_tags_by_name(buffer_widget, &iter, command, -1, "red", NULL);
    gtk_text_buffer_insert(buffer_widget, &iter, "\n", -1);

    // Check for "cd" command
    if (strncmp(command, "cd ", 3) == 0) {
        // Change directory
        const char *new_dir = command + 3;
        if (chdir(new_dir) == 0) {
            gtk_text_buffer_insert_with_tags_by_name(buffer_widget, &iter, "Output -> Command executed successfully.\n", -1, "green", NULL);
        } else {
            gtk_text_buffer_insert_with_tags_by_name(buffer_widget, &iter, "Output -> Failed to change directory.\n", -1, "red", NULL);
        }
    } else {
        // Execute the command and display output
        fp = popen(command, "r");
        if (fp == NULL) {
            gtk_text_buffer_insert_with_tags_by_name(buffer_widget, &iter, "Output -> Failed to run command\n", -1, "italic", "blue", NULL);
            gtk_entry_set_text(GTK_ENTRY(data), "");
            return;
        }

        gtk_text_buffer_insert_with_tags_by_name(buffer_widget, &iter, "Output ->\n", -1, "bold", "orange", NULL);

        int output_exists = 0; // Check if command produced output
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            output_exists = 1;
            gtk_text_buffer_insert_with_tags_by_name(buffer_widget, &iter, buffer, -1, "cyan", NULL); // Use cyan for output text
        }

        if (pclose(fp) != 0) {
            gtk_text_buffer_insert_with_tags_by_name(buffer_widget, &iter, "Error: Command failed to execute.\n", -1, "italic", "blue", NULL);
        }

        // If no output was produced, indicate success or failure
        if (!output_exists) {
            gtk_text_buffer_insert_with_tags_by_name(buffer_widget, &iter, "Command executed successfully.\n", -1, "green", NULL);
        }
    }

    // Insert blank line after output
    gtk_text_buffer_insert(buffer_widget, &iter, "\n", -1);

    gtk_entry_set_text(GTK_ENTRY(data), "");
}

// Function to apply CSS styling
void apply_css(GtkWidget *widget) {
    GtkCssProvider *css_provider = gtk_css_provider_new();
    GError *error = NULL;

    if (!gtk_css_provider_load_from_path(css_provider, "style.css", &error)) {
        g_warning("Failed to load CSS: %s", error->message);
        g_clear_error(&error);
        return;
    }

    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(css_provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(css_provider);
}

// Main function
int main(int argc, char *argv[]) {
    GtkWidget *window, *entry, *button, *scrolled_window, *box;
    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Simple Terminal");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), box);

    output_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(output_view), FALSE);
    gtk_widget_set_name(output_view, "textview");
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window), output_view);
    gtk_box_pack_start(GTK_BOX(box), scrolled_window, TRUE, TRUE, 0);

    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter command...");
    gtk_widget_set_name(entry, "entry");
    gtk_box_pack_start(GTK_BOX(box), entry, FALSE, FALSE, 0);

    button = gtk_button_new_with_label("Execute");
    gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);

    g_signal_connect(button, "clicked", G_CALLBACK(execute_command), entry);

    GtkTextBuffer *buffer_widget = gtk_text_view_get_buffer(GTK_TEXT_VIEW(output_view));
    gtk_text_buffer_create_tag(buffer_widget, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
    gtk_text_buffer_create_tag(buffer_widget, "italic", "style", PANGO_STYLE_ITALIC, NULL);
    gtk_text_buffer_create_tag(buffer_widget, "red", "foreground", "red", NULL);
    gtk_text_buffer_create_tag(buffer_widget, "green", "foreground", "green", NULL);
    gtk_text_buffer_create_tag(buffer_widget, "orange", "foreground", "orange", NULL);
    gtk_text_buffer_create_tag(buffer_widget, "cyan", "foreground", "cyan", NULL); // Added cyan tag
    gtk_text_buffer_create_tag(buffer_widget, "blue", "foreground", "blue", NULL);
    gtk_text_buffer_create_tag(buffer_widget, "darkgray", "foreground", "darkgray", NULL); // Added darkgray tag

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    apply_css(window);
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}