#pragma once
#include <gtk/gtk.h>
#include "testresources.c"
static cairo_surface_t *surface = NULL;
static double start_x;
static double start_y;

class GTKDrawingAreaTest {
  public:
  /* Surface to store current scribbles */

  static void
  clear_surface (void)
  {
    cairo_t *cr;

    cr = cairo_create (surface);

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_destroy (cr);
  }

  /* Create a new surface of the appropriate size to store our scribbles */
  static void
  resize_cb (GtkWidget *widget,
             int        width,
             int        height,
             gpointer   data)
  {
    if (surface)
      {
        cairo_surface_destroy (surface);
        surface = NULL;
      }

    if (gtk_native_get_surface (gtk_widget_get_native (widget)))
      {
        surface = gdk_surface_create_similar_surface (gtk_native_get_surface (gtk_widget_get_native (widget)),
                                                      CAIRO_CONTENT_COLOR,
                                                      gtk_widget_get_width (widget),
                                                      gtk_widget_get_height (widget));

        /* Initialize the surface to white */
        clear_surface ();
      }
  }

  /* Redraw the screen from the surface. Note that the draw
   * callback receives a ready-to-be-used cairo_t that is already
   * clipped to only draw the exposed areas of the widget
   */
  static void
  draw_cb (GtkDrawingArea *drawing_area,
           cairo_t        *cr,
           int             width,
           int             height,
           gpointer        data)
  {
    cairo_set_source_surface (cr, surface, 0, 0);
    cairo_paint (cr);
  }

  /* Draw a rectangle on the surface at the given position */
  static void
  draw_brush (GtkWidget *widget,
              double     x,
              double     y)
  {
    cairo_t *cr;

    /* Paint to the surface, where we store our state */
    cr = cairo_create (surface);

    cairo_rectangle (cr, x - 3, y - 3, 6, 6);
    cairo_fill (cr);

    cairo_destroy (cr);

    /* Now invalidate the drawing area. */
    gtk_widget_queue_draw (widget);
  }
  static void
  drag_begin (GtkGestureDrag *gesture,
              double          x,
              double          y,
              GtkWidget      *area)
  {
    start_x = x;
    start_y = y;

    draw_brush (area, x, y);
  }

  static void
  drag_update (GtkGestureDrag *gesture,
               double          x,
               double          y,
               GtkWidget      *area)
  {
    draw_brush (area, start_x + x, start_y + y);
  }

  static void
  drag_end (GtkGestureDrag *gesture,
            double          x,
            double          y,
            GtkWidget      *area)
  {
    draw_brush (area, start_x + x, start_y + y);
  }

  static void
  pressed (GtkGestureClick *gesture,
           int              n_press,
           double           x,
           double           y,
           GtkWidget       *area)
  {
    clear_surface ();
    gtk_widget_queue_draw (area);
  }

  static void
  close_window (void)
  {
    if (surface)
      cairo_surface_destroy (surface);
  }

  static void
  activate (GtkApplication *app,
            gpointer        user_data)
  {
    GtkWidget *window;
    GtkWidget *frame;
    GtkWidget *drawing_area;
    GtkGesture *drag;
    GtkGesture *press;

    window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (window), "Drawing Area");

    g_signal_connect (window, "destroy", G_CALLBACK (close_window), NULL);

    frame = gtk_frame_new (NULL);
    gtk_window_set_child (GTK_WINDOW (window), frame);

    drawing_area = gtk_drawing_area_new ();
    /* set a minimum size */
    gtk_widget_set_size_request (drawing_area, 100, 100);

    gtk_frame_set_child (GTK_FRAME (frame), drawing_area);

    gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (drawing_area), draw_cb, NULL, NULL);

    g_signal_connect_after (drawing_area, "resize", G_CALLBACK (resize_cb), NULL);

    drag = gtk_gesture_drag_new ();
    gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (drag), GDK_BUTTON_PRIMARY);
    gtk_widget_add_controller (drawing_area, GTK_EVENT_CONTROLLER (drag));
    g_signal_connect (drag, "drag-begin", G_CALLBACK (drag_begin), drawing_area);
    g_signal_connect (drag, "drag-update", G_CALLBACK (drag_update), drawing_area);
    g_signal_connect (drag, "drag-end", G_CALLBACK (drag_end), drawing_area);

    press = gtk_gesture_click_new ();
    gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (press), GDK_BUTTON_SECONDARY);
    gtk_widget_add_controller (drawing_area, GTK_EVENT_CONTROLLER (press));

    g_signal_connect (press, "pressed", G_CALLBACK (pressed), drawing_area);

    gtk_widget_show (window);
  };

  static int
  main (int    argc,
        char **argv)
  {
    GtkApplication *app;
    int status;

    app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
    status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);

    return status;
  }
};

