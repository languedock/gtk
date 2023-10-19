// Example of GtkColumnView
// gcc `pkg-config --cflags gtk4` -lm -o colview1 colview1.c `pkg-config --libs gtk4`
  
#include <gtk/gtk.h>
#include <glib-object.h>
#include <stdlib.h>

// Directory and file columns
enum COLS {
	COL_ICON = 1, 
	COL_NAME,
	COL_SIZE,
};

// GListStore
static GListStore *g_store  = NULL;

// Current directory/folder to list
static gchar *g_path = NULL;

// Our window

static void add_to_list_store(const gchar *fname);

static guint64 load_dir_content(const gchar *dir_path) {
	// Load and add directory content to g_store 
	// Return number of added items

	// Clear g_store 
	g_list_store_remove_all( g_store);

	// Save path 
	if (g_path) {
		g_free(g_path);
	}
	
	g_path = g_strdup(dir_path);

	// List directory
	GError *error = NULL;
	GDir *dir = g_dir_open(g_path, 0, &error);

	if(error) { 
		// Probably a file, ignore it
		g_error_free(error);	
		return 0;
	}

	guint64 count = 0;
	const gchar *name;
	name = g_dir_read_name(dir);
	while(name) {
		count ++;
		gchar *path_n_name = g_build_path("/", g_path, name, NULL);

		add_to_list_store(path_n_name);

		g_free(path_n_name);

		name = g_dir_read_name(dir);
	}

	// Close it
	g_dir_close(dir);

	return count;
}

// static gint sorting_func(gconstpointer a, gconstpointer b, gpointer data) {
// 	// Add sorted to g_store 
// 	GFileInfo *finfo_a = G_FILE_INFO(a);
// 	if (!G_IS_FILE_INFO(finfo_a)) {
// 		return 0;
// 	}

// 	GFileInfo *finfo_b = G_FILE_INFO(b);
// 	if (!G_IS_FILE_INFO(finfo_b)) {
// 		return 0;
// 	}

// 	const gchar *name1 = g_file_info_get_name(finfo_a);	
// 	const gchar *name2 = g_file_info_get_name(finfo_b);	

// 	return g_utf8_collate(name1, name2);
// }

static void add_to_list_store(const gchar *fname) {
	// Add filename (GFile) to the list 
	GFile *gfile = g_file_new_for_path(fname);
	GError *error = NULL;

	static const gchar *query_str = G_FILE_ATTRIBUTE_STANDARD_SIZE","
		G_FILE_ATTRIBUTE_STANDARD_NAME","
		G_FILE_ATTRIBUTE_TIME_MODIFIED","
		G_FILE_ATTRIBUTE_STANDARD_ICON","
		G_FILE_ATTRIBUTE_PREVIEW_ICON","
		G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE","
		G_FILE_ATTRIBUTE_ACCESS_CAN_READ","
		G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE","
		G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE;

	GFileInfo *finfo = g_file_query_info(gfile,  query_str, G_FILE_QUERY_INFO_NONE, NULL, &error);

	if (error) {
		g_print("Cannot get GFileInfo for %s. %s.\n", fname, error->message); 
		g_error_free(error);
		return;
	} 	 

	
	// Insert sorted		
	// g_list_store_insert_sorted(g_store, finfo, sorting_func, NULL); 

	// Add unsorted
	g_list_store_append(g_store, finfo);
	//gtk_list_store_append()
	g_object_unref(gfile);
}

// static char *get_file_time_modified(GFileInfo *finfo) {
// 	if (!G_IS_FILE_INFO(finfo)) {
// 		return NULL;
// 	}		

// 	GDateTime *dt = g_file_info_get_modification_date_time(finfo);

// 	return g_date_time_format (dt, "%F");
// }
 
// gint64 get_file_unixtime_modified (GFileInfo *finfo) {
// 	if (!G_IS_FILE_INFO(finfo)) {
// 		return -1;
// 	}
 
// 	GDateTime *dt = g_file_info_get_modification_date_time(finfo);
// 	return g_date_time_to_unix(dt);
// } 

static void setup_listitem_cb (GtkListItemFactory *factory, GtkListItem *list_item, gpointer data) {
	gint col_no = GPOINTER_TO_INT(data);

	switch (col_no) {

		case COL_ICON:
			// Icon
			GtkWidget *image = gtk_image_new();
			gtk_list_item_set_child(list_item, image);
			break;

		case COL_NAME:
			// Dir or file name
			GtkWidget *label = gtk_label_new(NULL);
			gtk_label_set_xalign(GTK_LABEL(label), 0.0);
			gtk_widget_set_size_request (label, 200, -1);
			gtk_list_item_set_child(list_item, label);
			break;

		// File size
		case COL_SIZE:
			label = gtk_label_new(NULL);
			gtk_label_set_xalign(GTK_LABEL(label), 0.0);
			gtk_list_item_set_child(list_item, label);
			break;
		
		default:
			;
	}
}
 
