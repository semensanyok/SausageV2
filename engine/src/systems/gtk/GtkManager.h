//#pragma once
//
//#include "sausage.h"
//#include "SystemsManager.h"
//#include "Renderer.h"
//#include "RendererContextManager.h"
//#include <gtk/gtk.h>
//#include "gtkresources.c"
//
//class GtkManager {
//  static RendererContextManager* renderer_context_manager;
//  static Renderer* renderer;
//
//  static GtkWidget *win;
//  static GtkWidget *area;
//
//public:
//  GtkManager(SystemsManager* systems_manager) :
//    renderer{systems_manager->renderer},
//    renderer_context_manager{systems_manager->renderer_context_manager} {
//  };
//  void Init() {
//    builder = gtk_builder_new_from_resource ("/test/testGL_gtk4.glade");
//    win = GTK_WIDGET (gtk_builder_get_object (builder, "win"));
//    area = GTK_WIDGET(gtk_builder_get_object (builder, "area"));
//    g_signal_connect(area, "create_context", G_CALLBACK(create_context), NULL);
//    
//  };
//  void Run() {
//    
//  };
//private:
//  static void create_context() {
//    renderer_context_manager->InitContext();
//  };
//  static void release() {
//    
//  };
//  ~GtkManager() {
//  };
//};