class GTKHelloWorld {
  public:
  static void
  print_hello (GtkWidget *widget,
               gpointer   data)
  {
    g_print ("Hello World\n");
  }

  static void
  activate (GtkApplication *app,
            gpointer        user_data)
  {
    GtkWidget *window;
    GtkWidget *button;

    window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (window), "Window");
    gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);

    button = gtk_button_new_with_label ("Hello World");
    g_signal_connect (button, "clicked", G_CALLBACK (print_hello), NULL);
    gtk_window_set_child (GTK_WINDOW (window), button);

    gtk_window_present (GTK_WINDOW (window));
  }

  static int
  gtk_main (int    argc,
        char **argv)
  {
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK (activate), NULL);
    status = g_application_run(G_APPLICATION (app), argc, argv);
    g_object_unref(app);

    return status;
  }
};


class GTKHelloWorldNonBlocking {
  public:
  static void
  print_hello (GtkWidget *widget,
               gpointer   data)
  {
    g_print ("Hello World\n");
  }
  static void exit(GtkWidget *widget,
           gpointer data) {
    g_main_context_wakeup (NULL);
    gboolean *done = (gboolean *)data;
    *done = TRUE;
  }

  static void
  gtk_main (int    argc,
        char **argv
  )
  {
    gboolean done = FALSE;
    GtkWidget *window, *button;
    gtk_init ();
    window = gtk_window_new ();
    gtk_window_set_title (GTK_WINDOW (window), "hello world");
    gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
    g_signal_connect (window, "destroy", G_CALLBACK (exit), &done);
    button = gtk_button_new ();
    gtk_button_set_label (GTK_BUTTON (button), "hello world");
    gtk_widget_set_margin_top (button, 10);
    gtk_widget_set_margin_bottom (button, 10);
    gtk_widget_set_margin_start (button, 10);
    gtk_widget_set_margin_end (button, 10);
    g_signal_connect (button, "clicked", G_CALLBACK (print_hello), NULL);
    gtk_window_set_child (GTK_WINDOW (window), button);
    gtk_widget_show (window);
    while (!done) {
      g_main_context_iteration(NULL, true);
    }
  }
};

class GTKHelloWorldOpenGl {
  public:
  static void exit(GtkWidget *widget,
            gpointer data) {
    g_main_context_wakeup (NULL);
    gboolean *done = (gboolean *)data;
    *done = TRUE;
  }
  static void render (GtkGLArea *area, GdkGLContext *context)
  {
    // inside this function it's safe to use GL; the given
    // GdkGLContext has been made current to the drawable
    // surface used by the `GtkGLArea` and the viewport has
    // already been set to be the size of the allocation

    // we can start by clearing the buffer
    glClearColor (255, 0, 0, 1);
    glClear (GL_COLOR_BUFFER_BIT);

    // draw your object
    // draw_an_object ();

    // we completed our drawing; the draw commands will be
    // flushed at the end of the signal emission chain, and
    // the buffers will be drawn on the window
  }
  static void create_context (GtkWidget *widget,
               gpointer   data) {
  }
  static void
  print_hello (GtkWidget *widget,
               gpointer   data)
  {
    g_print ("Hello World\n");
  }
  static void gtk_main(int argc, char** argv) {
    gboolean done = FALSE;
    GtkWidget *window, *button, *area, *box;
    gtk_init ();
    window = gtk_window_new ();
    gtk_window_set_title (GTK_WINDOW (window), "hello world");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 800);
    g_signal_connect (window, "destroy", G_CALLBACK (exit), &done);

    // add BOX container ///////////////////////////////////////////////
    box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
    gtk_window_set_child (GTK_WINDOW (window), box);
    // add BOX container end ///////////////////////////////////////////
    