static void bind_listitem_cb (GtkListItemFactory *factory, GtkListItem *list_item, gpointer data) {
	gint col_no = GPOINTER_TO_INT(data);

	static gchar buf[64];

	GFileInfo *finfo = gtk_list_item_get_item(list_item);
	GtkWidget *label = NULL;
	
	switch (col_no) {

		case COL_ICON:
			GIcon *gicon = g_file_info_get_icon(finfo);
			g_object_ref(gicon);
			GtkWidget *image = gtk_list_item_get_child (list_item);
			gtk_image_set_from_gicon(GTK_IMAGE(image), gicon);
  			break;

  		case COL_NAME:
			label = gtk_list_item_get_child (list_item);
			const gchar *name = g_file_info_get_name(finfo);
			gtk_label_set_text(GTK_LABEL(label), name);
			break;		

		case COL_SIZE:
			label = gtk_list_item_get_child (list_item);
			goffset siz = g_file_info_get_size(finfo);
			g_snprintf(buf, 10, "%ld", siz);
			gtk_label_set_text(GTK_LABEL(label), buf);
			break;

		default:
			;

	}	
}

static void teardown_listitem_cb(GtkListItemFactory *factory, GtkListItem *list_item, gpointer data) {
	gtk_list_item_set_child (list_item, NULL);
}

// gint column_sort_func(const void *a, const void *b, void *data) {
// 	gint col_no = GPOINTER_TO_INT(data);

// 	g_print("Sorting on column #%d.\n", col_no);

// 	GFileInfo *finfo_a = G_FILE_INFO(a);

// 	if (!G_IS_FILE_INFO(finfo_a)) {
// 		g_print("column_sort_func , *a is not a GFileInfo\n");
// 	}

// 	GFileInfo *finfo_b = G_FILE_INFO(b);
// 	if (!G_IS_FILE_INFO(finfo_b)) {
// 		g_print("column_sort_func , *b not a GFileInfo\n");
// 	}

// 	switch (col_no) {

// 		case COL_NAME:
// 			const gchar *name1 = g_file_info_get_name(finfo_a);	
// 			const gchar *name2 = g_file_info_get_name(finfo_b);	
// 			return g_utf8_collate(name1, name2);
// 			break;

// 		case COL_SIZE:
// 			goffset siz1 = g_file_info_get_size(finfo_a);
// 			goffset siz2 = g_file_info_get_size(finfo_b);
// 			if (siz1 == siz2) 
// 				return 0;
// 			else if (siz1 > siz2) 
// 				return 1;
// 			else 
// 				return -1;
						
// 			break;
		
// 		default:
// 			;
// 	}	

// 	return 0;
// }

void row_activated_cb(GtkColumnView *colview, guint position, gpointer data) {
	gchar *path_n_name = NULL;
	GAppInfo  *app_info = NULL;
	GList *files = NULL;
	GFile *gfile = NULL;
	GdkAppLaunchContext *context = NULL;
	
	GFileInfo *finfo = g_list_model_get_item(G_LIST_MODEL(gtk_column_view_get_model(colview)), position);
	if (!G_IS_FILE_INFO(finfo)) {
		g_warning("Cannot read GFileInfo at row #%d\n", position);
		return;
	}

	const gchar *name = g_file_info_get_name(finfo);
	path_n_name = g_build_path("/", g_path, name, NULL);

	const gchar *content_type = g_file_info_get_content_type(finfo);
	app_info = g_app_info_get_default_for_type(content_type, FALSE);
  
	// if (!G_IS_APP_INFO(app_info)) {
	// 	g_warning("Cannot find GAppInfo for %s\n", path_n_name);
	// 	goto LBL_1;
	// }

 	context = gdk_display_get_app_launch_context(gtk_widget_get_display(GTK_WIDGET(colview)));

	gfile = g_file_new_for_path(path_n_name);
	
	// if (!G_IS_FILE(gfile)) {
	// 	g_warning("Cannot get GFile for %s\n", path_n_name);
	// 	goto LBL_1;
	// }
	
	files = g_list_append(files, gfile);

	GError *error = NULL;
 	if (!g_app_info_launch (app_info,
                          files,
                          G_APP_LAUNCH_CONTEXT(context),
                          &error)) {
    
    g_print("Cannot launch application for %s. %s.\n", path_n_name, error ? error->message : "");                      
	}	
	
	if (error) {
		g_error_free(error);
	}

	g_list_free(files);
	g_object_unref(gfile); 	
	g_object_unref(context);
	g_free(path_n_name);
	g_object_unref(app_info);
}


// void row_selection_changed_cb(GtkSelectionModel* self, guint position, guint n_items, gpointer data) {

// 	GtkBitset *bset = gtk_selection_model_get_selection(self);
// 	if (gtk_bitset_is_empty(bset) != 1){

// 		guint row = gtk_bitset_get_nth(bset, 0);
//     }
// 		// Debug;	
// 		// g_print("BIT row=%d\n", row);	
	
// 	gtk_bitset_unref(bset);
// }

// static void right_click_menu_cb(GSimpleAction *action, GVariant *parameter, gpointer data) {
// 	// ask_remove_file((GFile*)data);
	
// 	g_print("right_click_menu_cb called...\n");
	
// 	GtkWidget *pop = GTK_WIDGET(pop);
// 	if (GTK_IS_WIDGET(pop)) {
// 		gtk_widget_unparent(GTK_WIDGET(pop));
// 		//g_object_unref(G_OBJECT(pop));
// 	}
// }

// static void create_popup_menu(gint x, gint y, gpointer data) {
// 	// Create/show popup menu 

// 	GtkWidget *colview = GTK_WIDGET(data);

// 	GMenu *menu = g_menu_new();
// 	GMenuItem *item = g_menu_item_new("Delete item", "win.delete-item");
// 	g_menu_append_item(menu, item);
// 	g_object_unref(item);
	
// 	GtkWidget *pop = gtk_popover_menu_new_from_model(G_MENU_MODEL(menu));
// 	gtk_widget_set_parent(pop, GTK_WIDGET(colview));

// 	gtk_popover_present(GTK_POPOVER(pop));	

// 	gtk_popover_set_position(GTK_POPOVER(pop), GTK_POS_LEFT); 
// 	gtk_popover_set_pointing_to(GTK_POPOVER(pop), &(const GdkRectangle){x, y, 1, 1 });

//   	GSimpleAction *action = g_simple_action_new("delete-item",  NULL);
//   	g_signal_connect(action, "activate", G_CALLBACK (right_click_menu_cb), pop);
//   	g_action_map_add_action(G_ACTION_MAP(g_window), G_ACTION(action));

// 	gtk_popover_popup(GTK_POPOVER(pop));	
// }

//static void button_pressed_cb(GtkGestureClick *gesture, gint n_press, gdouble x, gdouble y, gpointer data) {
	// Show popup menu at (x,y) location

	// gint button_no = gtk_gesture_single_get_button(GTK_GESTURE_SINGLE(gesture));
	// Convert to TreeView coordinates
	// gint x_bin, y_bin;
	// gtk_tree_view_convert_widget_to_bin_window_coords(colview, (gint)round(x), (gint)round(y), &x_bin, &y_bin);

	// Show popup menu
	//create_popup_menu((gint)x, (gint)y, data);	

//}

void button1_cb(GtkWidget *widget, gpointer data) {
	g_print("button1_cb\n");
}

void button2_cb(GtkWidget *widget, gpointer data) {
	g_print("button2_cb\n");
}