    // add GL area ///////////////////////////////////////////////
    area = GTK_WIDGET(gtk_gl_area_new ());
    g_signal_connect(area, "create_context", G_CALLBACK(create_context), NULL);
    //gtk_window_set_child (GTK_WINDOW (window), area);
    gtk_box_append (GTK_BOX (box), area);
    gtk_widget_set_size_request(area, 200, 200);
    // connect to the "render" signal
    g_signal_connect (area, "render", G_CALLBACK (render), NULL);
    // add GL area end ///////////////////////////////////////////
    

    // add button ///////////////////////////////////////////////
    button = gtk_button_new ();
    gtk_button_set_label (GTK_BUTTON (button), "hello world");
    gtk_widget_set_size_request(button, 20, 20);
    g_signal_connect (button, "clicked", G_CALLBACK (print_hello), NULL);
    gtk_box_append (GTK_BOX (box), button);
    // add button end ///////////////////////////////////////////////
    
    gtk_widget_show (window);
    while (!done) {
      g_main_context_iteration(NULL, true);
    }
    //GtkApplication *app;
    //int status;
    //app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    //g_signal_connect(app, "activate", G_CALLBACK (activate), NULL);
    //status = g_application_run(G_APPLICATION (app), argc, argv);
    //g_object_unref(app);
  }
};


class GTKLoadedFromUIFile {
  public:
  static void exit(GtkWidget *widget,
            gpointer data) {
    g_main_context_wakeup (NULL);
    gboolean *done = (gboolean *)data;
    *done = TRUE;
  }
  static void render (GtkGLArea *area, GdkGLContext *context)
  {
    // inside this function it's safe to use GL; the given
    // GdkGLContext has been made current to the drawable
    // surface used by the `GtkGLArea` and the viewport has
    // already been set to be the size of the allocation

    // we can start by clearing the buffer
    glClearColor (255, 0, 0, 1);
    glClear (GL_COLOR_BUFFER_BIT);

    // draw your object
    // draw_an_object ();

    // we completed our drawing; the draw commands will be
    // flushed at the end of the signal emission chain, and
    // the buffers will be drawn on the window
  }
  static void create_context (GtkWidget *widget,
               gpointer   data) {
  }
  static void
  print_hello (GtkWidget *widget,
               gpointer   data)
  {
    g_print ("Hello World\n");
  }
  
  static void
  clear_surface (void)
  {
    cairo_t *cr;

    cr = cairo_create (surface);

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_destroy (cr);
  }

  /* Create a new surface of the appropriate size to store our scribbles */
  static void
  resize_cb (GtkWidget *widget,
             int        width,
             int        height,
             gpointer   data)
  {
    if (surface)
      {
        cairo_surface_destroy (surface);
        surface = NULL;
      }

    if (gtk_native_get_surface (gtk_widget_get_native (widget)))
      {
        surface = gdk_surface_create_similar_surface (gtk_native_get_surface (gtk_widget_get_native (widget)),
                                                      CAIRO_CONTENT_COLOR,
                                                      gtk_widget_get_width (widget),
                                                      gtk_widget_get_height (widget));

        /* Initialize the surface to white */
        clear_surface ();
      }
  }

  /* Redraw the screen from the surface. Note that the draw
   * callback receives a ready-to-be-used cairo_t that is already
   * clipped to only draw the exposed areas of the widget
   */
  static void
  draw_cb (GtkDrawingArea *drawing_area,
           cairo_t        *cr,
           int             width,
           int             height,
           gpointer        data)
  {
    cairo_set_source_surface (cr, surface, 0, 0);
    cairo_paint (cr);
  }

  /* Draw a rectangle on the surface at the given position */
  static void
  draw_brush (GtkWidget *widget,
              double     x,
              double     y)
  {
    cairo_t *cr;

    /* Paint to the surface, where we store our state */
    cr = cairo_create (surface);

    cairo_rectangle (cr, x - 3, y - 3, 6, 6);
    cairo_fill (cr);

    cairo_destroy (cr);

    /* Now invalidate the drawing area. */
    gtk_widget_queue_draw (widget);
  }
  static void
  drag_begin (GtkGestureDrag *gesture,
              double          x,
              double          y,
              GtkWidget      *area)
  {
    start_x = x;
    start_y = y;

    draw_brush (area, x, y);
  }

  static void
  drag_update (GtkGestureDrag *gesture,
               double          x,
               double          y,
               GtkWidget      *area)
  {
    draw_brush (area, start_x + x, start_y + y);
  }

  static void
  drag_end (GtkGestureDrag *gesture,
            double          x,
            double          y,
            GtkWidget      *area)
  {
    draw_brush (area, start_x + x, start_y + y);
  }