static void *create_col_view(GtkApplication *app, gpointer data) {

	// Window
    static GtkWidget *g_window = NULL;
	g_window = gtk_application_window_new(app);
	gtk_window_set_default_size (GTK_WINDOW (g_window), 500, 600);

	gtk_window_set_title (GTK_WINDOW(g_window), "File listing");

	GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
	g_object_set(G_OBJECT(vbox), "vexpand", TRUE, NULL);
	gtk_window_set_child(GTK_WINDOW(g_window), vbox);

	// Scrolled window 
	GtkWidget *sw = gtk_scrolled_window_new ();
	g_object_set(G_OBJECT(sw), "vexpand", TRUE, NULL);
	gtk_box_append(GTK_BOX(vbox), sw);

	//Store of GFileInfo* objects	

	g_store = g_list_store_new(G_TYPE_STRING);
	//g_store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING); 

	GListModel *model = G_LIST_MODEL(g_store);
	
	GtkSingleSelection *selection = gtk_single_selection_new(G_LIST_MODEL(model));
	//g_signal_connect (selection, "selection-changed", G_CALLBACK (row_selection_changed_cb), NULL);

	// GtkColumnView
	GtkWidget *colview = gtk_column_view_new(NULL);
	gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (sw), colview);

	// Button press / gesture action on right-mouse-click	
 	// GtkGesture *press = gtk_gesture_click_new ();
  	// gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (press), GDK_BUTTON_SECONDARY); // Right mouse button
  	// gtk_widget_add_controller(colview, GTK_EVENT_CONTROLLER (press));
  	// g_signal_connect(press, "pressed", G_CALLBACK (button_pressed_cb), colview);

	//g_signal_connect(colview, "activate", G_CALLBACK(row_activated_cb), NULL);
	gtk_column_view_set_model(GTK_COLUMN_VIEW (colview), GTK_SELECTION_MODEL (selection));
	g_object_unref (selection);
	
	// Some GtkColumnViewColumns
	// COL_ICON
	GtkListItemFactory *factory = gtk_signal_list_item_factory_new ();
	g_signal_connect (factory, "setup", G_CALLBACK (setup_listitem_cb), GINT_TO_POINTER(COL_ICON));
	g_signal_connect (factory, "bind", G_CALLBACK (bind_listitem_cb), GINT_TO_POINTER(COL_ICON));
	g_signal_connect (factory, "teardown", G_CALLBACK (teardown_listitem_cb), GINT_TO_POINTER(COL_ICON));

	GtkColumnViewColumn *col = gtk_column_view_column_new("Icon2", factory);	
	gtk_column_view_append_column(GTK_COLUMN_VIEW(colview), GTK_COLUMN_VIEW_COLUMN(col));

	// COL_NAME
	factory = gtk_signal_list_item_factory_new ();
	g_signal_connect (factory, "setup", G_CALLBACK (setup_listitem_cb), GINT_TO_POINTER(COL_NAME));
	g_signal_connect (factory, "bind", G_CALLBACK (bind_listitem_cb), GINT_TO_POINTER(COL_NAME));
	g_signal_connect (factory, "teardown", G_CALLBACK (teardown_listitem_cb), GINT_TO_POINTER(COL_NAME));

	col = gtk_column_view_column_new("Name", factory);	
	//GtkCustomSorter *sorter = gtk_custom_sorter_new(column_sort_func, GINT_TO_POINTER(COL_NAME), NULL);
	//gtk_column_view_column_set_sorter(col, GTK_SORTER(sorter));
	gtk_column_view_append_column(GTK_COLUMN_VIEW(colview), GTK_COLUMN_VIEW_COLUMN(col));
	//g_object_unref(sorter);

	// COL_SIZE
	factory = gtk_signal_list_item_factory_new ();
	g_signal_connect (factory, "setup", G_CALLBACK (setup_listitem_cb), GINT_TO_POINTER(COL_SIZE));
	g_signal_connect (factory, "bind", G_CALLBACK (bind_listitem_cb), GINT_TO_POINTER(COL_SIZE));
	g_signal_connect (factory, "teardown", G_CALLBACK (teardown_listitem_cb), GINT_TO_POINTER(COL_SIZE));

	col = gtk_column_view_column_new("Size", factory);	
	//sorter = gtk_custom_sorter_new(column_sort_func, GINT_TO_POINTER(COL_SIZE), NULL);
	//gtk_column_view_column_set_sorter(col, GTK_SORTER(sorter));
	gtk_column_view_append_column(GTK_COLUMN_VIEW(colview), GTK_COLUMN_VIEW_COLUMN(col));
	// g_object_unref(sorter);

	//Two test-buttons at the bottom
	GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
	gtk_box_append(GTK_BOX(vbox), hbox);

  	GtkWidget *bt = gtk_button_new_with_label("Test1");
	g_signal_connect(bt, "clicked", G_CALLBACK(button1_cb), NULL);
	gtk_box_append(GTK_BOX(hbox), bt);

  	bt = gtk_button_new_with_label("Test2");
	g_signal_connect(bt, "clicked", G_CALLBACK(button2_cb), NULL);
	gtk_box_append(GTK_BOX(hbox), bt);

	//	Display listing of current directory
	gchar *curr_dir = g_get_current_dir();
	
	 load_dir_content(curr_dir);

	g_free(curr_dir);

	gtk_widget_show (g_window);
}

int main(int argc, char **argv) {
	GtkApplication *app = gtk_application_new ("org.gtk.colview1.example", G_APPLICATION_FLAGS_NONE);
	g_signal_connect (app, "activate", G_CALLBACK (create_col_view), NULL);
	 
	gint status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);

	return status;
}