  static void
  pressed (GtkGestureClick *gesture,
           int              n_press,
           double           x,
           double           y,
           GtkWidget       *area)
  {
    clear_surface ();
    gtk_widget_queue_draw (area);
  }

  static void
  close_window (void)
  {
    if (surface)
      cairo_surface_destroy (surface);
  }

  static void gtk_main(int argc, char** argv) {
    GtkWidget *win;
    GtkWidget *nb;
    GtkWidget *lab;
    GtkNotebookPage *nbp;
    GtkWidget *scr;
    GtkWidget *tv;
    GtkWidget *area;
    GtkWidget *button;
    GtkTextBuffer *tb;
    char *contents;
    gsize length;
    char *filename;
    int i;
    GtkBuilder *builder;
    GtkApplication *app;
    gboolean done = FALSE;
    GError* error;

    GtkWidget *drawing_area;
    GtkGesture *drag;
    GtkGesture *press;

    gtk_init ();
    app = gtk_application_new ("GTKLoadedFromUIFile", G_APPLICATION_HANDLES_OPEN);

    //builder = gtk_builder_new_from_resource ("/test/test.ui");
    //win = GTK_WIDGET (gtk_builder_get_object (builder, "win"));
    //gtk_window_set_application (GTK_WINDOW (win), GTK_APPLICATION (app));
    //nb = GTK_WIDGET (gtk_builder_get_object (builder, "nb"));

    // add GL area ///////////////////////////////////////////////
    //builder = gtk_builder_new_from_resource ("/test/testGL.ui");
    //win = GTK_WIDGET (gtk_builder_get_object (builder, "win"));
    //area = GTK_WIDGET(gtk_builder_get_object (builder, "area"));
    //g_signal_connect(area, "create_context", G_CALLBACK(create_context), NULL);
    ////gtk_window_set_child (GTK_WINDOW (window), area);
    //gtk_widget_set_size_request(area, 200, 200);
    //// connect to the "render" signal
    //g_signal_connect (area, "render", G_CALLBACK (render), NULL);
    // add GL area end ///////////////////////////////////////////

    // add GL via glad ///////////////////////////////////////////
    builder = gtk_builder_new_from_resource ("/test/testGL.glade");
    win = GTK_WIDGET (gtk_builder_get_object (builder, "win"));
    area = GTK_WIDGET(gtk_builder_get_object (builder, "area"));
    button = GTK_WIDGET(gtk_builder_get_object (builder, "button"));
    g_signal_connect (button, "clicked", G_CALLBACK (print_hello), NULL);
    g_signal_connect(area, "create_context", G_CALLBACK(create_context), NULL);
    //gtk_window_set_child (GTK_WINDOW (window), area);
    gtk_widget_set_size_request(area, 200, 200);
    // connect to the "render" signal
    g_signal_connect (area, "render", G_CALLBACK (render), NULL);
    // add GL via glad end ///////////////////////////////////////

    // DRAWING AREA ////////////////////////////////////////////////
    drawing_area = GTK_WIDGET(gtk_builder_get_object (builder, "drawing_area"));
    gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (drawing_area), draw_cb, NULL, NULL);

    g_signal_connect_after (drawing_area, "resize", G_CALLBACK (resize_cb), NULL);

    drag = gtk_gesture_drag_new ();
    gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (drag), GDK_BUTTON_PRIMARY);
    gtk_widget_add_controller (drawing_area, GTK_EVENT_CONTROLLER (drag));
    g_signal_connect (drag, "drag-begin", G_CALLBACK (drag_begin), drawing_area);
    g_signal_connect (drag, "drag-update", G_CALLBACK (drag_update), drawing_area);
    g_signal_connect (drag, "drag-end", G_CALLBACK (drag_end), drawing_area);

    press = gtk_gesture_click_new ();
    gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (press), GDK_BUTTON_SECONDARY);
    gtk_widget_add_controller (drawing_area, GTK_EVENT_CONTROLLER (press));

    g_signal_connect (press, "pressed", G_CALLBACK (pressed), drawing_area);
    ////////////////////////////////////////////////////////////

    g_object_unref(builder);
    gtk_window_present (GTK_WINDOW (win));
    g_signal_connect (win, "destroy", G_CALLBACK (exit), &done);

    while (!done) {
      g_main_context_iteration(NULL, true);
    }
  }
};
